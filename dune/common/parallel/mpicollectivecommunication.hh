// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_MPICOLLECTIVECOMMUNICATION_HH
#define DUNE_COMMON_PARALLEL_MPICOLLECTIVECOMMUNICATION_HH

/*!
   \file
   \brief Implements an utility class that provides
   MPI's collective communication methods.

   \ingroup ParallelCommunication
 */

#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>

#include <dune/common/exceptions.hh>
#include <dune/common/binaryfunctions.hh>

#include "managedmpicomm.hh"
#include "collectivecommunication.hh"
#include "mpitraits.hh"
#include "mpiexceptions.hh"

#include <dune/common/deprecated.hh>

#if HAVE_MPI

namespace Dune
{

  //=======================================================
  // use singleton pattern and template specialization to
  // generate MPI operations
  //=======================================================

  template<typename Type, typename BinaryFunction>
  class Generic_MPI_Op
  {

  public:
    static MPI_Op get ()
    {
      if (!op)
      {
        op = std::shared_ptr<MPI_Op>(new MPI_Op);
        dune_mpi_call(MPI_Op_create,(void (*)(void*, void*, int*, MPI_Datatype*))&operation,true,op.get());
      }
      return *op;
    }
  private:
    static void operation (Type *in, Type *inout, int *len, MPI_Datatype*)
    {
      BinaryFunction func;

      for (int i=0; i< *len; ++i, ++in, ++inout) {
        Type temp;
        temp = func(*in, *inout);
        *inout = temp;
      }
    }
    Generic_MPI_Op () {}
    Generic_MPI_Op (const Generic_MPI_Op& ) {}
    static std::shared_ptr<MPI_Op> op;
  };


  template<typename Type, typename BinaryFunction>
  std::shared_ptr<MPI_Op> Generic_MPI_Op<Type,BinaryFunction>::op = std::shared_ptr<MPI_Op>(static_cast<MPI_Op*>(0));

#define ComposeMPIOp(type,func,op) \
  template<> \
  class Generic_MPI_Op<type, func<type> >{ \
  public:\
    static MPI_Op get(){ \
      return op; \
    } \
  private:\
    Generic_MPI_Op () {}\
    Generic_MPI_Op (const Generic_MPI_Op & ) {}\
  }


  ComposeMPIOp(char, std::plus, MPI_SUM);
  ComposeMPIOp(unsigned char, std::plus, MPI_SUM);
  ComposeMPIOp(short, std::plus, MPI_SUM);
  ComposeMPIOp(unsigned short, std::plus, MPI_SUM);
  ComposeMPIOp(int, std::plus, MPI_SUM);
  ComposeMPIOp(unsigned int, std::plus, MPI_SUM);
  ComposeMPIOp(long, std::plus, MPI_SUM);
  ComposeMPIOp(unsigned long, std::plus, MPI_SUM);
  ComposeMPIOp(float, std::plus, MPI_SUM);
  ComposeMPIOp(double, std::plus, MPI_SUM);
  ComposeMPIOp(long double, std::plus, MPI_SUM);

  ComposeMPIOp(char, std::multiplies, MPI_PROD);
  ComposeMPIOp(unsigned char, std::multiplies, MPI_PROD);
  ComposeMPIOp(short, std::multiplies, MPI_PROD);
  ComposeMPIOp(unsigned short, std::multiplies, MPI_PROD);
  ComposeMPIOp(int, std::multiplies, MPI_PROD);
  ComposeMPIOp(unsigned int, std::multiplies, MPI_PROD);
  ComposeMPIOp(long, std::multiplies, MPI_PROD);
  ComposeMPIOp(unsigned long, std::multiplies, MPI_PROD);
  ComposeMPIOp(float, std::multiplies, MPI_PROD);
  ComposeMPIOp(double, std::multiplies, MPI_PROD);
  ComposeMPIOp(long double, std::multiplies, MPI_PROD);

  ComposeMPIOp(char, Min, MPI_MIN);
  ComposeMPIOp(unsigned char, Min, MPI_MIN);
  ComposeMPIOp(short, Min, MPI_MIN);
  ComposeMPIOp(unsigned short, Min, MPI_MIN);
  ComposeMPIOp(int, Min, MPI_MIN);
  ComposeMPIOp(unsigned int, Min, MPI_MIN);
  ComposeMPIOp(long, Min, MPI_MIN);
  ComposeMPIOp(unsigned long, Min, MPI_MIN);
  ComposeMPIOp(float, Min, MPI_MIN);
  ComposeMPIOp(double, Min, MPI_MIN);
  ComposeMPIOp(long double, Min, MPI_MIN);

  ComposeMPIOp(char, Max, MPI_MAX);
  ComposeMPIOp(unsigned char, Max, MPI_MAX);
  ComposeMPIOp(short, Max, MPI_MAX);
  ComposeMPIOp(unsigned short, Max, MPI_MAX);
  ComposeMPIOp(int, Max, MPI_MAX);
  ComposeMPIOp(unsigned int, Max, MPI_MAX);
  ComposeMPIOp(long, Max, MPI_MAX);
  ComposeMPIOp(unsigned long, Max, MPI_MAX);
  ComposeMPIOp(float, Max, MPI_MAX);
  ComposeMPIOp(double, Max, MPI_MAX);
  ComposeMPIOp(long double, Max, MPI_MAX);

  ComposeMPIOp(char, std::bit_and, MPI_BAND);
  ComposeMPIOp(unsigned char, std::bit_and, MPI_BAND);
  ComposeMPIOp(short, std::bit_and, MPI_BAND);
  ComposeMPIOp(unsigned short, std::bit_and, MPI_BAND);
  ComposeMPIOp(int, std::bit_and, MPI_BAND);
  ComposeMPIOp(unsigned int, std::bit_and, MPI_BAND);
  ComposeMPIOp(long, std::bit_and, MPI_BAND);
  ComposeMPIOp(unsigned long, std::bit_and, MPI_BAND);
  ComposeMPIOp(float, std::bit_and, MPI_BAND);
  ComposeMPIOp(double, std::bit_and, MPI_BAND);
  ComposeMPIOp(long double, std::bit_and, MPI_BAND);

  ComposeMPIOp(char, std::bit_or, MPI_BOR);
  ComposeMPIOp(unsigned char, std::bit_or, MPI_BOR);
  ComposeMPIOp(short, std::bit_or, MPI_BOR);
  ComposeMPIOp(unsigned short, std::bit_or, MPI_BOR);
  ComposeMPIOp(int, std::bit_or, MPI_BOR);
  ComposeMPIOp(unsigned int, std::bit_or, MPI_BOR);
  ComposeMPIOp(long, std::bit_or, MPI_BOR);
  ComposeMPIOp(unsigned long, std::bit_or, MPI_BOR);
  ComposeMPIOp(float, std::bit_or, MPI_BOR);
  ComposeMPIOp(double, std::bit_or, MPI_BOR);
  ComposeMPIOp(long double, std::bit_or, MPI_BOR);

  ComposeMPIOp(bool, std::logical_and, MPI_LAND);
  ComposeMPIOp(bool, std::logical_or, MPI_LOR);
#undef ComposeMPIOp


  /*! \brief Specialization of CollectiveCommunication for MPI

    \tparam Comm should be a derived class of ManagedMPIComm

    \ingroup ParallelCommunication
   */
  template<typename Comm>
  class GenericMPICollectiveCommunication
  {
  public:
    // Export Future type
    template<class T = void>
    using FutureType = typename Comm::template FutureType<T>;

    //! Instantiation using a MPI communicator_.
    GenericMPICollectiveCommunication (const Comm& c = Comm::comm_world())
      : communicator_(c)
    {
      if (communicator_) {
        int initialized = 0;
        MPI_Initialized(&initialized);
        if (!initialized)
          DUNE_THROW(ParallelError,"You must call MPIHelper::instance(argc,argv) in your main() function before using the MPI CollectiveCommunication!");
      }
    }

    //! @copydoc CollectiveCommunication<No_Comm>::rank
    int rank () const
    {
      return communicator_.rank();
    }

    //! @copydoc CollectiveCommunication<No_Comm>::size
    int size () const
    {
      return communicator_.size();
    }

    operator Comm () const
    {
      return communicator_;
    }

    operator MPI_Comm () const
    {
      return communicator_;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::sum(const T&)const
    template<typename T>
    T sum (const T& in) const
    {
      T out;
      allreduce<std::plus<T>>(in, out);
      return out;
    }

    //! @copydoc Dune::CollectiveCommunication<No_Comm>::sum(T*,int)const
    template<typename T>
    DUNE_DEPRECATED
    int sum (T* inout, int len) const
    {
      Span<T*> span(Span<T*>(inout, len));
      allreduce<std::plus<T>>(span);
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::prod(const T&)const
    template<typename T>
    T prod (const T& in) const
    {
      T out;
      allreduce<std::multiplies<T>>(in, out);
      return out;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::prod(T*,int)const
    template<typename T>
    DUNE_DEPRECATED
    int prod (T* inout, int len) const
    {
      allreduce<std::multiplies<T>, Span<T*>>(Span<T*>(inout, len));
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::min(const T&)const
    template<typename T>
    T min (const T& in) const
    {
      T out;
      allreduce<Min<T>>(in, out);
      return out;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::min(T*,int)const
    template<typename T>
    DUNE_DEPRECATED
    int min (T* inout, int len) const
    {
      allreduce<Min<T>, Span<T*>>(Span<T*>(inout, len));
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::max(const T&)const
    template<typename T>
    T max (const T& in) const
    {
      T out;
      allreduce<Max<T>>(in, out);
      return out;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::max(T*,int)const
    template<typename T>
    DUNE_DEPRECATED
    int max (T* inout, int len) const
    {
      allreduce<Max<T>, Span<T*>>(Span<T*>(inout, len));
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::barrier()
    void barrier () const
    {
      dune_mpi_call(MPI_Barrier, communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::ibarrier()
    FutureType<> ibarrier() const
    {
      FutureType<> f(communicator_, false);
      dune_mpi_call(MPI_Ibarrier, communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::broadcast(T*,int,int)const
    template<typename T>
    DUNE_DEPRECATED
    int broadcast (T* inout, int len, int root) const
    {
      broadcast<Span<T*>>({inout, len}, root);
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::broadcast(T&,int)const
    template<typename T>
    void broadcast (T& inout, int root) const
    {
      Span<T> span(inout);
      dune_mpi_call(MPI_Bcast, span.ptr(), span.size(),
                    span.mpi_type(), root, communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::ibroadcast
    template<typename T>
    FutureType<std::decay_t<T>> ibroadcast (T&& data, int root) const
    {
      FutureType<std::decay_t<T>> f(communicator_, false,
                                    std::forward<T>(data));
      Span<std::decay_t<T>> span(f.buffer());
      dune_mpi_call(MPI_Ibcast, span.ptr(), span.size(),
                    span.mpi_type(), root, communicator_,
                    &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::gather(T*,S*,int,int)const
    //! @note out must have space for P*len elements
    template<typename T, typename S>
    DUNE_DEPRECATED
    int gather (T* in, S* out, int len, int root) const
    {
      gather<Span<const T*>, Span<S*>>({in, len}, {out, size()*len}, root);
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::gather(const T&, S&, int)const
    template<typename T, typename S>
    void gather (const T& in, S& out, int root) const
    {
      Span<const T> span_in(in);
      Span<S> span_out(out);
      // WARNING: The size argument of the receive buffer is the count per process!
      dune_mpi_call(MPI_Gather, span_in.ptr(), span_in.size(),
                    span_in.mpi_type(), span_out.ptr(),
                    span_in.size(), span_out.mpi_type(),
                    root, communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::igather
    template<typename T, typename S>
    FutureType<std::decay_t<S>> igather (const T& in, S&& out, int root) const
    {
      Span<const T> span_in(in);
      FutureType<std::decay_t<S>> f(communicator_, false, std::forward<S>(out));
      Span<std::decay_t<S>> span_out(f.buffer());
      // WARNING: The size argument of the receive buffer is the count per process!
      dune_mpi_call(MPI_Igather, span_in.ptr(), span_in.size(),
                    span_in.mpi_type(), span_out.ptr(),
                    span_in.size(), span_out.mpi_type(),
                    root, communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::gatherv(T*,int,S*,int*,int*,int)const
    template<typename T, typename S>
    DUNE_DEPRECATED
    int gatherv (T* in, int sendlen, S* out, int* recvlen, int* displ, int root) const
    {
      gatherv<Span<const T*>, Span<S*>>({in, sendlen}, {out, 0}, {recvlen, size()}, {displ, size()}, root);
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::gatherv(const T&,S&,Span<int*>,Span<int*>,int)const
    template<typename T, typename S>
    void gatherv (const T& in, S& out, Span<int*> recvlen, Span<int*> displ, int root) const
    {
      Span<const T> span_in(in);
      Span<S> span_out(out);
      dune_mpi_call(MPI_Gatherv, span_in.ptr(), span_in.size(),
                    span_in.mpi_type(), span_out.ptr(),
                    recvlen.ptr(), displ.ptr(),
                    span_out.mpi_type(), root,
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::scatter(T*,S*,int,int)const
    template<typename T, typename S>
    DUNE_DEPRECATED
    int scatter (T* send, S* recv, int len, int root) const
    {
      scatter<Span<const T*>, Span<S*>>({send, size()*len}, {recv, len}, root);
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::scatter(const T&, S&, int)const
    template<typename T, typename S>
    void scatter (const T& send, S& recv, int root) const
    {
      Span<const T> span_in(send);
      Span<S> span_out(recv);
      // WARNING: The size argument of the send buffer is the count per process!
      dune_mpi_call(MPI_Scatter, span_in.ptr(), span_out.size(),
                    span_in.mpi_type(), span_out.ptr(),
                    span_out.size(), span_out.mpi_type(),
                    root, communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::iscatter
    template<typename T, typename S>
    FutureType<std::decay_t<S>> iscatter (const T& in, S&& out, int root) const
    {
      Span<const T> span_in(in);
      FutureType<std::decay_t<S>> f(communicator_, false, std::forward<S>(out));
      Span<std::decay_t<S>> span_out(f.buffer());
      // WARNING: The size argument of the send buffer is the count per process!
      dune_mpi_call(MPI_Iscatter, span_in.ptr(), span_out.size(),
                    span_in.mpi_type(), span_out.ptr(), span_out.size(),
                    span_out.mpi_type(), root,
                    communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::scatterv(T*,int*,int*,S*,int,int)const
    template<typename T, typename S>
    DUNE_DEPRECATED
    int scatterv (T* send, int* sendlen, int* displ, S* recv,
                  int recvlen, int root) const
    {
      scatterv<Span<const T*>, Span<S*>>({send, 0}, {sendlen, size()}, {displ, size()},
                         {recv, recvlen}, root);
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::scatterv(const T&, Span<int*>, Span<int*>, S&, int)const
    template<typename T, typename S>
    void scatterv (const T& send, Span<int*> sendlen, Span<int*> displ,
                   S& recv, int root) const
    {
      Span<const T> span_in(send);
      Span<S> span_out(recv);
      dune_mpi_call(MPI_Scatterv, span_in.ptr(), sendlen.ptr(),
                    displ.ptr(),span_in.mpi_type(),
                    span_out.ptr(), span_out.size(),
                    span_out.mpi_type(), root,
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allgather(T*,int,T1*)const
    template<typename T, typename T1>
    DUNE_DEPRECATED
    int allgather(T* sbuf, int count, T1* rbuf) const
    {
      allgather<Span<const T*>, Span<T1*>>({sbuf, count}, {rbuf, size()*count});
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allgather(const T&, T1&)const
    template<typename T, typename T1>
    void allgather(const T& send, T1& recv) const
    {
      Span<const T> span_in(send);
      Span<T1> span_out(recv);
      // WARNING: The size argument of the receive buffer is the count per process!
      dune_mpi_call(MPI_Allgather, span_in.ptr(), span_in.size(),
                    span_in.mpi_type(), span_out.ptr(),
                    span_in.size(), span_out.mpi_type(),
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::iallgather
    template<typename T, typename S>
    FutureType<std::decay_t<S>> iallgather (const T& in, S&& out) const
    {
      Span<const T> span_in(in);
      FutureType<std::decay_t<S>> f(communicator_, false, std::forward<S>(out));
      Span<std::decay_t<S>> span_out(f.buffer());
      // WARNING: The size argument of the receive buffer is the count per process!
      dune_mpi_call(MPI_Iallgather, span_in.ptr(), span_in.size(),
                    span_in.mpi_type(), span_out.ptr(), span_in.size(),
                    span_out.mpi_type(), communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allgatherv(T*,int,S*,int*,int*)const
    template<typename T, typename S>
    DUNE_DEPRECATED
    int allgatherv (T* in, int sendlen, S* out, int* recvlen, int* displ) const
    {
      allgatherv<Span<const T*>, Span<S*>>({in, sendlen}, {out, 0},
                                           {recvlen, size()}, {displ, size()});
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allgatherv(const T&, S&,Span<int*>,Span<int*>)const
    template<typename T, typename S>
    void allgatherv (const T& in, S& out, Span<int*> recvlen, Span<int*> displ) const
    {
      Span<const T> span_in(in);
      Span<S> span_out(out);
      dune_mpi_call(MPI_Allgatherv, span_in.ptr(), span_in.size(),
                    span_in.mpi_type(), span_out.ptr(),
                    recvlen.ptr(), displ.ptr(), span_out.mpi_type(),
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allreduce(T*,int)const
    template<typename BinaryFunction, typename T>
    DUNE_DEPRECATED
    int allreduce(T* inout, int len) const
    {
      allreduce<BinaryFunction, Span<T*>>({inout, len});
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allreduce(T&)const
    template<typename BinaryFunction, typename T>
    void allreduce(T& inout) const
    {
      Span<T> span(inout);
      dune_mpi_call(MPI_Allreduce, MPI_IN_PLACE, span.ptr(), span.size(),
                    span.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::iallreduce(T&&)const
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iallreduce (T&& data) const
    {
      FutureType<std::decay_t<T>> f(communicator_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(f.buffer());
      dune_mpi_call(MPI_Iallreduce, MPI_IN_PLACE, span.ptr(),
                    span.size(), span.mpi_type(),
                    (Generic_MPI_Op<typename decltype(span)::type, BinaryFunction>::get()),
                    communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allreduce(T*,T*,int)const
    template<typename BinaryFunction, typename T>
    DUNE_DEPRECATED
    int allreduce(T* in, T* out, int len) const
    {
      allreduce<BinaryFunction, Span<T*>>({in, len}, {out, len});
      return 0;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::allreduce(const T&,T&)const
    template<typename BinaryFunction, typename T>
    void allreduce(const T& in, T& out) const
    {
      Span<const T> span_in(in);
      Span<T> span_out(out);
      dune_mpi_call(MPI_Allreduce, span_in.ptr(), span_out.ptr(),
                    span_in.size(), span_in.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::iallreduce
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iallreduce (const T& in, T&& out) const
    {
      Span<const T> span_in(in);
      FutureType<std::decay_t<T>> f(communicator_, false, std::forward<T>(out));
      Span<std::decay_t<T>> span_out(f.buffer());
      dune_mpi_call(MPI_Iallreduce, span_in.ptr(), span_out.ptr(), span_out.size(),
                    span_in.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::scan
    template<typename BinaryFunction, typename T>
    void scan (const T& in, T& out) const
    {
      Span<const T> span_in(in);
      Span<T> span_out(out);
      dune_mpi_call(MPI_Scan, span_in.ptr(), span_out.ptr(),
                    span_in.size(), span_in.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::iscan
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iscan (const T& in, T&& out) const
    {
      Span<const T> span_in(in);
      FutureType<std::decay_t<T>> f(communicator_, false, std::forward<T>(out));
      Span<T> span_out(f.buffer());
      dune_mpi_call(MPI_Iscan, span_in.ptr(), span_out.ptr(),
                    span_in.size(), span_in.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_, &f.mpirequest());
      return f;
    }

    //! @copydoc CollectiveCommunication<No_Comm>::exscan
    template<typename BinaryFunction, typename T>
    void exscan (const T& in, T& out) const
    {
      Span<const T> span_in(in);
      Span<T> span_out(out);
      dune_mpi_call(MPI_Exscan, span_in.ptr(), span_out.ptr(),
                    span_in.size(), span_in.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_);
    }

    //! @copydoc CollectiveCommunication<No_Comm>::iexscan
    template<typename BinaryFunction, typename T>
    FutureType<std::decay_t<T>> iexscan (const T& in, T&& out) const
    {
      Span<const T> span_in(in);
      FutureType<std::decay_t<T>> f(communicator_, false, std::forward<T>(out));
      Span<std::decay_t<T>> span_out(f.buffer());
      dune_mpi_call(MPI_Iexscan, span_in.ptr(), span_out.ptr(), span_in.size(),
                    span_in.mpi_type(),
                    (Generic_MPI_Op<typename Span<T>::type, BinaryFunction>::get()),
                    communicator_, &f.mpirequest());
      return f;
    }

  private:
    Comm communicator_;
  };

  template<>
  class CollectiveCommunication<ManagedMPIComm> :
    public GenericMPICollectiveCommunication<ManagedMPIComm>
  {
  public:
    CollectiveCommunication(const ManagedMPIComm& c = ManagedMPIComm::comm_world()) :
      GenericMPICollectiveCommunication(c) {}
  };
} // namespace dune

#endif
#endif
