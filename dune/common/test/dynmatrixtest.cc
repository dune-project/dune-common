// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// Activate checking
#ifndef DUNE_FMatrix_WITH_CHECKING
#define DUNE_FMatrix_WITH_CHECKING
#endif

#include <dune/common/dynmatrix.hh>
#include <dune/common/dynvector.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/fvector.hh>

#include <iostream>
#include <algorithm>
#include <vector>

#include "checkmatrixinterface.hh"

using namespace Dune;

template<typename T, std::size_t n>
int test_invert_solve(Dune::DynamicMatrix<T> &A,
                      Dune::DynamicMatrix<T> &inv,
                      Dune::FieldVector<T, n> &x,
                      Dune::FieldVector<T, n> &b)
{
  int ret=0;

  std::cout <<"Checking inversion of:"<<std::endl;

  DynamicMatrix<T> calced_inv(n,n);
  FieldVector<T,n> calced_x;

  std::cout<<A<<std::endl;

  // Check whether given inverse is correct
  DynamicMatrix<T> prod = A;
  prod.rightmultiply(inv);
  for (size_t i=0; i<n; i++)
    prod[i][i] -= 1;

  bool equal=true;
  if (prod.infinity_norm() > 1e-6) {
    std::cerr<<"Given inverse wrong"<<std::endl;
    equal=false;
  }

  DynamicMatrix<T> copy(A);
  A.invert();

  calced_inv = A;
  A-=inv;


  auto epsilon = std::numeric_limits<typename FieldTraits<T>::real_type>::epsilon();
  auto tolerance = 10*epsilon;
  for(size_t i =0; i < n; ++i)
    for(size_t j=0; j <n; ++j)
      if(std::abs(A[i][j])>tolerance) {
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

  for(size_t i =0; i < n; ++i)
    if(std::abs(xcopy[i])>tolerance) {
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
  int ret = 0;

  using DM = Dune::DynamicMatrix<double>;
  using FV = Dune::FieldVector<double, 3>;

  DM A = {{1, 5, 7}, {2, 14, 15}, {4, 40, 39}};
  DM inv = {{-9.0 / 4, 85.0 / 24, -23.0 / 24},
            {-3.0 / 4, 11.0 / 24, -1.0 / 24},
            {1, -5.0 / 6, 1.0 / 6}};
  FV b = {32, 75, 201};
  FV x = {1, 2, 3};

  ret += test_invert_solve<double, 3>(A, inv, x, b);

  DM A0 = {{-0.5, 0, -0.25}, {0.5, 0, -0.25}, {0, 0.5, 0}};
  DM inv0 = {{-1, 1, 0}, {0, 0, 2}, {-2, -2, 0}};
  FV b0 = {32, 75, 201};
  FV x0 = {43, 402, -214};

  ret += test_invert_solve<double, 3>(A0, inv0, x0, b0);

  DM A1 = {{0, 1, 0}, {1, 0, 0}, {0, 0, 1}};
  FV b1 = {0, 1, 2};
  FV x1 = {1, 0, 2};

  ret += test_invert_solve<double, 3>(A1, A1, x1, b1);

  DM A2 = {{3, 1, 6}, {2, 1, 3}, {1, 1, 1}};
  DM inv2 = {{-2, 5, -3}, {1, -3, 3}, {1, -2, 1}};
  FV b2 = {2, 7, 4};
  FV x2 = {19, -7, -8};

  return ret + test_invert_solve<double, 3>(A2, inv2, x2, b2);
}

template<class K, class X, class Y>
void test_mult(DynamicMatrix<K>& A,
               X& v, Y& f)
{
  // test the various matrix-vector products
  A.mv(v,f);
  A.mtv(f,v);
  A.umv(v,f);
  A.umtv(f,v);
  A.umhv(f,v);
  A.mmv(v,f);
  A.mmtv(f,v);
  A.mmhv(f,v);
  A.usmv((K)0.5,v,f);
  A.usmtv((K)0.5,f,v);
  A.usmhv((K)0.5,f,v);
}


template<class K, int n, int m>
void test_matrix()
{
  typedef typename DynamicMatrix<K>::size_type size_type;

  DynamicMatrix<K> A(n,m);
  DynamicVector<K> f(n);
  DynamicVector<K> v(m);

  DynamicMatrix<K> A2(A); // copy constructor
  if (A!=A2)
    DUNE_THROW(InvalidStateException,"Copy constructor does not work properly");

  DynamicMatrix<K> A3(std::move(A2)); // move constructor
  if (A!=A3)
    DUNE_THROW(InvalidStateException,"Move constructor does not work properly");

  A2 = std::move(A3); // move assignment
  if (A!=A2)
    DUNE_THROW(InvalidStateException,"Move assignment does not work properly");

  // assign matrix
  A=K();
  // random access matrix
  for (size_type i=0; i<n; i++)
    for (size_type j=0; j<m; j++)
      A[i][j] = i*j;
  // iterator matrix
  typename DynamicMatrix<K>::RowIterator rit = A.begin();
  for (; rit!=A.end(); ++rit)
  {
    rit.index();
    typename DynamicMatrix<K>::ColIterator cit = rit->begin();
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
  typename DynamicVector<K>::iterator it = v.begin();
  typename DynamicVector<K>::ConstIterator end = v.end();
  for (; it!=end; ++it)
  {
    it.index();
    (*it) *= 2;
  }
  // reverse iterator vector
  it = v.beforeEnd();
  end = v.beforeBegin();
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
    DynamicVector<K> res2(n,0);
    DynamicVector<K> res1(n);

    DynamicVector<K> b(m,1);

    A.mv(b, res1);
    A.umv(b, res2);

    if( (res1 - res2).two_norm() > 1e-12 )
    {
      DUNE_THROW(FMatrixError,"mv and umv are not doing the same!");
    }
  }

  {
    FieldVector<K,m> v0;
    for (size_t i=0; i<m; i++) v0[i] = v[i];
    test_mult(A, v0, f );
  }

  {
    DynamicVector<K> v0 ( v );
    test_mult(A, v0, f );
  }

  // {
  //   std::vector<K> v1( m ) ;
  //   std::vector<K> f1( n, 1 ) ;
  //   // random access vector
  //   for (size_type i=0; i<v1.size(); i++) v1[i] = i;
  //   test_mult(A, v1, f1 );
  // }
  // {
  //   K v2[ m ];
  //   K f2[ n ];
  //   // random access vector
  //   for (size_type i=0; i<m; ++i) v2[i] = i;
  //   for (size_type i=0; i<n; ++i) f2[i] = 1;
  //   test_mult(A, v2, f2 );
  // }

  // Test the different matrix norms
  assert( A.frobenius_norm() >= 0 );
  assert( A.frobenius_norm2() >= 0 );
  assert( A.infinity_norm() >= 0 );
  assert( A.infinity_norm_real() >= 0);

  std::sort(v.begin(), v.end());

  // print matrix
  std::cout << A << std::endl;
  // print vector
  std::cout << f << std::endl;

  // transposed(Matrix)
  {
    DynamicMatrix<K> AT = A.transposed();
    for (size_type i=0; i<AT.N(); i++)
      for (size_type j=0; j<AT.M(); j++)
        if (AT[i][j] != A[j][i])
          DUNE_THROW(FMatrixError, "Return value of matrix.transposed() incorrect!");
  }

  {
    DynamicMatrix<K> A2 = A;
    A2 *= 2;

    DynamicMatrix<K> B = A;
    B += A;
    B -= A2;
    if (std::abs(B.infinity_norm()) > 1e-12)
      DUNE_THROW(FMatrixError,"Operator +=/-= test failed!");
  }
  {
    DynamicMatrix<K> A3 = A;
    A3 *= 3;

    DynamicMatrix<K> B = A;
    B.axpy( K( 2 ), B );
    B -= A3;
    if (std::abs(B.infinity_norm()) > 1e-12)
      DUNE_THROW(FMatrixError,"Axpy test failed!");
  }
  {
    DynamicMatrix<K> A2(n,n+1);
    for(size_type i=0; i<A2.N(); ++i)
      for(size_type j=0; j<A2.M(); ++j)
        A2[i][j] = i;
    [[maybe_unused]] const DynamicMatrix<K>& Aref = A2;


    DynamicMatrix<K> B(n+1,n+1);
    for(size_type i=0; i<B.N(); ++i)
      for(size_type j=0; j<B.M(); ++j)
        B[i][j] = i;
    [[maybe_unused]] const DynamicMatrix<K>& Bref = B;

    DynamicMatrix<K> C(n,n);
    for(size_type i=0; i<C.N(); ++i)
      for(size_type j=0; j<C.M(); ++j)
        C[i][j] = i;
    [[maybe_unused]] const DynamicMatrix<K>& Cref = C;

#if 0
    DynamicMatrix<K> AB = Aref.rightmultiplyany(B);
    for(size_type i=0; i<AB.N(); ++i)
      for(size_type j=0; j<AB.M(); ++j)
        if (std::abs<double>(AB[i][j] - i*n*(n+1)/2) > 1e-10)
          DUNE_THROW(FMatrixError,"Rightmultiplyany test failed!");

    DynamicMatrix<K> AB2 = A;
    AB2.rightmultiply(B);
    AB2 -= AB;
    if (std::abs(AB2.infinity_norm() > 1e-10))
      DUNE_THROW(FMatrixError,"Rightmultiply test failed!");

    DynamicMatrix<K> AB3 = Bref.leftmultiplyany(A);
    AB3 -= AB;
    if (std::abs(AB3.infinity_norm() > 1e-10))
      DUNE_THROW(FMatrixError,"Leftmultiplyany test failed!");

    DynamicMatrix<K> CA = Aref.leftmultiplyany(C);
    for(size_type i=0; i<CA.N(); ++i)
      for(size_type j=0; j<CA.M(); ++j)
        if (std::abs<double>(CA[i][j] - i*n*(n-1)/2) > 1e-10)
          DUNE_THROW(FMatrixError,"Leftmultiplyany test failed!");

    DynamicMatrix<K> CA2 = A;
    CA2.leftmultiply(C);
    CA2 -= CA;
    if (std::abs(CA2.infinity_norm() > 1e-10))
      DUNE_THROW(FMatrixError,"Leftmultiply test failed!");

    DynamicMatrix<K> CA3 = Cref.rightmultiplyany(A);
    CA3 -= CA;
    if (std::abs(CA3.infinity_norm() > 1e-10))
      DUNE_THROW(FMatrixError,"Rightmultiplyany test failed!");
#endif
  }
}

int test_determinant()
{
  int ret = 0;

  DynamicMatrix<double> B(4,4);
  B[0][0] =  3.0; B[0][1] =  0.0; B[0][2] =  1.0; B[0][3] =  0.0;
  B[1][0] = -1.0; B[1][1] =  3.0; B[1][2] =  0.0; B[1][3] =  0.0;
  B[2][0] = -3.0; B[2][1] =  0.0; B[2][2] = -1.0; B[2][3] =  2.0;
  B[3][0] =  0.0; B[3][1] = -1.0; B[3][2] =  0.0; B[3][3] =  1.0;
  if (std::abs(B.determinant() + 2.0) > 1e-12)
  {
    std::cerr << "Determinant 1 test failed" << std::endl;
    ++ret;
  }

  B[0][0] =  3.0; B[0][1] =  0.0; B[0][2] =  1.0; B[0][3] =  0.0;
  B[1][0] = -1.0; B[1][1] =  3.0; B[1][2] =  0.0; B[1][3] =  0.0;
  B[2][0] = -3.0; B[2][1] =  0.0; B[2][2] = -1.0; B[2][3] =  2.0;
  B[3][0] = -1.0; B[3][1] =  3.0; B[3][2] =  0.0; B[3][3] =  2.0;
  if (B.determinant() != 0.0)
  {
    std::cerr << "Determinant 2 test failed" << std::endl;
    ++ret;
  }

  return 0;
}

int main()
{
  try {
    Dune::DynamicMatrix<double> A( 5, 5 );
    checkMatrixInterface( A );

    test_matrix<float, 1, 1>();
    test_matrix<double, 1, 1>();
    test_matrix<int, 10, 5>();
    test_matrix<double, 5, 10>();
    test_determinant();
    Dune::DynamicMatrix<double> B(34, 34, 1e-15);
    for (int i=0; i<34; i++) B[i][i] = 1;
    B.invert();
    return test_invert_solve();
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
  }
}
