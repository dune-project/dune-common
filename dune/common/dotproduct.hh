// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_DOTPRODUCT_HH
#define DUNE_DOTPRODUCT_HH

#include "ftraits.hh"
#include "typetraits.hh"

namespace Dune {
  /**
   * @file
   * @brief  Provides the functions dot(a,b) := \f$a^H \cdot b \f$ and dotT(a,b) := \f$a^T \cdot b \f$
   *
   * The provided dot products dot,dotT are used to compute (indefinite) dot products for fundamental types as well as DUNE vector types, such as DenseVector, FieldVector, ISTLVector.
   * Note that the definition of dot(a,b) conjugates the first argument. This agrees with the behaviour of Matlab and Petsc, but not with BLAS.
   * @author Jö Fahlke, Matthias Wohlmuth
   */

  /** @addtogroup Common
   *
   * @{
   */

  template<class T, class = void>
  struct IsVector : std::false_type {};

  template<class T>
  struct IsVector<T, std::void_t<typename T::field_type> >
    : std::true_type {};

  /** @brief computes the dot product for fundamental data types according to Petsc's VectDot function: dot(a,b) := std::conj(a)*b
   *
   * @see http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Vec/VecDot.html#VecDot
   * @param a
   * @param b
   * @return conj(a)*b
   */
  template<class A, class B>
  auto
  dot(const A & a, const B & b) -> typename std::enable_if<IsNumber<A>::value && !IsVector<A>::value && !std::is_same<typename FieldTraits<A>::field_type,typename FieldTraits<A>::real_type> ::value, decltype(conj(a)*b)>::type
  {
    return conj(a)*b;
  }

  /**
   * @brief computes the dot product for fundamental data types according to Petsc's VectDot function: dot(a,b) := std::conj(a)*b
   *
   * Specialization for real first arguments which replaces conj(a) by a.
   * @see http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Vec/VecTDot.html#VecTDot
   * @param a
   * @param b
   * @return a*b (which is the same as conj(a)*b in this case)
   */
  // fundamental type with A being a real type
  template<class A, class B>
  auto
  dot(const A & a, const B & b) -> typename std::enable_if<IsNumber<A>::value && !IsVector<A>::value && std::is_same<typename FieldTraits<A>::field_type,typename FieldTraits<A>::real_type>::value, decltype(a*b)>::type
  {
    return a*b;
  }

  /**
   * @brief computes the dot product for various dune vector types according to Petsc's VectDot function: dot(a,b) := std::conj(a)*b
   *
   * Specialization for real first arguments which replaces conj(a) by a.
   * @see http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Vec/VecTDot.html#VecTDot
   * @param a
   * @param b
   * @return dot(a,b)
   */
  template<typename A, typename B>
  auto
  dot(const A & a, const B & b) -> typename std::enable_if<IsVector<A>::value, decltype(a.dot(b))>::type
  {
    return a.dot(b);
  }
  /** @} */

  /**
   * @brief Computes an indefinite vector dot product for fundamental data types according to Petsc's VectTDot function: dotT(a,b) := a*b
   * @see http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Vec/VecTDot.html#VecTDot
   * @param a
   * @param b
   * @return a*b
   */
  template<class A, class B>
  auto
  dotT(const A & a, const B & b) -> decltype(a*b)
  {
    return a*b;
  }

  /** @} */
} // end namespace DUNE

#endif // DUNE_DOTPRODUCT_HH
