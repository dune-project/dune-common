// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MPICOLLECTIVECOMMUNICATION_HH
#define DUNE_MPICOLLECTIVECOMMUNICATION_HH

/*!
   \file
   \brief Implements an utility class that provides
   MPI's collective communication methods.

   \ingroup ParallelCommunication
 */

#if HAVE_MPI

#include <algorithm>
#include <functional>
#include <memory>

#include <mpi.h>

#include <dune/common/binaryfunctions.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/parallel/collectivecommunication.hh>
#include <dune/common/parallel/mpitraits.hh>
#include <dune/common/parallel/mpifuture.hh>

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
        MPI_Op_create((void (*)(void*, void*, int*, MPI_Datatype*))&operation,true,op.get());
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

#define ComposeMPIOp(func,op)                   \
  template<class T, class S>                             \
  class Generic_MPI_Op<T, func<S> >{             \
  public:\
    static MPI_Op get(){ \
      return op; \
    } \
  private:\
    Generic_MPI_Op () {}\
    Generic_MPI_Op (const Generic_MPI_Op & ) {}\
  }


  ComposeMPIOp(std::plus, MPI_SUM);
  ComposeMPIOp(std::multiplies, MPI_PROD);
  ComposeMPIOp(Min, MPI_MIN);
  ComposeMPIOp(Max, MPI_MAX);

#undef ComposeMPIOp


  //=======================================================
  // use singleton pattern and template specialization to
  // generate MPI operations
  //=======================================================

  /*! \brief Specialization of CollectiveCommunication for MPI
        \ingroup ParallelCommunication
   */
  template<>
  class CollectiveCommunication<MPI_Comm>
  {
  public:
    //! Instantiation using a MPI communicator
    CollectiveCommunication (const MPI_Comm& c = MPI_COMM_WORLD)
      : communicator(c)
    {
      if(communicator!=MPI_COMM_NULL) {
        int initialized = 0;
        MPI_Initialized(&initialized);
        if (!initialized)
          DUNE_THROW(ParallelError,"You must call MPIHelper::instance(argc,argv) in your main() function before using the MPI CollectiveCommunication!");
        MPI_Comm_rank(communicator,&me);
        MPI_Comm_size(communicator,&procs);
      }else{
        procs=0;
        me=-1;
      }
    }

    //! @copydoc CollectiveCommunication::rank
    int rank () const
    {
      return me;
    }

    //! @copydoc CollectiveCommunication::size
    int size () const
    {
      return procs;
    }

    template<class T>
    int send(const T& data, int dest_rank, int tag){
      MPIData<const T&> mpi_data(data);
      return MPI_Send(mpi_data.ptr(), mpi_data.size(), mpi_data.type(),
                      dest_rank, tag, communicator);
    }

    template<class T>
    MPIFuture<T> isend(T&& data, int dest_rank, int tag){
      MPIFuture<T> future(std::forward<T>(data));
      MPI_Isend(future.data_->ptr(), future.data_->size(), future.data_->type(),
                       dest_rank, tag, communicator, &future.req_);
      return std::move(future);
    }

    template<class T>
    T recv(T&& data, int source_rank, int tag, MPI_Status* status = MPI_STATUS_IGNORE){
      MPIData<T> mpi_data(std::forward<T>(data));
      MPI_Recv(mpi_data.ptr(), mpi_data.size(), mpi_data.type(),
                      source_rank, tag, communicator, status);
      return std::forward<T>(mpi_data.get());
    }

    template<class T>
    MPIFuture<T> irecv(T&& data, int source_rank, int tag){
      MPIFuture<T> future(std::forward<T>(data));
      MPI_Irecv(future.data_->ptr(), future.data_->size(), future.data_->type(),
                             source_rank, tag, communicator, &future.req_);
      return std::move(future);
    }

    //! @copydoc CollectiveCommunication::sum
    template<typename T>
    T sum (const T& in) const
    {
      T out;
      allreduce<std::plus<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc CollectiveCommunication::sum
    template<typename T>
    int sum (T* inout, int len) const
    {
      return allreduce<std::plus<T> >(inout,len);
    }

    //! @copydoc CollectiveCommunication::prod
    template<typename T>
    T prod (const T& in) const
    {
      T out;
      allreduce<std::multiplies<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc CollectiveCommunication::prod
    template<typename T>
    int prod (T* inout, int len) const
    {
      return allreduce<std::multiplies<T> >(inout,len);
    }

    //! @copydoc CollectiveCommunication::min
    template<typename T>
    T min (const T& in) const
    {
      T out;
      allreduce<Min<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc CollectiveCommunication::min
    template<typename T>
    int min (T* inout, int len) const
    {
      return allreduce<Min<T> >(inout,len);
    }


    //! @copydoc CollectiveCommunication::max
    template<typename T>
    T max (const T& in) const
    {
      T out;
      allreduce<Max<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc CollectiveCommunication::max
    template<typename T>
    int max (T* inout, int len) const
    {
      return allreduce<Max<T> >(inout,len);
    }

    //! @copydoc CollectiveCommunication::barrier
    int barrier () const
    {
      return MPI_Barrier(communicator);
    }

    //! @copydoc CollectiveCommunication::barrier
    MPIFuture<void> ibarrier () const
    {
      MPIFuture<void> future(true);
      MPI_Ibarrier(communicator, &future.req_);
      return future;
    }


    //! @copydoc CollectiveCommunication::broadcast
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return MPI_Bcast(inout,len,MPITraits<T>::getType(),root,communicator);
    }

    //! @copydoc CollectiveCommunication::ibroadcast
    template<class T>
    MPIFuture<T> ibroadcast(T&& data, int root) const{
      MPIFuture<T> future(std::forward<T>(data));
      MPI_Ibcast(future.data_->ptr(),
                 future.data_->size(),
                 future.data_->type(),
                 root,
                 communicator,
                 &future.req_);
      return std::move(future);
    }

    //! @copydoc CollectiveCommunication::gather()
    //! @note out must have space for P*len elements
    template<typename T>
    int gather (const T* in, T* out, int len, int root) const
    {
      return MPI_Gather(const_cast<T*>(in),len,MPITraits<T>::getType(),
                        out,len,MPITraits<T>::getType(),
                        root,communicator);
    }

    template<class TIN, class TOUT = std::vector<TIN>>
    MPIFuture<TOUT, TIN> igather(TIN&& data_in, TOUT&& data_out, int root){
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      assert(root != me || future.send_data_->size()*procs <= future.data_->size());
      int outlen = me==root * future.send_data_->size();
      MPI_Igather(future.send_data_->ptr(), future.send_data_->size(), future.send_data_->type(),
                  future.data_->ptr(), outlen, future.data_->type(),
                  root, communicator, &future.req_);
      return std::move(future);
    }

    //! @copydoc CollectiveCommunication::gatherv()
    template<typename T>
    int gatherv (const T* in, int sendlen, T* out, int* recvlen, int* displ, int root) const
    {
      return MPI_Gatherv(const_cast<T*>(in),sendlen,MPITraits<T>::getType(),
                         out,recvlen,displ,MPITraits<T>::getType(),
                         root,communicator);
    }

    //! @copydoc CollectiveCommunication::scatter()
    //! @note out must have space for P*len elements
    template<typename T>
    int scatter (const T* send, T* recv, int len, int root) const
    {
      return MPI_Scatter(const_cast<T*>(send),len,MPITraits<T>::getType(),
                         recv,len,MPITraits<T>::getType(),
                         root,communicator);
    }

    template<class TIN, class TOUT = TIN>
    MPIFuture<TOUT, TIN> iscatter(TIN&& data_in, TOUT&& data_out, int root){
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      int inlen = me==root * future.send_data_->size();
      MPI_Iscatter(future.send_data_->ptr(), inlen, future.send_data_->type(),
                  future.data_->ptr(), future.data_->size(), future.data_->type(),
                  root, communicator, &future.req_);
      return std::move(future);
    }

    //! @copydoc CollectiveCommunication::scatterv()
    template<typename T>
    int scatterv (const T* send, int* sendlen, int* displ, T* recv, int recvlen, int root) const
    {
      return MPI_Scatterv(const_cast<T*>(send),sendlen,displ,MPITraits<T>::getType(),
                          recv,recvlen,MPITraits<T>::getType(),
                          root,communicator);
    }


    operator MPI_Comm () const
    {
      return communicator;
    }

    //! @copydoc CollectiveCommunication::allgather()
    template<typename T, typename T1>
    int allgather(const T* sbuf, int count, T1* rbuf) const
    {
      return MPI_Allgather(const_cast<T*>(sbuf), count, MPITraits<T>::getType(),
                           rbuf, count, MPITraits<T1>::getType(),
                           communicator);
    }

    template<class TIN, class TOUT = TIN>
    MPIFuture<TOUT, TIN> iallgather(TIN&& data_in, TOUT&& data_out){
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      assert(future.send_data_->size()*procs <= future.data_->size());
      int outlen = future.send_data_->size();
      MPI_Iallgather(future.send_data_->ptr(), future.send_data_->size(), future.send_data_->type(),
                  future.data_->ptr(), outlen, future.data_->type(),
                  communicator, &future.req_);
      return std::move(future);
    }

    //! @copydoc CollectiveCommunication::allgatherv()
    template<typename T>
    int allgatherv (const T* in, int sendlen, T* out, int* recvlen, int* displ) const
    {
      return MPI_Allgatherv(const_cast<T*>(in),sendlen,MPITraits<T>::getType(),
                            out,recvlen,displ,MPITraits<T>::getType(),
                            communicator);
    }

    //! @copydoc CollectiveCommunication::allreduce(Type* inout,int len) const
    template<typename BinaryFunction, typename Type>
    int allreduce(Type* inout, int len) const
    {
      Type* out = new Type[len];
      int ret = allreduce<BinaryFunction>(inout,out,len);
      std::copy(out, out+len, inout);
      delete[] out;
      return ret;
    }

    template<class BinaryFunction, class TIN, class TOUT = TIN>
    MPIFuture<TOUT, TIN> iallreduce(TIN&& data_in, TOUT&& data_out){
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      assert(future.data_->size() == future.send_data_->size());
      assert(future.data_->type() == future.send_data_->type());
      MPI_Iallreduce(future.send_data_->ptr(), future.data_->ptr(),
                     future.data_->size(), future.data_->type(),
                     (Generic_MPI_Op<TIN, BinaryFunction>::get()),
                     communicator, &future.req_);
      return std::move(future);
    }

    template<class BinaryFunction, class T>
    MPIFuture<T> iallreduce(T&& data){
      MPIFuture<T> future(std::forward<T>(data));
      MPI_Iallreduce(MPI_IN_PLACE, future.data_->ptr(),
                     future.data_->size(), future.data_->type(),
                     (Generic_MPI_Op<T, BinaryFunction>::get()),
                     communicator, &future.req_);
      return std::move(future);
    }

    //! @copydoc CollectiveCommunication::allreduce(Type* in,Type* out,int len) const
    template<typename BinaryFunction, typename Type>
    int allreduce(const Type* in, Type* out, int len) const
    {
      return MPI_Allreduce(const_cast<Type*>(in), out, len, MPITraits<Type>::getType(),
                           (Generic_MPI_Op<Type, BinaryFunction>::get()),communicator);
    }

  private:
    MPI_Comm communicator;
    int me;
    int procs;
  };
} // namespace dune

#endif // HAVE_MPI

#endif
