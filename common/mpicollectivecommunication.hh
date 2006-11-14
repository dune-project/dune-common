// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MPICOLLECTIVECOMMUNICATION_HH
#define DUNE_MPICOLLECTIVECOMMUNICATION_HH

#include <iostream>
#include <complex>
#include <algorithm>

#include "exceptions.hh"
#include "collectivecommunication.hh"

// MPI header
#include <mpi.h>


namespace Dune
{
  //=======================================================
  // use singleton pattern and template specialization to
  // generate MPI data types
  //=======================================================

  // any type is interpreted as contiguous piece of memory
  // i.e. only value types are allowed !
  template<typename T>
  class Generic_MPI_Datatype
  {
  public:
    static MPI_Datatype get ()
    {
      if (type==0)
      {
        type = new MPI_Datatype;
        MPI_Type_contiguous(sizeof(T),MPI_BYTE,type);
        MPI_Type_commit(type);
      }
      return *type;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
    static MPI_Datatype *type;
  };

  template<typename T>
  MPI_Datatype* Generic_MPI_Datatype<T>::type = 0;

  // A Macro for defining traits for the primitive data types
#define ComposeMPITraits(p,m) \
  template<> \
  struct Generic_MPI_Datatype<p>{ \
    static inline MPI_Datatype get(){ \
      return m; \
    } \
  };


  ComposeMPITraits(char, MPI_CHAR);
  ComposeMPITraits(unsigned char,MPI_UNSIGNED_CHAR);
  ComposeMPITraits(short,MPI_SHORT);
  ComposeMPITraits(unsigned short,MPI_UNSIGNED_SHORT);
  ComposeMPITraits(int,MPI_INT);
  ComposeMPITraits(unsigned int,MPI_UNSIGNED);
  ComposeMPITraits(long,MPI_LONG);
  ComposeMPITraits(unsigned long,MPI_UNSIGNED_LONG);
  ComposeMPITraits(float,MPI_FLOAT);
  ComposeMPITraits(double,MPI_DOUBLE);
  ComposeMPITraits(long double,MPI_LONG_DOUBLE);

#undef ComposeMPITraits


  //=======================================================
  // use singleton pattern and template specialization to
  // generate MPI operations
  //=======================================================

  //===========
  // sum
  //===========

  // the general case, assumes operator+ for type T
  template<typename T>
  class GenericSum_MPI_Op
  {
  public:
    static MPI_Op get ()
    {
      if (op==0)
      {
        op = new MPI_Op;
        MPI_Op_create((void (*)(void*, void*, int*, MPI_Datatype*))&operation,true,op);
      }
      return *op;
    }
  private:
    static void operation (T *in, T *inout, int *len, MPI_Datatype *dptr)
    {
      for (int i=0; i< *len; ++i)
      {
        T temp;
        temp = (*in)+(*inout);
        *inout = temp;
        in++; inout++;
      }
    }
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
    static MPI_Op* op;
  };

  template<typename T>
  MPI_Op* GenericSum_MPI_Op<T>::op = 0;

  // char
  template<>
  class GenericSum_MPI_Op<char>
  {
  public:
    static MPI_Op get ()
    {
      return MPI_SUM;
    }
  private:
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
  };

  // short
  template<>
  class GenericSum_MPI_Op<short>
  {
  public:
    static MPI_Op get ()
    {
      return MPI_SUM;
    }
  private:
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
  };

  // int
  template<>
  class GenericSum_MPI_Op<int>
  {
  public:
    static MPI_Op get ()
    {
      return MPI_SUM;
    }
  private:
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
  };

  // long int
  template<>
  class GenericSum_MPI_Op<long int>
  {
  public:
    static MPI_Op get ()
    {
      return MPI_SUM;
    }
  private:
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
  };

  // float
  template<>
  class GenericSum_MPI_Op<float>
  {
  public:
    static MPI_Op get ()
    {
      return MPI_SUM;
    }
  private:
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
  };

  // double
  template<>
  class GenericSum_MPI_Op<double>
  {
  public:
    static MPI_Op get ()
    {
      return MPI_SUM;
    }
  private:
    GenericSum_MPI_Op () {}
    GenericSum_MPI_Op (const GenericSum_MPI_Op& ) {}
  };


  //===========
  // product
  //===========

  // the general case, assumes operator+ for type T
  template<typename T>
  class GenericProduct_MPI_Op
  {
  public:
    static MPI_Op get ()
    {
      if (op==0)
      {
        op = new MPI_Op;
        MPI_Op_create((void (*)(void*, void*, int*, MPI_Datatype*))&operation,true,op);
      }
      return *op;
    }
  private:
    static void operation (T *in, T *inout, int *len, MPI_Datatype *dptr)
    {
      for (int i=0; i< *len; ++i)
      {
        T temp;
        temp = (*in)*(*inout);
        *inout = temp;
        in++; inout++;
      }
    }
    GenericProduct_MPI_Op () {}
    GenericProduct_MPI_Op (const GenericProduct_MPI_Op& ) {}
    static MPI_Op* op;
  };

  template<typename T>
  MPI_Op* GenericProduct_MPI_Op<T>::op = 0;


  //===========
  // min
  //===========

  // the general case, assumes operator+ for type T
  template<typename T>
  class GenericMin_MPI_Op
  {
  public:
    static MPI_Op get ()
    {
      if (op==0)
      {
        op = new MPI_Op;
        MPI_Op_create((void (*)(void*, void*, int*, MPI_Datatype*))&operation,true,op);
      }
      return *op;
    }
  private:
    static void operation (T *in, T *inout, int *len, MPI_Datatype *dptr)
    {
      for (int i=0; i< *len; ++i)
      {
        T temp;
        temp = std::min(*in,*inout);
        *inout = temp;
        in++; inout++;
      }
    }
    GenericMin_MPI_Op () {}
    GenericMin_MPI_Op (const GenericMin_MPI_Op& ) {}
    static MPI_Op* op;
  };

  template<typename T>
  MPI_Op* GenericMin_MPI_Op<T>::op = 0;

  //===========
  // max
  //===========

  // the general case, assumes operator+ for type T
  template<typename T>
  class GenericMax_MPI_Op
  {
  public:
    static MPI_Op get ()
    {
      if (op==0)
      {
        op = new MPI_Op;
        MPI_Op_create((void (*)(void*, void*, int*, MPI_Datatype*))&operation,true,op);
      }
      return *op;
    }
  private:
    static void operation (T *in, T *inout, int *len, MPI_Datatype *dptr)
    {
      for (int i=0; i< *len; ++i)
      {
        T temp;
        temp = std::max(*in,*inout);
        *inout = temp;
        in++; inout++;
      }
    }
    GenericMax_MPI_Op () {}
    GenericMax_MPI_Op (const GenericMax_MPI_Op& ) {}
    static MPI_Op* op;
  };

  template<typename T>
  MPI_Op* GenericMax_MPI_Op<T>::op = 0;


  //=======================================================
  // use singleton pattern and template specialization to
  // generate MPI operations
  //=======================================================

  /*! \brief Specialization of CollectiveCommunication for MPI
        \ingroup GICollectiveCommunication
   */
  template<>
  class CollectiveCommunication<MPI_Comm>
  {
  public:
    //! Instantiation using a MPI communicator
    CollectiveCommunication (const MPI_Comm& c)
      : communicator(c)
    {
      MPI_Comm_rank(communicator,&me);
      MPI_Comm_size(communicator,&procs);
    }

    //! @copydoc CollectiveCommunication::rank()
    int rank () const
    {
      return me;
    }

    //! @copydoc CollectiveCommunication::size()
    int size () const
    {
      return procs;
    }

    //! @copydoc CollectiveCommunication::sum(T&)
    template<typename T>
    T sum (T& in) const     // MPI does not know about const :-(
    {
      T out;
      MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                    GenericSum_MPI_Op<T>::get(),communicator);
      return out;
    }

    //! @copydoc CollectiveCommunication::sum(T*,int)
    template<typename T>
    int sum (T* inout, int len) const
    {
      T* in(inout);
      return MPI_Allreduce(in,inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericSum_MPI_Op<T>::get(),communicator);
    }

    //! @copydoc CollectiveCommunication::prod(T&)
    template<typename T>
    T prod (T& in) const     // MPI does not know about const :-(
    {
      T out;
      MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                    GenericProduct_MPI_Op<T>::get(),communicator);
      return out;
    }

    //! @copydoc CollectiveCommunication::prod(T*,int)
    template<typename T>
    int prod (T* inout, int len) const
    {
      T* in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericProduct_MPI_Op<T>::get(),communicator);
    }

    //! @copydoc CollectiveCommunication::min(T&)
    template<typename T>
    T min (T& in) const     // MPI does not know about const :-(
    {
      T out;
      MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                    GenericMin_MPI_Op<T>::get(),communicator);
      return out;
    }

    //! @copydoc CollectiveCommunication::min(T*,int)
    template<typename T>
    int min (T* inout, int len) const
    {
      T* in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericMin_MPI_Op<T>::get(),communicator);
    }

    //! @copydoc CollectiveCommunication::max(T&)
    template<typename T>
    T max (T& in) const     // MPI does not know about const :-(
    {
      T out;
      MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                    GenericMax_MPI_Op<T>::get(),communicator);
      return out;
    }

    //! @copydoc CollectiveCommunication::max(T*,int)
    template<typename T>
    int max (T* inout, int len) const
    {
      T* in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericMax_MPI_Op<T>::get(),communicator);
    }

    //! @copydoc CollectiveCommunication::barrier()
    int barrier () const
    {
      return MPI_Barrier(communicator);
    }

    //! @copydoc CollectiveCommunication::broadcast()
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return MPI_Bcast(inout,len,Generic_MPI_Datatype<T>::get(),root,communicator);
    }


    //! @copydoc CollectiveCommunication::gather()
    template<typename T>
    int gather (T* in, T* out, int len, int root) const     // note out must have space for P*len elements
    {
      return MPI_Gather(in,len,Generic_MPI_Datatype<T>::get(),
                        out,len,Generic_MPI_Datatype<T>::get(),
                        root,communicator);
    }


    operator MPI_Comm () const
    {
      return communicator;
    }

  private:
    MPI_Comm communicator;
    int me;
    int procs;
  };
} // namespace dune

#endif
