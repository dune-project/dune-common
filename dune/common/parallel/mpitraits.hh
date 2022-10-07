// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_MPITRAITS_HH
#define DUNE_MPITRAITS_HH

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

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <complex>

#include <mpi.h>

namespace Dune
{
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
    static constexpr bool is_intrinsic = false;
  };
  template<class T>
  MPI_Datatype MPITraits<T>::datatype = MPI_DATATYPE_NULL;

#ifndef DOXYGEN

  // A Macro for defining traits for the primitive data types
#define ComposeMPITraits(p,m)                   \
  template<>                                    \
  struct MPITraits<p>{                          \
    static inline MPI_Datatype getType(){       \
      return m;                                 \
    }                                           \
    static constexpr bool is_intrinsic = true;  \
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
  ComposeMPITraits(std::complex<double>, MPI_CXX_DOUBLE_COMPLEX);
  ComposeMPITraits(std::complex<long double>, MPI_CXX_LONG_DOUBLE_COMPLEX);
  ComposeMPITraits(std::complex<float>, MPI_CXX_FLOAT_COMPLEX);


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
        MPI_Get_address(&fvector, &base);
        MPI_Get_address(&(fvector[0]), &displ);
        displ -= base;
        int length[1]={1};

        MPI_Type_create_struct(1, length, &displ, &vectortype, &datatype);
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
        MPI_Get_address(&data, &base);
        MPI_Get_address(&(data.digit), &displ);
        displ -= base;
        int length[1]={1};
        MPI_Type_create_struct(1, length, &displ, &vectortype, &datatype);
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
      int length[2] = {1, 1};
      MPI_Aint disp[2];
      MPI_Datatype types[2] = {MPITraits<T1>::getType(),
                               MPITraits<T2>::getType()};

      using Pair = std::pair<T1, T2>;
      static_assert(std::is_standard_layout<Pair>::value, "offsetof() is only defined for standard layout types");
      disp[0] = offsetof(Pair, first);
      disp[1] = offsetof(Pair, second);

      MPI_Datatype tmp;
      MPI_Type_create_struct(2, length, disp, types, &tmp);

      MPI_Type_create_resized(tmp, 0, sizeof(Pair), &type);
      MPI_Type_commit(&type);

      MPI_Type_free(&tmp);
    }
    return type;
  }

  template<typename T1, typename T2>
  MPI_Datatype MPITraits<std::pair<T1,T2> >::type=MPI_DATATYPE_NULL;

#endif // !DOXYGEN

} // namespace Dune

#endif // HAVE_MPI

/** @} group ParallelCommunication */

#endif
