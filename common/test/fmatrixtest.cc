// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#define DUNE_ISTL_WITH_CHECKING
#include "../fmatrix.hh"
#include <iostream>
#include <algorithm>

using namespace Dune;

template<class K, int n, int m>
void test_matrix()
{
  typedef typename FieldMatrix<K,n,m>::size_type size_type;

  FieldMatrix<K,n,m> A;
  FieldVector<K,n> f;
  FieldVector<K,m> v;

  // assign matrix
  A=0;
  // random access matrix
  for (size_type i=0; i<A.rowdim(); i++)
    for (size_type j=0; j<A.coldim(); j++)
      A[i][j] = i*j;
  // iterator matrix
  typename FieldMatrix<K,n,m>::RowIterator rit = A.begin();
  for (; rit!=A.end(); ++rit)
  {
    rit.index();
    typename FieldMatrix<K,n,m>::ColIterator cit = rit->begin();
    for (; cit!=rit->end(); ++cit)
    {
      cit.index();
      (*cit) *= 2;
    }
  }

  // assign vector
  f = 1;

  // random access vector
  for (size_type i=0; i<v.dim(); i++)
    v[i] = i;
  // iterator vector
  typename FieldVector<K,m>::iterator it = v.begin();
  typename FieldVector<K,m>::ConstIterator end = v.end();
  for (; it!=end; ++it)
  {
    it.index();
    (*it) *= 2;
  }
  // reverse iterator vector
  it = v.rbegin();
  end = v.rend();
  for (; it!=end; --it)
    (*it) /= 2;
  // find vector
  for (size_type i=0; i<v.dim(); i++)
  {
    it = v.find(i);
    (*it) += 1;
  }

  // matrix vector product
  A.umv(v,f);

  A.infinity_norm();

  std::sort(v.begin(), v.end());

  // print matrix
  std::cout << A << std::endl;
  // print vector
  std::cout << f << std::endl;
}

int main()
{
  try {
    test_matrix<float, 1, 1>();
    test_matrix<double, 1, 1>();
    test_matrix<int, 10, 5>();
    test_matrix<double, 5, 10>();
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
  }
}
