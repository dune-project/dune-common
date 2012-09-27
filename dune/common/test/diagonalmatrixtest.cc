// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"

#include <dune/common/diagonalmatrix.hh>

#include <iostream>
#include <algorithm>

#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>

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
  FieldMatrix<K,n,n> AFM = FieldMatrix<K,n,n>(A);
}

int main()
{
  try {
    test_matrix<float, 1>();
    test_matrix<double, 1>();
    test_matrix<double, 5>();
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
  }
}
