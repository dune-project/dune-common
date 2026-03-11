// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_MATRIXCONCEPTS_HH
#define DUNE_COMMON_MATRIXCONCEPTS_HH



#include <utility>
#include <type_traits>

#include <dune/common/std/type_traits.hh>



namespace Dune {

  template<class, int, int>
  class FieldMatrix;

  template<class>
  class DynamicMatrix;

}

namespace Dune::Impl {


  // Some traits for checking matrix concepts. Currently these are
  // all technical internal helpers that just serve different headers
  // to do internal checks and are thus collected here.

  template<class T>
  using IsMatrixHelper = std::void_t<decltype(std::declval<T>().N(), std::declval<T>().M())>;

  template<class T>
  struct IsMatrix : public Dune::Std::is_detected<IsMatrixHelper, T> {};

  // Check if T is a matrix by checking for member functions N() and M().
  template<class T>
  constexpr bool IsMatrix_v = Impl::IsMatrix<T>::value;



  template<class T>
  using IsStaticSizeMatrixHelper = std::void_t<decltype(T::rows, T::cols)>;

  template<class T>
  struct IsStaticSizeMatrix : public Dune::Std::is_detected<IsStaticSizeMatrixHelper, T> {};

  // Check if T is a statically sized matrix by checking for static members rows and cols.
  template<class T>
  constexpr bool IsStaticSizeMatrix_v = Impl::IsStaticSizeMatrix<T>::value;



  template<class T>
  class IsFieldMatrix : public std::false_type {};

  template< class K, int ROWS, int COLS>
  class IsFieldMatrix<Dune::FieldMatrix<K, ROWS, COLS>> : public std::true_type {};

  // Check if T is an instance of FieldMatrix
  template<class T>
  constexpr bool IsFieldMatrix_v = Impl::IsFieldMatrix<T>::value;



  /**
   * \brief The traits `IsDenseMatrix` is satisfied for all matrices with dense
   * element access.
   *
   * A matrix `m` has dense element access by a chained `m[i][j]` if this is
   * valid for all indices in its index space `[0,m.N()) x [0,m.M())`. User-defined
   * types can be registered to this traits by specializing `IsDenseMatrix`.
   *
   * \b Examples:
   * - `FieldMatrix<K, n, m>`
   * - `DynamicMatrix<K>`
   */
  template<class T>
  class IsDenseMatrix : public std::false_type {};

  template<class T>
  class IsDenseMatrix<const T> : public IsDenseMatrix<T> {};

  template<class K, int ROWS, int COLS>
  class IsDenseMatrix<Dune::FieldMatrix<K, ROWS, COLS>> : public std::true_type {};

  template<class K>
  class IsDenseMatrix<Dune::DynamicMatrix<K>> : public std::true_type {};

  // Check if T is a dense matrix. This is implemented by specialization.
  template<class T>
  constexpr bool IsDenseMatrix_v = Impl::IsDenseMatrix<T>::value;



} // namespace Dune::Impl



#endif // DUNE_COMMON_MATRIXCONCEPTS_HH
