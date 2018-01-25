// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/*!
  \file
  \brief Implements an utility class that provides
  point-to-point communication methods for sequential and mpi programs.
  \author Nils-Arne Dreier
  \ingroup ParallelCommunication
*/

#ifndef DUNE_COMMON_PARALLEL_POINTTOPOINTCOMMUNICATION_HH
#define DUNE_COMMON_PARALLEL_POINTTOPOINTCOMMUNICATION_HH

#if HAVE_MPI
#include <mpi.h>
#endif

#include "future.hh"
#include "mpiexceptions.hh"
#include "span.hh"
#include "mpitraits.hh"
#include "mpistatus.hh"

namespace Dune{
  template<class Comm>
  class PointToPointCommunication;

  enum Communication_Mode {
    standard,
    buffered,
    synchronous,
    ready
  };

#if HAVE_MPI
  /*! @brief Implements an interface for point-to-point communication.
   */

  template<class Comm>
  class PointToPointCommunication {
    template<class T = void>
    using RecvFutureType = typename Comm::template RecvFutureType<T>;
    template<class T = void>
    using ProbeFutureType = typename Comm::template ProbeFutureType<T>;
  public:
    template<class T = void>
    using FutureType = typename Comm::template FutureType<T>;
    // Export Communicator type
    typedef Comm Communicator;

    PointToPointCommunication ( const Comm& c = Comm::comm_world()) :
      communicator(c)
    {}

    /** @brief See MPI_Send
     *
     * @param data
     * @param rank
     * @param tag
     *
     * @throw MPIError
     */
    template<typename T, Communication_Mode m = standard>
    void send(const T& data, int rank, int tag)
    {
      std::function<decltype(MPI_Send)> send_fun;
      switch(m){
      case synchronous:
        send_fun = MPI_Ssend;
        break;
      case buffered:
        send_fun = MPI_Bsend;
        break;
      case ready:
        send_fun = MPI_Rsend;
        break;
      default:
        send_fun = MPI_Send;
      }
      Span<const T> span(data);
      dune_mpi_call(send_fun, span.ptr(), span.size(),
                    span.mpi_type(), rank, tag, communicator);
    }

    /** @brief See MPI_ISend
     *
     * @param data
     * @param rank
     * @param tag
     *
     * @warning \p data should not be modified until the returned
     * future is valid.
     *
     * @throw MPIError
     */
    template<typename T, Communication_Mode m = standard>
    FutureType<> isend(const T& data, int rank, int tag)
    {
      Span<const T> span(data);
      FutureType<> f(communicator, false);
      std::function<decltype(MPI_Isend)> send_fun;
      switch(m){
      case Communication_Mode::synchronous:
        send_fun = MPI_Issend;
        break;
      case Communication_Mode::buffered:
        send_fun = MPI_Ibsend;
        break;
      case Communication_Mode::ready:
        send_fun = MPI_Irsend;
        break;
      default:
        send_fun = MPI_Isend;
      }
      dune_mpi_call(send_fun, span.ptr(), span.size(),
                    span.mpi_type(), rank, tag,
                    communicator, &f.req_);
      dverb << "isend() req = " << f.req_ << std::endl;
      return std::move(f);
    }


    /** @brief See MPI_Recv
     *
     * @param data
     * @param rank
     * @param tag
     *
     * @return Status object containing information about the
     * received message.
     *
     * @throw MPIError
     */
    template<typename T>
    MPIStatus recv(T& data, int rank, int tag)
    {
      Span<T> span(data);
      if(Span<T>::dynamic_size){
        auto ms = mprobe(rank, tag);
        ms.recv(data);
        return ms;
      }else{
        MPIStatus status;
        dune_mpi_call(MPI_Recv, span.ptr(), span.size(),
                      span.mpi_type(), rank, tag,
                      communicator, status);
        return status;
      }
    }

    /** @brief see MPI_Irecv
     *
     * @param data Message buffer. It is forwarded into the Future object.
     * @param rank
     * @param tag
     * @param dynamic_size indicates whether the buffer is resized by
     * the Future before actually receiving the message. That only
     * works for types of dynamic size (e.g. std::vector).
     *
     * @return Future which contains the message.
     *
     * @throw MPIError
     */
    template<typename T>
    RecvFuture<std::decay_t<T>> irecv(T&& data, int rank, int tag, bool dynamic_size = false)
    {
      if(dynamic_size && Span<std::decay_t<T>>::dynamic_size){
        return ProbeFutureType<std::decay_t<T>>(communicator, false,
                                                std::forward<T>(data),
                                                rank, tag);
      }else{
        RecvFutureType<std::decay_t<T>> f(communicator, false, std::forward<T>(data));
        Span<std::decay_t<T>> span(f.buffer());
        dune_mpi_call(MPI_Irecv, span.ptr(), span.size(),
                      span.mpi_type(), rank, tag,
                      communicator, &f.req_);
        dverb << "irecv() req = " << f.req_ << std::endl;
        return std::move(f);
      }
    }

    /** @brief See MPI_Probe.
     *
     * @param source
     * @param tag
     *
     * @return Status object containing information about the message
     * @throw MPIError
     */
    MPIStatus probe(int source, int tag) const {
      MPI_Status status;
      dune_mpi_call(MPI_Probe, source, tag, communicator, &status);
      return status;
    }

    /** @brief See MPI_IProbe.
     *
     * @param source
     * @param tag
     *
     * @return Status object containing information about the message.
     * @throw MPIError
     */
    MPIStatus iprobe(int source, int tag) const {
      int flag;
      MPIStatus status;
      dune_mpi_call(MPI_Iprobe, source, tag, communicator, &flag, status);
      if(flag)
        return status;
      else
        return {};
    }

    /** @brief See MPI_Mprobe.
     *
     * @param source
     * @param tag
     *
     * @return Status object containing information about the message
     * and providing functionality for receiving the message.
     * @throw MPIError
     */
    MPIMatchingStatus mprobe(int source, int tag) const {
      MPI_Status status;
      MPI_Message message;
      dune_mpi_call(MPI_Mprobe, source, tag, communicator, &message, &status);
      return {status, message};
    }

    /** @brief See MPI_Improbe.
     *
     * @param source
     * @param tag
     *
     * @return Status object containing information about the message
     * and providing functionality for receiving the message.
     * @throw MPIError
     */
    MPIMatchingStatus improbe(int source, int tag) const {
      MPI_Status status;
      MPI_Message message;
      int flag;
      dune_mpi_call(MPI_Improbe, source, tag, communicator, &flag, &message, &status);
      if(flag)
        return {status, message};
      else
        return {};
    }

    int rank() const{
      return communicator.rank();
    }

    int size() const{
      return communicator.size();
    }

    operator Comm () const{
      return communicator;
    }

  private:
    Comm communicator;
  };

#endif
  struct No_Comm;

  template<>
  class PointToPointCommunication<No_Comm> {
  public:
    template<class T>
    using FutureType = PseudoFuture<T>;

    PointToPointCommunication(const No_Comm&){}

    // No implementation since point to point doesn't make sense in
    // sequential case
  };
}
#endif
