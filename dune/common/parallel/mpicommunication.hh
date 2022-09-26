// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_PARALLEL_MPICOMMUNICATION_HH
#define DUNE_COMMON_PARALLEL_MPICOMMUNICATION_HH

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
#include <dune/common/parallel/communication.hh>
#include <dune/common/parallel/mpitraits.hh>
#include <dune/common/parallel/mpifuture.hh>
#include <dune/common/parallel/mpidata.hh>

namespace Dune
{

  //=======================================================
  // use singleton pattern and template specialization to
  // generate MPI operations
  //=======================================================

  template<typename Type, typename BinaryFunction, typename Enable=void>
  class Generic_MPI_Op
  {

  public:
    static MPI_Op get ()
    {
      if (!op)
      {
        op = std::make_unique<MPI_Op>();
        // The following line leaks an MPI operation object, because the corresponding
        //`MPI_Op_free` is never called.  It is never called because there is no easy
        // way to call it at the right moment: right before the call to MPI_Finalize.
        // See https://gitlab.dune-project.org/core/dune-istl/issues/80
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
    static std::unique_ptr<MPI_Op> op;
  };


  template<typename Type, typename BinaryFunction, typename Enable>
  std::unique_ptr<MPI_Op> Generic_MPI_Op<Type,BinaryFunction, Enable>::op;

#define ComposeMPIOp(func,op)                                           \
  template<class T, class S>                                            \
  class Generic_MPI_Op<T, func<S>, std::enable_if_t<MPITraits<S>::is_intrinsic> >{ \
  public:                                                               \
  static MPI_Op get(){                                                  \
    return op;                                                          \
  }                                                                     \
  private:                                                              \
  Generic_MPI_Op () {}                                                  \
  Generic_MPI_Op (const Generic_MPI_Op & ) {}                           \
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

  /*! \brief Specialization of Communication for MPI
        \ingroup ParallelCommunication
   */
  template<>
  class Communication<MPI_Comm>
  {
  public:
    //! Instantiation using a MPI communicator
    Communication (const MPI_Comm& c = MPI_COMM_WORLD)
      : communicator(c)
    {
      if(communicator!=MPI_COMM_NULL) {
        int initialized = 0;
        MPI_Initialized(&initialized);
        if (!initialized)
          DUNE_THROW(ParallelError,"You must call MPIHelper::instance(argc,argv) in your main() function before using the MPI Communication!");
        MPI_Comm_rank(communicator,&me);
        MPI_Comm_size(communicator,&procs);
      }else{
        procs=0;
        me=-1;
      }
    }

    //! @copydoc Communication::rank
    int rank () const
    {
      return me;
    }

    //! @copydoc Communication::size
    int size () const
    {
      return procs;
    }

    //! @copydoc Communication::send
    template<class T>
    int send(const T& data, int dest_rank, int tag) const
    {
      auto mpi_data = getMPIData(data);
      return MPI_Send(mpi_data.ptr(), mpi_data.size(), mpi_data.type(),
                      dest_rank, tag, communicator);
    }

    //! @copydoc Communication::isend
    template<class T>
    MPIFuture<const T> isend(const T&& data, int dest_rank, int tag) const
    {
      MPIFuture<const T> future(std::forward<const T>(data));
      auto mpidata = future.get_mpidata();
      MPI_Isend(mpidata.ptr(), mpidata.size(), mpidata.type(),
                       dest_rank, tag, communicator, &future.req_);
      return future;
    }

    //! @copydoc Communication::recv
    template<class T>
    T recv(T&& data, int source_rank, int tag, MPI_Status* status = MPI_STATUS_IGNORE) const
    {
      T lvalue_data(std::forward<T>(data));
      auto mpi_data = getMPIData(lvalue_data);
      MPI_Recv(mpi_data.ptr(), mpi_data.size(), mpi_data.type(),
                      source_rank, tag, communicator, status);
      return lvalue_data;
    }

    //! @copydoc Communication::irecv
    template<class T>
    MPIFuture<T> irecv(T&& data, int source_rank, int tag) const
    {
      MPIFuture<T> future(std::forward<T>(data));
      auto mpidata = future.get_mpidata();
      MPI_Irecv(mpidata.ptr(), mpidata.size(), mpidata.type(),
                             source_rank, tag, communicator, &future.req_);
      return future;
    }

    template<class T>
    T rrecv(T&& data, int source_rank, int tag, MPI_Status* status = MPI_STATUS_IGNORE) const
    {
      MPI_Status _status;
      MPI_Message _message;
      T lvalue_data(std::forward<T>(data));
      auto mpi_data = getMPIData(lvalue_data);
      static_assert(!mpi_data.static_size, "rrecv work only for non-static-sized types.");
      if(status == MPI_STATUS_IGNORE)
        status = &_status;
      MPI_Mprobe(source_rank, tag, communicator, &_message, status);
      int size;
      MPI_Get_count(status, mpi_data.type(), &size);
      mpi_data.resize(size);
      MPI_Mrecv(mpi_data.ptr(), mpi_data.size(), mpi_data.type(), &_message, status);
      return lvalue_data;
    }

    //! @copydoc Communication::sum
    template<typename T>
    T sum (const T& in) const
    {
      T out;
      allreduce<std::plus<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc Communication::sum
    template<typename T>
    int sum (T* inout, int len) const
    {
      return allreduce<std::plus<T> >(inout,len);
    }

    //! @copydoc Communication::prod
    template<typename T>
    T prod (const T& in) const
    {
      T out;
      allreduce<std::multiplies<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc Communication::prod
    template<typename T>
    int prod (T* inout, int len) const
    {
      return allreduce<std::multiplies<T> >(inout,len);
    }

    //! @copydoc Communication::min
    template<typename T>
    T min (const T& in) const
    {
      T out;
      allreduce<Min<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc Communication::min
    template<typename T>
    int min (T* inout, int len) const
    {
      return allreduce<Min<T> >(inout,len);
    }


    //! @copydoc Communication::max
    template<typename T>
    T max (const T& in) const
    {
      T out;
      allreduce<Max<T> >(&in,&out,1);
      return out;
    }

    //! @copydoc Communication::max
    template<typename T>
    int max (T* inout, int len) const
    {
      return allreduce<Max<T> >(inout,len);
    }

    //! @copydoc Communication::barrier
    int barrier () const
    {
      return MPI_Barrier(communicator);
    }

    //! @copydoc Communication::ibarrier
    MPIFuture<void> ibarrier () const
    {
      MPIFuture<void> future(true); // make a valid MPIFuture<void>
      MPI_Ibarrier(communicator, &future.req_);
      return future;
    }


    //! @copydoc Communication::broadcast
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return MPI_Bcast(inout,len,MPITraits<T>::getType(),root,communicator);
    }

    //! @copydoc Communication::ibroadcast
    template<class T>
    MPIFuture<T> ibroadcast(T&& data, int root) const{
      MPIFuture<T> future(std::forward<T>(data));
      auto mpidata = future.get_mpidata();
      MPI_Ibcast(mpidata.ptr(),
                 mpidata.size(),
                 mpidata.type(),
                 root,
                 communicator,
                 &future.req_);
      return future;
    }

    //! @copydoc Communication::gather()
    //! @note out must have space for P*len elements
    template<typename T>
    int gather (const T* in, T* out, int len, int root) const
    {
      return MPI_Gather(const_cast<T*>(in),len,MPITraits<T>::getType(),
                        out,len,MPITraits<T>::getType(),
                        root,communicator);
    }

    //! @copydoc Communication::igather
    template<class TIN, class TOUT = std::vector<TIN>>
    MPIFuture<TOUT, TIN> igather(TIN&& data_in, TOUT&& data_out, int root) const{
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      auto mpidata_in = future.get_send_mpidata();
      auto mpidata_out = future.get_mpidata();
      assert(root != me || mpidata_in.size()*procs <= mpidata_out.size());
      int outlen = (me==root) * mpidata_in.size();
      MPI_Igather(mpidata_in.ptr(), mpidata_in.size(), mpidata_in.type(),
                  mpidata_out.ptr(), outlen, mpidata_out.type(),
                  root, communicator, &future.req_);
      return future;
    }

    //! @copydoc Communication::gatherv()
    template<typename T>
    int gatherv (const T* in, int sendDataLen, T* out, int* recvDataLen, int* displ, int root) const
    {
      return MPI_Gatherv(const_cast<T*>(in),sendDataLen,MPITraits<T>::getType(),
                         out,recvDataLen,displ,MPITraits<T>::getType(),
                         root,communicator);
    }

    //! @copydoc Communication::scatter()
    //! @note out must have space for P*len elements
    template<typename T>
    int scatter (const T* sendData, T* recvData, int len, int root) const
    {
      return MPI_Scatter(const_cast<T*>(sendData),len,MPITraits<T>::getType(),
                         recvData,len,MPITraits<T>::getType(),
                         root,communicator);
    }

    //! @copydoc Communication::iscatter
    template<class TIN, class TOUT = TIN>
    MPIFuture<TOUT, TIN> iscatter(TIN&& data_in, TOUT&& data_out, int root) const
    {
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      auto mpidata_in = future.get_send_mpidata();
      auto mpidata_out = future.get_mpidata();
      int inlen = (me==root) * mpidata_in.size()/procs;
      MPI_Iscatter(mpidata_in.ptr(), inlen, mpidata_in.type(),
                  mpidata_out.ptr(), mpidata_out.size(), mpidata_out.type(),
                  root, communicator, &future.req_);
      return future;
    }

    //! @copydoc Communication::scatterv()
    template<typename T>
    int scatterv (const T* sendData, int* sendDataLen, int* displ, T* recvData, int recvDataLen, int root) const
    {
      return MPI_Scatterv(const_cast<T*>(sendData),sendDataLen,displ,MPITraits<T>::getType(),
                          recvData,recvDataLen,MPITraits<T>::getType(),
                          root,communicator);
    }


    operator MPI_Comm () const
    {
      return communicator;
    }

    //! @copydoc Communication::allgather()
    template<typename T, typename T1>
    int allgather(const T* sbuf, int count, T1* rbuf) const
    {
      return MPI_Allgather(const_cast<T*>(sbuf), count, MPITraits<T>::getType(),
                           rbuf, count, MPITraits<T1>::getType(),
                           communicator);
    }

    //! @copydoc Communication::iallgather
    template<class TIN, class TOUT = TIN>
    MPIFuture<TOUT, TIN> iallgather(TIN&& data_in, TOUT&& data_out) const
    {
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      auto mpidata_in = future.get_send_mpidata();
      auto mpidata_out = future.get_mpidata();
      assert(mpidata_in.size()*procs <= mpidata_out.size());
      int outlen = mpidata_in.size();
      MPI_Iallgather(mpidata_in.ptr(), mpidata_in.size(), mpidata_in.type(),
                  mpidata_out.ptr(), outlen, mpidata_out.type(),
                  communicator, &future.req_);
      return future;
    }

    //! @copydoc Communication::allgatherv()
    template<typename T>
    int allgatherv (const T* in, int sendDataLen, T* out, int* recvDataLen, int* displ) const
    {
      return MPI_Allgatherv(const_cast<T*>(in),sendDataLen,MPITraits<T>::getType(),
                            out,recvDataLen,displ,MPITraits<T>::getType(),
                            communicator);
    }

    //! @copydoc Communication::allreduce(Type* inout,int len) const
    template<typename BinaryFunction, typename Type>
    int allreduce(Type* inout, int len) const
    {
      Type* out = new Type[len];
      int ret = allreduce<BinaryFunction>(inout,out,len);
      std::copy(out, out+len, inout);
      delete[] out;
      return ret;
    }

    template<typename BinaryFunction, typename Type>
    Type allreduce(Type&& in) const{
      Type lvalue_data = std::forward<Type>(in);
      auto data = getMPIData(lvalue_data);
      MPI_Allreduce(MPI_IN_PLACE, data.ptr(), data.size(), data.type(),
                    (Generic_MPI_Op<Type, BinaryFunction>::get()),
                    communicator);
      return lvalue_data;
    }

    //! @copydoc Communication::iallreduce
    template<class BinaryFunction, class TIN, class TOUT = TIN>
    MPIFuture<TOUT, TIN> iallreduce(TIN&& data_in, TOUT&& data_out) const {
      MPIFuture<TOUT, TIN> future(std::forward<TOUT>(data_out), std::forward<TIN>(data_in));
      auto mpidata_in = future.get_send_mpidata();
      auto mpidata_out = future.get_mpidata();
      assert(mpidata_out.size() == mpidata_in.size());
      assert(mpidata_out.type() == mpidata_in.type());
      MPI_Iallreduce(mpidata_in.ptr(), mpidata_out.ptr(),
                     mpidata_out.size(), mpidata_out.type(),
                     (Generic_MPI_Op<TIN, BinaryFunction>::get()),
                     communicator, &future.req_);
      return future;
    }

    //! @copydoc Communication::iallreduce
    template<class BinaryFunction, class T>
    MPIFuture<T> iallreduce(T&& data) const{
      MPIFuture<T> future(std::forward<T>(data));
      auto mpidata = future.get_mpidata();
      MPI_Iallreduce(MPI_IN_PLACE, mpidata.ptr(),
                     mpidata.size(), mpidata.type(),
                     (Generic_MPI_Op<T, BinaryFunction>::get()),
                     communicator, &future.req_);
      return future;
    }

    //! @copydoc Communication::allreduce(Type* in,Type* out,int len) const
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
