// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_FMatrix_WITH_CHECKING
#define DUNE_FMatrix_WITH_CHECKING
#endif

#include <iostream>
#include <algorithm>
#include <type_traits>

#include <dune/common/dynmatrix.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/transpose.hh>

#include "checkmatrixinterface.hh"

using namespace Dune;

template<class K, int n>
void test_matrix()
{
  [[maybe_unused]] typedef typename DiagonalMatrix<K,n>::size_type size_type;

  DiagonalMatrix<K,n> A(1);
  FieldVector<K,n> f;
  FieldVector<K,n> v;

  // test constexpr size
  static_assert(A.N() == n, "");
  static_assert(A.M() == n, "");

  // assign matrix
  A=2;

  // assign vector
  f = 1;
  v = 2;

  // matrix vector product
  A.umv(v,f);


  // test norms
  A.frobenius_norm();
  A.frobenius_norm2();
  A.infinity_norm();
  A.infinity_norm_real();

  std::sort(v.begin(), v.end());

  // print matrix
  std::cout << A << std::endl;
  // print vector
  std::cout << f << std::endl;

  // assign to FieldMatrix
  [[maybe_unused]] FieldMatrix<K,n,n> AFM = FieldMatrix<K,n,n>(A);
  [[maybe_unused]] FieldMatrix<K,n,n> AFM2 = A;
  [[maybe_unused]] FieldMatrix<K,n,n> AFM3;
  AFM3 = A;

  // test transposed
  DiagonalMatrix<K,n> AT = A.transposed();
  if (AT != A)
    DUNE_THROW(FMatrixError, "Return value of DiagoalMatrix::transposed() incorrect!");

  // check matrix-matrix multiplication
  [[maybe_unused]] auto AA = A * A;
  [[maybe_unused]] auto AF = A * AFM;
  [[maybe_unused]] auto FA = AFM * A;
  [[maybe_unused]] auto AFt = A * transposedView(AFM);
  [[maybe_unused]] auto FtA = transposedView(AFM) * A;

  Dune::DynamicMatrix<K> ADM(n,n);
  [[maybe_unused]] auto AD = A * ADM;
  // [[maybe_unused]] auto DA = ADM * A;
  [[maybe_unused]] auto ADt = A * transposedView(ADM);
  // [[maybe_unused]] auto DtA = transposedView(ADM) * A;


  // check mixed copy/assignment
  {
    using K2 = std::complex<K>;
    using K3 = std::conditional_t<std::is_same_v<K,float>, double, float>;

    if constexpr (std::is_convertible_v<K,K2>) {
      A = K(1);
      DiagonalMatrix<K2,n> A2(A);
      if (A2.infinity_norm() != K2(1))
        DUNE_THROW(FMatrixError,"Mixed Copy-construction test failed!");

      A = K(2);
      A2 = A;
      if (A2.infinity_norm() != K2(2))
        DUNE_THROW(FMatrixError,"Mixed Copy-assignment test failed!");
    }

    if constexpr (std::is_convertible_v<K,K3>) {
      A = K(1);
      DiagonalMatrix<K3,n> A3(A);
      if (A3.infinity_norm() != K3(1))
        DUNE_THROW(FMatrixError,"Mixed Copy-construction test failed!");

      A = K(2);
      A3 = A;
      if (A3.infinity_norm() != K3(2))
        DUNE_THROW(FMatrixError,"Mixed Copy-assignment test failed!");
    }
  }
}

template<class K, int n>
void test_interface()
{
  typedef CheckMatrixInterface::UseFieldVector<K,n,n> Traits;
  typedef Dune::DiagonalMatrix<K,n> DiagonalMatrix;

  const DiagonalMatrix A(1);
  checkMatrixInterface< DiagonalMatrix >( A );
  checkMatrixInterface< DiagonalMatrix, Traits >( A );
}

void test_initialisation()
{
  [[maybe_unused]] Dune::DiagonalMatrix<int, 2> const b = { 1, 2 };

  assert(b.diagonal(0) == 1);
  assert(b.diagonal(1) == 2);
}

int main()
{
  try {
    test_matrix<float, 1>();
    test_interface<float, 1>();
    test_matrix<double, 1>();
    test_interface<double, 1>();
    test_matrix<double, 5>();
    test_interface<double, 5>();
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
  }
}
