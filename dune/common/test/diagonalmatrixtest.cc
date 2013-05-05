// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/diagonalmatrix.hh>

#include <iostream>
#include <algorithm>

#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>

#include "checkmatrixinterface.hh"

using namespace Dune;

template<class K, int n>
void test_matrix()
{
  typedef typename DiagonalMatrix<K,n>::size_type size_type;

  DiagonalMatrix<K,n> A(1);
  FieldVector<K,n> f;
  FieldVector<K,n> v;

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
  DUNE_UNUSED FieldMatrix<K,n,n> AFM = FieldMatrix<K,n,n>(A);
  DUNE_UNUSED FieldMatrix<K,n,n> AFM2 = A;
  DUNE_UNUSED FieldMatrix<K,n,n> AFM3;
  AFM3 = A;
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
