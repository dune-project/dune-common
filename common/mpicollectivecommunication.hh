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

  // char
  template<>
  class Generic_MPI_Datatype<char>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_CHAR;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // short
  template<>
  class Generic_MPI_Datatype<short>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_SHORT;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // int
  template<>
  class Generic_MPI_Datatype<int>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_INT;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // long int
  template<>
  class Generic_MPI_Datatype<long int>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_LONG;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // unsigned char
  template<>
  class Generic_MPI_Datatype<unsigned char>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_UNSIGNED_CHAR;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // unsigned short
  template<>
  class Generic_MPI_Datatype<unsigned short>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_UNSIGNED_SHORT;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // unsigned int
  template<>
  class Generic_MPI_Datatype<unsigned int>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_UNSIGNED;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // long int
  template<>
  class Generic_MPI_Datatype<unsigned long int>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_UNSIGNED_LONG;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // float
  template<>
  class Generic_MPI_Datatype<float>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_FLOAT;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // double
  template<>
  class Generic_MPI_Datatype<double>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_DOUBLE;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };

  // long double
  template<>
  class Generic_MPI_Datatype<long double>
  {
  public:
    static MPI_Datatype get ()
    {
      return MPI_LONG_DOUBLE;
    }
  private:
    Generic_MPI_Datatype () {}
    Generic_MPI_Datatype (const Generic_MPI_Datatype& ) {}
  };


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

  // specialization for MPI
  template<>
  class CollectiveCommunication<MPI_Comm>
  {
  public:
    CollectiveCommunication (const MPI_Comm& c)
      : communicator(c)
    {
      MPI_Comm_rank(communicator,&me);
      MPI_Comm_size(communicator,&procs);
    }

    int rank () const
    {
      return me;
    }

    int size () const
    {
      return procs;
    }

    template<typename T>
    T sum (T& in) const     // MPI does not know about const :-(
    {
      T out;
      //int rv=MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
      //			GenericSum_MPI_Op<T>::get(),communicator);
      return out;
    }

    template<typename T>
    int sum (T* inout, int len) const
    {
      T in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericSum_MPI_Op<T>::get(),communicator);
    }

    template<typename T>
    T prod (T& in) const     // MPI does not know about const :-(
    {
      T out;
      int rv=MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                           GenericProduct_MPI_Op<T>::get(),communicator);
      return out;
    }

    template<typename T>
    int prod (T* inout, int len) const
    {
      T in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericProduct_MPI_Op<T>::get(),communicator);
    }

    template<typename T>
    T min (T& in) const     // MPI does not know about const :-(
    {
      T out;
      int rv=MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                           GenericMin_MPI_Op<T>::get(),communicator);
      return out;
    }

    template<typename T>
    int min (T* inout, int len) const
    {
      T in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericMin_MPI_Op<T>::get(),communicator);
    }

    template<typename T>
    T max (T& in) const     // MPI does not know about const :-(
    {
      T out;
      int rv=MPI_Allreduce(&in,&out,1,Generic_MPI_Datatype<T>::get(),
                           GenericMax_MPI_Op<T>::get(),communicator);
      return out;
    }

    template<typename T>
    int max (T* inout, int len) const
    {
      T in(inout);     // copy input
      return MPI_Allreduce(&in,&inout,len,Generic_MPI_Datatype<T>::get(),
                           GenericMax_MPI_Op<T>::get(),communicator);
    }

    int barrier () const
    {
      return MPI_Barrier(communicator);
    }

    //! send array from process with rank root to all others
    template<typename T>
    int broadcast (T* inout, int len, int root) const
    {
      return MPI_Bcast(inout,len,Generic_MPI_Datatype<T>::get(),root,communicator);
    }

    //! receive array of values from each processor in root
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
