// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DOTPRODUCT_HH
#define DUNE_DOTPRODUCT_HH

#include "ftraits.hh"
#include "typetraits.hh"
#include "promotiontraits.hh"

namespace Dune {
  /**
   * @file
   * @brief  Provides the functions dot(a,b) := \f$a^H \cdot b \f$ and dotT(a,b) := \f$a^T \cdot b \f$
   *
   * The provided dot products dot,dotT are used to compute (indefinite) dot products for fundamental types as well as DUNE vector types, such as DenseVector, FieldVector, ISTLVector.
   * Note that the definition of dot(a,b) conjugates the first argument. This agrees with the behvaior of Matlab and Petsc, but noch with BLAS.
   * @author Jö Fahlke, Matthias Wohlmuth
   */

  /** @addtogroup Common
   *
   * @{
   */

  template<class T>
  struct AlwaysVoid { typedef void type; };

  template<class T, class = void>
  struct IsVector : false_type {};

  template<class T>
  struct IsVector<T, typename AlwaysVoid<typename T::field_type>::type>
    : true_type {};

  /** @brief computes the dot product for fundamental data types according to Petsc's VectDot function: dot(a,b) := std::conj(a)*b
   *
   * @see http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Vec/VecDot.html#VecDot
   * @param a
   * @param b
   * @return conj(a)*b
   */
  template<class A, class B>
  inline typename enable_if<!IsVector<A>::value && !is_same<typename FieldTraits<A>::field_type,typename FieldTraits<A>::real_type> ::value,  typename PromotionTraits<A,B>::PromotedType>::type
  dot(const A & a, const B & b) {
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
  inline typename enable_if<!IsVector<A>::value && is_same<typename FieldTraits<A>::field_type,typename FieldTraits<A>::real_type>::value,  typename PromotionTraits<A,B>::PromotedType>::type
  dot(const A & a, const B & b) {
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
  // vectors
  template<typename A, typename B>
  //  inline typename enable_if<IsVector<A>::value, typename PromotionTraits<typename FieldTraits<A>::field_type, typename FieldTraits<B>::field_type >::PromotedType>::type
  inline typename enable_if<IsVector<A>::value, typename PromotionTraits<typename A::field_type, typename B::field_type >::PromotedType>::type
  dot(const A & a, const B & b) {
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
  inline typename enable_if<!IsVector<A>::value && !is_same<typename FieldTraits<A>::field_type,typename FieldTraits<A>::real_type> ::value,  typename PromotionTraits<A,B>::PromotedType>::type
  dotT(const A & a, const B & b) {
    return a*b;
  }

  /**
   * @brief Computes an indefinite vector dot product for various dune vector types according to Petsc's VectTDot function: dotT(a,b) := a*b
   * @see http://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Vec/VecTDot.html#VecTDot
   * @param a
   * @param b
   * @return a*b
   */
  template<class A, class B>
  inline typename enable_if<IsVector<A>::value, typename PromotionTraits<typename A::field_type, typename B::field_type >::PromotedType>::type
  dotT(const A & a, const B & b) {
    return a*b;
  }

  /** @} */
} // end namespace DUNE

#endif // DUNE_DOTPRODUCT_HH
