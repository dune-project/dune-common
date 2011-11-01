// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <dune/common/fvector.hh>
#include <dune/common/fassign.hh>

using Dune::zero;
using Dune::nextRow;

template<class T> struct Print {};

template<int s>
struct Print< Dune::FieldVector<int,s> >
{
  static void print(Dune::FieldVector<int,s> & v)
  {
    for (int i=0; i<s; i++)
      std::cout << "value[" << i << "] = " << v[i] << "\n";
  }
};

template<int n, int m>
struct Print< Dune::FieldMatrix<int,n,m> >
{
  static void print(Dune::FieldMatrix<int,n,m> & A)
  {
    for (int i=0; i<n; i++)
      for (int j=0; j<m; j++)
        std::cout << "value[" << i << "][" << j << "] = " << A[i][j] << "\n";
  }
};

template<class T>
void print(T & t) {
  Print<T>::print(t);
}

int main ()
{
  try
  {
#ifdef _DUNE_TEST_M
    Dune::FieldMatrix<int,_DUNE_TEST_N,_DUNE_TEST_M> x;
#else
    Dune::FieldVector<int,_DUNE_TEST_N> x;
#endif

    x <<= _VALUES;
    print(x);

    return 0;
  }
  catch (Dune::MathError &e)
  {
    std::cerr << "Dune reported MathError: " << e << std::endl;
    return 1;
  }
  catch (Dune::Exception &e)
  {
    std::cerr << "Dune reported error: " << e << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown exception thrown!" << std::endl;
    return 1;
  }
}
