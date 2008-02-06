// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#define DUNE_ISTL_WITH_CHECKING
#include "../fmatrix.hh"
#include <iostream>
#include <algorithm>

using namespace Dune;

template<typename T, std::size_t n>
int test_invert_solve(T A_data[n*n], T inv_data[n*n],
                      T x_data[n], T b_data[n])
{
  int ret=0;

  std::cout <<"Checking inversion of:"<<std::endl;

  FieldMatrix<T,n,n> A, inv, calced_inv;
  FieldVector<T,n> x, b, calced_x;

  for(int i =0; i < n; ++i) {
    x[i]=x_data[i];
    b[i]=b_data[i];
    for(int j=0; j <n; ++j) {
      A[i][j] = A_data[i*n+j];
      inv[i][j] = inv_data[i*n+j];
    }
  }

  std::cout<<A<<std::endl;

  // Check whether given inverse is correct
  FieldMatrix<T,n,n> prod = A;
  prod.rightmultiply(inv);
  for (int i=0; i<n; i++)
    prod[i][i] -= 1;

  bool equal=true;
  if (prod.infinity_norm() > 1e-6) {
    std::cerr<<"Given inverse wrong"<<std::endl;
    equal=false;
  }

  FieldMatrix<T,n,n> copy(A);
  A.invert();

  calced_inv = A;
  A-=inv;


  double singthres = FMatrixPrecision<>::singular_limit()*10;
  for(int i =0; i < n; ++i)
    for(int j=0; j <n; ++j)
      if(std::abs(A[i][j])>singthres) {
        std::cerr<<"calculated inverse wrong at ("<<i<<","<<j<<")"<<std::endl;
        equal=false;
      }

  if(!equal) {
    ret++;
    std::cerr<<"Calculated inverse was:"<<std::endl;
    std::cerr <<calced_inv<<std::endl;
    std::cerr<<"Should have been"<<std::endl;
    std::cerr<<inv << std::endl;
  }else
    std::cout<<"Result is"<<std::endl<<calced_inv<<std::endl;


  std::cout<<"Checking solution for rhs="<<b<<std::endl;

  // Check whether given solution is correct
  FieldVector<T,n> trhs=b;

  copy.mmv(x,trhs);
  equal=true;

  if (trhs.infinity_norm() > 1e-6) {
    std::cerr<<"Given rhs does not fit solution"<<std::endl;
    equal=false;
  }
  copy.solve(calced_x, b);
  FieldVector<T,n> xcopy(calced_x);
  xcopy-=x;

  equal=true;

  for(int i =0; i < n; ++i)
    if(std::abs(xcopy[i])>singthres) {
      std::cerr<<"calculated isolution wrong at ("<<i<<")"<<std::endl;
      equal=false;
    }

  if(!equal) {
    ret++;
    std::cerr<<"Calculated solution was:"<<std::endl;
    std::cerr <<calced_x<<std::endl;
    std::cerr<<"Should have been"<<std::endl;
    std::cerr<<x<<std::endl;
    std::cerr<<"difference is "<<xcopy<<std::endl;
  }else
    std::cout<<"Result is "<<calced_x<<std::endl;

  return ret;
}


int test_invert_solve()
{
  int ret=0;

  double A_data[9] = {1, 5, 7, 2, 14, 15, 4, 40, 39};
  double inv_data[9] = {-9.0/4, 85.0/24, -23.0/24, -3.0/4, 11.0/24, -1.0/24, 1, -5.0/6, 1.0/6};
  double b[3] = {32,75,201};
  double x[3] = {1,2,3};

  ret += test_invert_solve<double,3>(A_data, inv_data, x, b);

  double A_data0[9] = {-0.5, 0, -0.25, 0.5, 0, -0.25, 0, 0.5, 0};
  double inv_data0[9] = {-1, 1, 0, 0, 0, 2, -2, -2, 0};
  double b0[3] = {32,75,201};
  double x0[3] = {43, 402, -214};

  ret += test_invert_solve<double,3>(A_data0, inv_data0, x0, b0);

  double A_data1[9] = {0, 1, 0, 1, 0, 0, 0, 0, 1};
  double b1[3] = {0,1,2};
  double x1[3] = {1,0,2};

  ret += test_invert_solve<double,3>(A_data1, A_data1, x1, b1);

  double A_data2[9] ={3, 1, 6, 2, 1, 3, 1, 1, 1};
  double inv_data2[9] ={-2, 5, -3, 1, -3, 3, 1, -2, 1};
  double b2[3] = {2, 7, 4};
  double x2[3] = {19,-7,-8};

  return ret + test_invert_solve<double,3>(A_data2, inv_data2, x2, b2);
}

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
  // check that mv and umv are doing the same thing
  {
    FieldVector<K,n> res2(0);
    FieldVector<K,n> res1;

    FieldVector<K,m> b(1);

    A.mv(b, res1);
    A.umv(b, res2);

    if( (res1 - res2).two_norm() > 1e-12 )
    {
      DUNE_THROW(FMatrixError,"mv and umv are not doing the same!");
    }
  }

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
    return test_invert_solve();
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
  }
}
