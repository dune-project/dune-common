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

#ifndef DISABLE_ULFM
#ifdef OMPI_MPI_H //Openmpi
#include <mpi-ext.h>
#endif
#ifdef HAVE_BLACKCHANNEL
#include <blackchannel-ulfm.h>
#endif
#endif

#include <dune/common/binaryfunctions.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/parallel/collectivecommunication.hh>
#include <dune/common/parallel/mpitraits.hh>
#include <dune/common/stdstreams.hh>

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
  protected:
    // deleter function to be used with shared_ptr
    static void freeComm(MPI_Comm* c)
    {
      if (c != nullptr &&
          *c != MPI_COMM_WORLD &&
          *c != MPI_COMM_SELF &&
          *c != MPI_COMM_NULL)
      {
        int wasFinalized = 0;
        MPI_Finalized(&wasFinalized);
        if (!wasFinalized){
          int res = MPI_Comm_free(c);
          if(res == MPI_SUCCESS)
            *c = MPI_COMM_NULL;
          else
            dverb << "Cannot free MPI_Comm (" << *c << ")" << std::endl;
        }
      }
    }

  public:
    //! Instantiation using a MPI communicator
    CollectiveCommunication (const MPI_Comm& c = MPI_COMM_WORLD)
      : p_communicator(new MPI_Comm(c), freeComm)
    {
      if(*p_communicator!=MPI_COMM_NULL) {
        int initialized = 0;
        MPI_Initialized(&initialized);
        if (!initialized)
          DUNE_THROW(ParallelError,"You must call MPIHelper::instance(argc,argv) in your main() function before using the MPI CollectiveCommunication!");
        MPI_Comm_rank(*p_communicator,&me);
        MPI_Comm_size(*p_communicator,&procs);
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
      return MPI_Barrier(*p_communicator);
    }

    //! @copydoc CollectiveCommunication::broadcast
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return MPI_Bcast(inout,len,MPITraits<T>::getType(),root,*p_communicator);
    }

    //! @copydoc CollectiveCommunication::gather()
    //! @note out must have space for P*len elements
    template<typename T>
    int gather (const T* in, T* out, int len, int root) const
    {
      return MPI_Gather(const_cast<T*>(in),len,MPITraits<T>::getType(),
                        out,len,MPITraits<T>::getType(),
                        root,*p_communicator);
    }

    //! @copydoc CollectiveCommunication::gatherv()
    template<typename T>
    int gatherv (const T* in, int sendlen, T* out, int* recvlen, int* displ, int root) const
    {
      return MPI_Gatherv(const_cast<T*>(in),sendlen,MPITraits<T>::getType(),
                         out,recvlen,displ,MPITraits<T>::getType(),
                         root,*p_communicator);
    }

    //! @copydoc CollectiveCommunication::scatter()
    //! @note out must have space for P*len elements
    template<typename T>
    int scatter (const T* send, T* recv, int len, int root) const
    {
      return MPI_Scatter(const_cast<T*>(send),len,MPITraits<T>::getType(),
                         recv,len,MPITraits<T>::getType(),
                         root,*p_communicator);
    }

    //! @copydoc CollectiveCommunication::scatterv()
    template<typename T>
    int scatterv (const T* send, int* sendlen, int* displ, T* recv, int recvlen, int root) const
    {
      return MPI_Scatterv(const_cast<T*>(send),sendlen,displ,MPITraits<T>::getType(),
                          recv,recvlen,MPITraits<T>::getType(),
                          root,*p_communicator);
    }


    operator MPI_Comm () const
    {
      return *p_communicator;
    }

    //! @copydoc CollectiveCommunication::allgather()
    template<typename T, typename T1>
    int allgather(const T* sbuf, int count, T1* rbuf) const
    {
      return MPI_Allgather(const_cast<T*>(sbuf), count, MPITraits<T>::getType(),
                           rbuf, count, MPITraits<T1>::getType(),
                           *p_communicator);
    }

    //! @copydoc CollectiveCommunication::allgatherv()
    template<typename T>
    int allgatherv (const T* in, int sendlen, T* out, int* recvlen, int* displ) const
    {
      return MPI_Allgatherv(const_cast<T*>(in),sendlen,MPITraits<T>::getType(),
                            out,recvlen,displ,MPITraits<T>::getType(),
                            *p_communicator);
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

    //! @copydoc CollectiveCommunication::allreduce(Type* in,Type* out,int len) const
    template<typename BinaryFunction, typename Type>
    int allreduce(const Type* in, Type* out, int len) const
    {
      return MPI_Allreduce(const_cast<Type*>(in), out, len, MPITraits<Type>::getType(),
                           (Generic_MPI_Op<Type, BinaryFunction>::get()),*p_communicator);
    }

#if HAVE_ULFM_REVOKE
    int revoke()
    {
      return MPIX_Comm_revoke(*p_communicator);
    }

    int shrink()
    {
      MPI_Comm oldcomm = *p_communicator;
      int result = MPIX_Comm_shrink(oldcomm, p_communicator.get());
      if(result == MPI_SUCCESS){
        freeComm(&oldcomm);
      }else{
        *p_communicator = oldcomm;
      }
      return result;
    }

    int agree(int& i) const{
      return MPIX_Comm_agree(*p_communicator, &i);
    }
#endif
  private:
    std::shared_ptr<MPI_Comm> p_communicator;
    int me;
    int procs;
  };
} // namespace dune

#endif // HAVE_MPI

#endif
