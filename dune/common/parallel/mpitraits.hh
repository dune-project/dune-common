// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MPITRAITS_HH
#define DUNE_MPITRAITS_HH

#if HAVE_MPI
#include <mpi.h>
#endif

#include <cstdint>
#include <utility>

namespace Dune
{
  /** @addtogroup ParallelCommunication
   *
   * @{
   */
  /**
   * @file
   * @brief Traits classes for mapping types onto MPI_Datatype.
   * @author Markus Blatt
   */

#if HAVE_MPI
  /**
   * @brief A traits class describing the mapping of types onto MPI_Datatypes.
   *
   * Specializations exist for the default types.
   * Specializations should provide a static method
   * \code
   * static MPI_Datatype getType();
   * \endcode
   */
  template<typename T>
  struct MPITraits
  {
  private:
    MPITraits(){}
    MPITraits(const MPITraits&){}
    static MPI_Datatype datatype;
    static MPI_Datatype vectortype;
  public:
    static inline MPI_Datatype getType()
    {
      if(datatype==MPI_DATATYPE_NULL) {
        MPI_Type_contiguous(sizeof(T),MPI_BYTE,&datatype);
        MPI_Type_commit(&datatype);
      }
      return datatype;
    }

  };
  template<class T>
  MPI_Datatype MPITraits<T>::datatype = MPI_DATATYPE_NULL;

#ifndef DOXYGEN
#if HAVE_MPI

  // A Macro for defining traits for the primitive data types
#define ComposeMPITraits(p,m) \
  template<> \
  struct MPITraits<p>{ \
    static inline MPI_Datatype getType(){ \
      return m; \
    } \
  }

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

  template<class K, int n> class FieldVector;

  template<class K, int n>
  struct MPITraits<FieldVector<K,n> >
  {
    static MPI_Datatype datatype;
    static MPI_Datatype vectortype;

    static inline MPI_Datatype getType()
    {
      if(datatype==MPI_DATATYPE_NULL) {
        MPI_Type_contiguous(n, MPITraits<K>::getType(), &vectortype);
        MPI_Type_commit(&vectortype);
        FieldVector<K,n> fvector;
        MPI_Aint base;
        MPI_Aint displ;
        MPI_Address(&fvector, &base);
        MPI_Address(&(fvector[0]), &displ);
        displ -= base;
        int length[1]={1};

        MPI_Type_struct(1, length, &displ, &vectortype, &datatype);
        MPI_Type_commit(&datatype);
      }
      return datatype;
    }

  };

  template<class K, int n>
  MPI_Datatype MPITraits<FieldVector<K,n> >::datatype = MPI_DATATYPE_NULL;
  template<class K, int n>
  MPI_Datatype MPITraits<FieldVector<K,n> >::vectortype = {MPI_DATATYPE_NULL};


  template<int k>
  class bigunsignedint;

  template<int k>
  struct MPITraits<bigunsignedint<k> >
  {
    static MPI_Datatype datatype;
    static MPI_Datatype vectortype;

    static inline MPI_Datatype getType()
    {
      if(datatype==MPI_DATATYPE_NULL) {
        MPI_Type_contiguous(bigunsignedint<k>::n, MPITraits<std::uint16_t>::getType(),
                            &vectortype);
        //MPI_Type_commit(&vectortype);
        bigunsignedint<k> data;
        MPI_Aint base;
        MPI_Aint displ;
        MPI_Address(&data, &base);
        MPI_Address(&(data.digit), &displ);
        displ -= base;
        int length[1]={1};
        MPI_Type_struct(1, length, &displ, &vectortype, &datatype);
        MPI_Type_commit(&datatype);
      }
      return datatype;
    }
  };
}

namespace Dune
{
  template<int k>
  MPI_Datatype MPITraits<bigunsignedint<k> >::datatype = MPI_DATATYPE_NULL;
  template<int k>
  MPI_Datatype MPITraits<bigunsignedint<k> >::vectortype = MPI_DATATYPE_NULL;

  template<typename T1, typename T2>
  struct MPITraits<std::pair<T1,T2 > >
  {
  public:
    inline static MPI_Datatype getType();
  private:
    static MPI_Datatype type;
  };
  template<typename T1, typename T2>
  MPI_Datatype MPITraits<std::pair<T1,T2> >::getType()
  {
    if(type==MPI_DATATYPE_NULL) {
      int length[4];
      MPI_Aint disp[4];
      MPI_Datatype types[4] = {MPI_LB, MPITraits<T1>::getType(),
                               MPITraits<T2>::getType(), MPI_UB};
      std::pair<T1,T2> rep[2];
      length[0]=length[1]=length[2]=length[3]=1;
      MPI_Address(rep, disp); // lower bound of the datatype
      MPI_Address(&(rep[0].first), disp+1);
      MPI_Address(&(rep[0].second), disp+2);
      MPI_Address(rep+1, disp+3); // upper bound of the datatype
      for(int i=3; i >= 0; --i)
        disp[i] -= disp[0];
      MPI_Type_struct(4, length, disp, types, &type);
      MPI_Type_commit(&type);
    }
    return type;
  }

  template<typename T1, typename T2>
  MPI_Datatype MPITraits<std::pair<T1,T2> >::type=MPI_DATATYPE_NULL;
#endif
#endif
#endif
  /** @} */
}

#endif
