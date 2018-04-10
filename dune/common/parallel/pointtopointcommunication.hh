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
#include <mutex>

#include "mpiexceptions.hh"
#include "span.hh"
#include "mpitraits.hh"
#include "mpistatus.hh"
#include "mpigrequest.hh"

namespace Dune{
  template<class Comm>
  class PointToPointCommunication;

  enum CommunicationMode {
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

    template<class T>
    MPI_Request Iarecv(Span<T>& data, int rank, int tag){
      constexpr int WAKEUPTAG = 321654;
      std::shared_ptr<std::tuple<int, std::mutex>> mutex_data =
        std::make_shared<std::tuple<int, std::mutex>>();
      std::get<0>(*mutex_data) = 0;
      auto worker = [data, rank, tag, comm{(MPI_Comm)communicator}, me{communicator.rank()}, mutex_data, WAKEUPTAG]
        (MPIStatus& status) mutable{
        MPIStatus s;
        bool done = false;
        do{
          duneMPICall(MPI_Probe, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, s);
          if(s.source() == rank && s.tag() == tag){
            data.resize(s.count(data.mpiType()));
            duneMPICall(MPI_Recv, data.ptr(), data.size(), data.mpiType(),
                          rank, tag, comm, status);
            done = true;
          }
          if(s.source() == me && s.tag() == WAKEUPTAG){
            bool recv = false;
            std::get<1>(*mutex_data).lock();
            if(std::get<0>(*mutex_data) == 1){
              recv = true;
            }
            std::get<0>(*mutex_data) = 1;
            std::get<1>(*mutex_data).unlock();
            if(recv)
              duneMPICall(MPI_Recv, nullptr, 0, MPI_INT, me, WAKEUPTAG, comm, MPI_STATUS_IGNORE);
            done = true;
          }
        }while(!done);
      };
      auto cancel = [comm{(MPI_Comm)communicator}, me{communicator.rank()}, WAKEUPTAG, mutex_data]
        (bool /*complete*/)
        {
        bool send = false;
        std::get<1>(*mutex_data).lock();
        if(std::get<0>(*mutex_data) == 0){
          send = true;
          std::get<0>(*mutex_data) = 1;
        }
        std::get<1>(*mutex_data).unlock();
        if(send)
          duneMPICall(MPI_Send, nullptr, 0, MPI_INT, me, WAKEUPTAG, comm);
      };
      return MPIGRequest<decltype(worker), decltype(cancel)> (std::move(worker), std::move(cancel));
    }
  public:
    template<class T = void>
    using FutureType = typename Comm::template FutureType<T>;
    // Export Communicator type
    typedef Comm Communicator;

    PointToPointCommunication ( const Comm& c = Comm::commWorld()) :
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
    template<typename T, CommunicationMode m = standard>
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
      duneMPICall(send_fun, span.ptr(), span.size(),
                    span.mpiType(), rank, tag, communicator);
    }

    /** @brief See MPI_ISend
     *
     * @param data
     * @param rank
     * @param tag
     *
     * @warning \p data should not be modified until the returned
     * future is ready.
     *
     * @throw MPIError
     */
    template<typename T, CommunicationMode m = standard>
    FutureType<> isend(const T& data, int rank, int tag)
    {
      Span<const T> span(data);
      FutureType<> f(communicator, false);
      std::function<decltype(MPI_Isend)> send_fun;
      switch(m){
      case CommunicationMode::synchronous:
        send_fun = MPI_Issend;
        break;
      case CommunicationMode::buffered:
        send_fun = MPI_Ibsend;
        break;
      case CommunicationMode::ready:
        send_fun = MPI_Irsend;
        break;
      default:
        send_fun = MPI_Isend;
      }
      duneMPICall(send_fun, span.ptr(), span.size(),
                    span.mpiType(), rank, tag,
                    communicator, &f.mpiRequest());
      dverb << "isend() req = " << f.mpiRequest() << std::endl;
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
    MPIStatus recv(T& data, int rank, int tag, bool resize = false)
    {
      Span<T> span(data);
      if(Span<T>::dynamicSize && resize){
        auto ms = mprobe(rank, tag);
        ms.recv(data);
        return ms;
      }else{
        MPIStatus status;
        duneMPICall(MPI_Recv, span.ptr(), span.size(),
                      span.mpiType(), rank, tag,
                      communicator, status);
        return status;
      }
    }

    /** @brief see MPI_Irecv
     *
     * @param data Message buffer. It is forwarded into the Future object.
     * @param rank
     * @param tag
     * @param dynamicSize indicates whether the buffer is resized by
     * the Future before actually receiving the message. That only
     * works for types of dynamic size (e.g. std::vector).
     *
     * @return Future which contains the message.
     *
     * @throw MPIError
     */
    template<typename T>
    FutureType<std::decay_t<T>> irecv(T&& data, int rank, int tag, bool dynamicSize = false)
    {
      FutureType<std::decay_t<T>> f(communicator, true, std::forward<T>(data));
      Span<std::decay_t<T>> span(f.buffer());
      if(dynamicSize && Span<std::decay_t<T>>::dynamicSize){
        f.mpiRequest() = Iarecv(span, rank, tag);
      }else{
        duneMPICall(MPI_Irecv, span.ptr(), span.size(),
                      span.mpiType(), rank, tag,
                      communicator, &f.mpiRequest());
      }
      dverb << "irecv() req = " << f.mpiRequest() << std::endl;
      return std::move(f);
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
      duneMPICall(MPI_Probe, source, tag, communicator, &status);
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
      duneMPICall(MPI_Iprobe, source, tag, communicator, &flag, status);
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
      duneMPICall(MPI_Mprobe, source, tag, communicator, &message, &status);
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
      duneMPICall(MPI_Improbe, source, tag, communicator, &flag, &message, &status);
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
  struct NoComm;

  template<>
  class PointToPointCommunication<NoComm> {
  public:
    template<class T>
    using FutureType = PseudoFuture<T>;

    PointToPointCommunication(const NoComm&){}

    // No implementation since point to point doesn't make sense in
    // sequential case
  };
}
#endif
