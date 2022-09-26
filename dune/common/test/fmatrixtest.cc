// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Activate checking.
#ifndef DUNE_FMatrix_WITH_CHECKING
#define DUNE_FMatrix_WITH_CHECKING
#endif

#include <algorithm>
#include <cassert>
#include <complex>
#include <iostream>
#include <vector>

#include <dune/common/classname.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/fmatrixev.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/simd/loop.hh>
#include <dune/common/simd/simd.hh>
#if HAVE_VC
#include <dune/common/simd/vc.hh>
#endif

#include "checkmatrixinterface.hh"

using namespace Dune;

template<typename T, std::size_t n>
int test_invert_solve(Dune::FieldMatrix<T, n, n> &A,
                      Dune::FieldMatrix<T, n, n> &inv,
                      Dune::FieldVector<T, n> &x,
                      Dune::FieldVector<T, n> &b,
                      bool doPivoting = true)
{
  using std::abs;

  int ret=0;

  std::cout <<"Checking inversion of:"<<std::endl;

  FieldMatrix<T,n,n> calced_inv;
  FieldVector<T,n> calced_x;

  std::cout<<A<<std::endl;

  // Check whether given inverse is correct
  FieldMatrix<T,n,n> prod = A;
  prod.rightmultiply(inv);
  for (size_t i=0; i<n; i++)
    prod[i][i] -= 1;

  bool equal=true;
  if (Simd::anyTrue(prod.infinity_norm() > 1e-6)) {
    std::cerr<<"Given inverse wrong"<<std::endl;
    equal=false;
  }

  FieldMatrix<T,n,n> copy(A);
  A.invert(doPivoting);

  calced_inv = A;
  A-=inv;


  auto epsilon = std::numeric_limits<typename FieldTraits<T>::real_type>::epsilon();
  auto tolerance = 10*epsilon;
  for(size_t i =0; i < n; ++i)
    for(size_t j=0; j <n; ++j)
      if(Simd::anyTrue(abs(A[i][j])>tolerance)) {
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

  if (Simd::anyTrue(trhs.infinity_norm() > 1e-6)) {
    std::cerr<<"Given rhs does not fit solution"<<std::endl;
    equal=false;
  }
  copy.solve(calced_x, b, doPivoting);
  FieldVector<T,n> xcopy(calced_x);
  xcopy-=x;

  equal=true;

  for(size_t i =0; i < n; ++i)
    if(Simd::anyTrue(abs(xcopy[i])>tolerance)) {
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

  using FM = Dune::FieldMatrix<double, 3, 3>;
  using FV = Dune::FieldVector<double, 3>;

  FM A_data = {{1, 5, 7}, {2, 14, 15}, {4, 40, 39}};
  FM inv_data = {{-9.0 / 4, 85.0 / 24, -23.0 / 24},
                 {-3.0 / 4, 11.0 / 24, -1.0 / 24},
                 {1, -5.0 / 6, 1.0 / 6}};
  FV b = {32, 75, 201};
  FV x = {1, 2, 3};
  ret += test_invert_solve<double, 3>(A_data, inv_data, x, b);

  FM A_data0 = {{-0.5, 0, -0.25}, {0.5, 0, -0.25}, {0, 0.5, 0}};
  FM inv_data0 = {{-1, 1, 0}, {0, 0, 2}, {-2, -2, 0}};
  FV b0 = {32, 75, 201};
  FV x0 = {43, 402, -214};
  ret += test_invert_solve<double, 3>(A_data0, inv_data0, x0, b0);

  FM A_data1 = {{0, 1, 0}, {1, 0, 0}, {0, 0, 1}};
  FV b1 = {0, 1, 2};
  FV x1 = {1, 0, 2};
  ret += test_invert_solve<double, 3>(A_data1, A_data1, x1, b1);

  FM A_data2 = {{3, 1, 6}, {2, 1, 3}, {1, 1, 1}};
  FM inv_data2 = {{-2, 5, -3}, {1, -3, 3}, {1, -2, 1}};
  FV b2 = {2, 7, 4};
  FV x2 = {19, -7, -8};
  ret += test_invert_solve<double, 3>(A_data2, inv_data2, x2, b2);

  using FM6 = Dune::FieldMatrix<double, 6, 6>;
  using FV6 = Dune::FieldVector<double, 6>;
  using FM6f = Dune::FieldMatrix<float, 6, 6>;
  using FV6f = Dune::FieldVector<float, 6>;
  using FM6c = Dune::FieldMatrix<std::complex<double>, 6, 6>;
  using FV6c = Dune::FieldVector<std::complex<double>, 6>;
  using FM6cf = Dune::FieldMatrix<std::complex<float>, 6, 6>;
  using FV6cf = Dune::FieldVector<std::complex<float>, 6>;
  FM6 A_data3 = {{0.1756212892262638, 0.18004482126181995, -0.49348712464381461, 0.49938830949606494, -0.7073160963417815, 1.0595994834402057e-06},
                {0.17562806606385517, 0.18005184462676252, -0.49354113600539418, 0.50059575375120657, 0.70689735319270453, -3.769499436967368e-07},
                {0.17562307226079987, 0.1800466692525447, -0.49350050991711036, -0.5000065175076156, 0.00018887507812282846, -0.70710715811504954},
                {0.17562308446070105, 0.18004668189625178, -0.49350060714612815, -0.50000869003275417, 0.00019031361405394119, 0.70710640425695015},
                {-0.0072214111281474463, 0.93288324029450198, -0.11009998093332186, -1.7482015044681947e-06, -2.35420746900079e-06, -4.2380607559371285e-09},
                {0.93625470097440933, -0.0077746247590777659, -0.11696151733678119, -1.8717676241478393e-06, -2.5225363177584535e-06, -4.5410877139483271e-09}};
  FM6 inv_data3 =  {{-0.069956619842954, -0.069956322880040, -0.069956501823745, -0.069956501289142, 0.063349638850509, 1.121064161778902},
                   {-0.066113473123754, -0.066113223084417, -0.066113362249636, -0.066113361799508, 1.123470950632021, 0.058271943290769},
                   {-0.555587502096003, -0.555615651279932, -0.555585807267011, -0.555585857939820, 0.432422844944552, 0.420211281044740},
                   { 0.499710573383257, 0.500274796075355, -0.500006831431901, -0.500007846623773, 0.000003909674199, 0.000003817686226},
                   {-0.707554041861306, 0.706659150542343, 0.000405628342406, 0.000407065756770, 0.000010628642550, 0.000010383891450},
                   { 0.000001450379141, 0.000000012708409, -0.707107586716496, 0.707105975654669, 0.000000019133995, 0.000000018693387}};
  FV6 b3 = {1, 1, 1, 1, 1, 1};
  FV6 x3 = {0.904587854793530, 0.917289473665475, -1.369740692593475, -0.000021581236636, -0.000061184685788, -0.000000110146895};
  FM6f A_data3f, inv_data3f;
  FM6c A_data3c, inv_data3c;
  FM6cf A_data3cf, inv_data3cf;
  std::copy(A_data3.begin(), A_data3.end(), A_data3f.begin());
  std::copy(inv_data3.begin(), inv_data3.end(), inv_data3f.begin());
  std::copy(A_data3.begin(), A_data3.end(), A_data3c.begin());
  std::copy(inv_data3.begin(), inv_data3.end(), inv_data3c.begin());
  std::copy(A_data3.begin(), A_data3.end(), A_data3cf.begin());
  std::copy(inv_data3.begin(), inv_data3.end(), inv_data3cf.begin());
  FV6f b3f = b3;
  FV6f x3f = x3;
  FV6c b3c = b3;
  FV6c x3c = x3;
  FV6cf b3cf = b3;
  FV6cf x3cf = x3;
#if HAVE_VC
  using FM6vc = Dune::FieldMatrix< Vc::SimdArray<double, 8>, 6, 6>;
  using FV6vc = Dune::FieldVector< Vc::SimdArray<double, 8>, 6>;
  FM6vc A_data3vc, inv_data3vc;
  std::copy(A_data3.begin(), A_data3.end(), A_data3vc.begin());
  std::copy(inv_data3.begin(), inv_data3.end(), inv_data3vc.begin());
  FV6vc b3vc = b3;
  FV6vc x3vc = x3;
  ret += test_invert_solve< Vc::SimdArray<double, 8>, 6>(A_data3vc, inv_data3vc, x3vc, b3vc);
#endif
  ret += test_invert_solve<double, 6>(A_data3, inv_data3, x3, b3);
  ret += test_invert_solve<std::complex<double>, 6>(A_data3c, inv_data3c, x3c, b3c);
  ret += test_invert_solve<std::complex<float>, 6>(A_data3cf, inv_data3cf, x3cf, b3cf);
  ret += test_invert_solve<float, 6>(A_data3f, inv_data3f, x3f, b3f);

  FM A_data4 = {{2, -1, 0}, {-1, 2, -1}, {0, -1, 2}};
  FM inv_data4 = {{0.75, 0.5, 0.25}, {0.5, 1, 0.5}, {0.25, 0.5, 0.75}};
  FV b4 = {1, 2, 3};
  FV x4 = {2.5, 4, 3.5};
  ret += test_invert_solve<double, 3>(A_data4, inv_data4, x4, b4, false);
  return ret;
}

template<class K, int n, int m, class X, class Y, class XT, class YT>
void test_mult(FieldMatrix<K, n, m>& A,
               X& v, Y& f, XT& vT, YT& fT)
{
  // test the various matrix-vector products
  A.mv(v,f);
  A.mtv(fT,vT);
  A.umv(v,f);
  A.umtv(fT,vT);
  A.umhv(fT,vT);
  A.mmv(v,f);
  A.mmtv(fT,vT);
  A.mmhv(fT,vT);
  using S = typename FieldTraits<Y>::field_type;
  using S2 = typename FieldTraits<XT>::field_type;
  S scalar = (S)(0.5);
  S2 scalar2 = (S2)(0.5);
  A.usmv(scalar,v,f);
  A.usmtv(scalar2,fT,vT);
  A.usmhv(scalar2,fT,vT);
}

template<class K, class K2, class K3, int n, int m>
void test_matrix()
{
  typedef typename FieldMatrix<K,n,m>::size_type size_type;

  FieldMatrix<K,n,m> A;
  FieldVector<K2,m> v;
  FieldVector<K3,n> f;

  // test constexpr size
  static_assert(A.N() == n, "");
  static_assert(A.M() == m, "");

  // assign matrix
  A=K();
  // random access matrix
  for (size_type i=0; i<n; i++)
    for (size_type j=0; j<m; j++)
      A[i][j] = i*j;
  // iterator matrix
  auto rit = A.begin();
  for (; rit!=A.end(); ++rit)
  {
    rit.index();
    auto cit = rit->begin();
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
  auto it = v.begin();
  auto end = v.end();
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
    FieldVector<K3,n> res2(0);
    FieldVector<K3,n> res1;

    FieldVector<K2,m> b(1);

    A.mv(b, res1);
    A.umv(b, res2);

    if( (res1 - res2).two_norm() > 1e-12 )
    {
      DUNE_THROW(FMatrixError,"mv and umv are not doing the same!");
    }
  }

  {
    FieldVector<K2,m> v0 (v);
    FieldVector<K3,n> f0 (f);
    FieldVector<K3,m> vT (0);
    FieldVector<K2,n> fT (0);
    test_mult(A, v0, f0, vT, fT);
  }

  // {
  //   std::vector<K2> v1( m ) ;
  //   std::vector<K3> f1( n, 1 ) ;
  //   // random access vector
  //   for (size_type i=0; i<v1.size(); i++) v1[i] = i;
  //   test_mult(A, v1, f1 );
  // }
  // {
  //   K2 v2[ m ];
  //   K3 f2[ n ];
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

  // print matrix
  std::cout << A << std::endl;
  // print vector
  std::cout << f << std::endl;

  A[0][0] += 5; // Make matrix non-zero
  {
    // Test that operator= and operator-= work before we can test anything else
    using FM = FieldMatrix<K,n,m>;
    FM A0 = A;
    {
      if (A0.infinity_norm() < 1e-12)
        DUNE_THROW(FMatrixError, "Assignment had no effect!");
    }
    A0 -= A;
    {
      if (A0.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Operator-= had no effect!");
    }
    FM A1 = A;         // A1 == A
    FM A2 = (A1 *= 2); // A1 == A2 == 2*A
    {
      FM tmp = A1; tmp -= A;
      if (tmp.infinity_norm() < 1e-12)
        DUNE_THROW(FMatrixError,"Operator*= had no effect!");
    }
    {
      FM tmp = A2; tmp -= A1;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of Operator*= incorrect!");
    }
    [[maybe_unused]] FM A3 = (A2 *= 3); // A2 == A3 == 6*A
    FM A4 = (A2 /= 2); // A2 == A4 == 3*A;
    FM A5 = A;
    A5 *= 3;           // A5       == 3*A
    {
      FM tmp = A2; tmp -= A5;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Operator/= had no effect!");
    }
    {
      FM tmp = A4; tmp -= A5;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of Operator/= incorrect!");
    }

    FM A6 = A;
    FM A7 = (A6 += A); // A6 == A7 == 2*A
    {
      FM tmp = A1; tmp -= A6;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Operator+= had no effect!");
    }
    {
      FM tmp = A1; tmp -= A7;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of Operator+= incorrect!");
    }

    FM A8 = A2;        // A8 == A2 == 3*A
    FM A9 = (A8 -= A); // A9 == A8 == 2*A;
    {
      FM tmp = A8; tmp -= A1;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Operator-= had no effect!");
    }
    {
      FM tmp = A9; tmp -= A1;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of Operator-= incorrect!");
    }
    FM A10 = A;
    FM A11 = A10.axpy(2, A); // A11 = 3*A;
    {
      FM tmp = A10; tmp -= A2;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "axpy() had no effect!");
    }
    {
      FM tmp = A10; tmp -= A11;
      if (tmp.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of axpy() incorrect!");
    }

    // Scalar * Matrix and Matrix * Scalar
    {
      typename FM::field_type scalar = 3;
      FM sA = scalar * A;
      FM aS = A * scalar;
      FM ref = A;
      ref *= scalar;

      if ((sA-ref).infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of operator*(scalar,matrix) incorrect!");

      if ((aS-ref).infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of operator*(matrix,scalar) incorrect!");
    }

    // Matrix / Scalar
    {
      typename FM::field_type scalar = 3;
      FM aS = A / scalar;
      FM ref = A;
      ref /= scalar;

      if ((aS-ref).infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of operator/(matrix,scalar) incorrect!");
    }

    // Matrix + Matrix
    {
      FM twiceA = A + A;
      FM ref = typename FM::field_type(2)*A;

      if ((twiceA-ref).infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of operator+(matrix,matrix) incorrect!");
    }

    // Matrix - Matrix
    {
      FM zero = A - A;

      if (zero.infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of operator-(matrix,matrix) incorrect!");
    }

    // -Matrix
    {
      FM neg = -A;
      FM ref = typename FM::field_type(-1)*A;

      if ((neg-ref).infinity_norm() > 1e-12)
        DUNE_THROW(FMatrixError, "Return value of operator-(matrix) incorrect!");
    }

    // transposed(Matrix)
    {
      FieldMatrix<typename FM::field_type,FM::cols,FM::rows> AT = A.transposed();
      for (int i=0; i<AT.rows; i++)
        for (int j=0; j<AT.cols; j++)
          if (AT[i][j] != A[j][i])
            DUNE_THROW(FMatrixError, "Return value of matrix.transposed() incorrect!");
    }


    // Matrix * Matrix
    [[maybe_unused]] auto product = A.transposed() * A;

  }
  {
    using std::abs;

    FieldMatrix<K,n,m> A3 = A;
    A3 *= 3;

    FieldMatrix<K,n,m> B = A;
    B.axpy( K( 2 ), B );
    B -= A3;
    if (abs(B.infinity_norm()) > 1e-12)
      DUNE_THROW(FMatrixError,"Axpy test failed!");
  }
  {
    using std::abs;

    FieldMatrix<K,n,n+1> A2;
    for(size_type i=0; i<A2.N(); ++i)
      for(size_type j=0; j<A2.M(); ++j)
        A2[i][j] = i;
    const FieldMatrix<K,n,n+1>& Aref = A2;


    FieldMatrix<K,n+1,n+1> B;
    for(size_type i=0; i<B.N(); ++i)
      for(size_type j=0; j<B.M(); ++j)
        B[i][j] = i;
    const FieldMatrix<K,n+1,n+1>& Bref = B;

    FieldMatrix<K,n,n> C;
    for(size_type i=0; i<C.N(); ++i)
      for(size_type j=0; j<C.M(); ++j)
        C[i][j] = i;
    const FieldMatrix<K,n,n>& Cref = C;

    FieldMatrix<K,n,n+1> AB = Aref.rightmultiplyany(B);
    for(size_type i=0; i<AB.N(); ++i)
      for(size_type j=0; j<AB.M(); ++j)
        if (abs(AB[i][j] - K(i*n*(n+1)/2)) > 1e-10)
          DUNE_THROW(FMatrixError,"Rightmultiplyany test failed!");

    FieldMatrix<K,n,n+1> AB2 = A2;
    AB2.rightmultiply(B);
    AB2 -= AB;
    if (abs(AB2.infinity_norm()) > 1e-10)
      DUNE_THROW(FMatrixError,"Rightmultiply test failed!");

    FieldMatrix<K,n,n+1> AB3 = Bref.leftmultiplyany(A2);
    AB3 -= AB;
    if (abs(AB3.infinity_norm()) > 1e-10)
      DUNE_THROW(FMatrixError,"Leftmultiplyany test failed!");

    FieldMatrix<K,n,n+1> CA = Aref.leftmultiplyany(C);
    for(size_type i=0; i<CA.N(); ++i)
      for(size_type j=0; j<CA.M(); ++j)
        if (abs(CA[i][j] - K(i*n*(n-1)/2)) > 1e-10)
          DUNE_THROW(FMatrixError,"Leftmultiplyany test failed!");

    FieldMatrix<K,n,n+1> CA2 = A2;
    CA2.leftmultiply(C);
    CA2 -= CA;
    if (abs(CA2.infinity_norm()) > 1e-10)
      DUNE_THROW(FMatrixError,"Leftmultiply test failed!");

    FieldMatrix<K,n,n+1> CA3 = Cref.rightmultiplyany(A2);
    CA3 -= CA;
    if (abs(CA3.infinity_norm()) > 1e-10)
      DUNE_THROW(FMatrixError,"Rightmultiplyany test failed!");
  }
}

template<class T>
int test_determinant()
{
  using std::abs;

  int ret = 0;

  FieldMatrix<T, 4, 4> B;
  B[0][0] =  3.0; B[0][1] =  0.0; B[0][2] =  1.0; B[0][3] =  0.0;
  B[1][0] = -1.0; B[1][1] =  3.0; B[1][2] =  0.0; B[1][3] =  0.0;
  B[2][0] = -3.0; B[2][1] =  0.0; B[2][2] = -1.0; B[2][3] =  2.0;
  B[3][0] =  0.0; B[3][1] = -1.0; B[3][2] =  0.0; B[3][3] =  1.0;
  if (Simd::anyTrue(abs(B.determinant() + 2.0) > 1e-12))
  {
    std::cerr << "Determinant 1 test failed (" << Dune::className<T>() << ")"
              << std::endl;
    std::cerr << "Determinant 1 is " << B.determinant(true) << ", expected 2.0"
              << std::endl;
    ++ret;
  }

  B[0][0] =  3.0; B[0][1] =  0.0; B[0][2] =  1.0; B[0][3] =  0.0;
  B[1][0] = -1.0; B[1][1] =  3.0; B[1][2] =  0.0; B[1][3] =  0.0;
  B[2][0] = -3.0; B[2][1] =  0.0; B[2][2] = -1.0; B[2][3] =  2.0;
  B[3][0] = -1.0; B[3][1] =  3.0; B[3][2] =  0.0; B[3][3] =  2.0;
  if (Simd::anyTrue(B.determinant(false) != 0.0))
  {
    std::cerr << "Determinant 2 test failed (" << Dune::className<T>() << ")"
              << std::endl;
    std::cerr << "Determinant 2 is " << B.determinant(false) << ", expected 0.0"
              << std::endl;
    ++ret;
  }

  return ret;
}

template<class ft>
struct ScalarOperatorTest
{
  ScalarOperatorTest()
  {
    ft a = 1;
    ft c = 2;
    FieldMatrix<ft,1,1> v(2);
    FieldMatrix<ft,1,1> w(2);
    [[maybe_unused]] bool b;

    std::cout << __func__ << "\t ( " << className(v) << " )" << std::endl;

    a = a * c;
    a = a + c;
    a = a / c;
    a = a - c;

    v = a;
    v = w = v;
    a = v;

    a = v + a;
    a = v - a;
    a = v * a;
    a = v / a;

    v = v + a;
    v = v - a;
    v = v * a;
    v = v / a;

    a = a + v;
    a = a - v;
    a = a * v;
    a = a / v;

    v = a + v;
    v = a - v;
    v = a * v;
    v = a / v;

    v -= w;
    v -= a;
    v += w;
    v += a;
    v *= a;
    v /= a;

    b = (v == a);
    b = (v != a);
    b = (a == v);
    b = (a != v);

  }
};

template<typename ft>
void test_ev()
{
  // rosser test matrix

  /*
     This matrix was a challenge for many matrix eigenvalue
     algorithms. But the Francis QR algorithm, as perfected by
     Wilkinson and implemented in EISPACK, has no trouble with it. The
     matrix is 8-by-8 with integer elements. It has:

   * A double eigenvalue
   * Three nearly equal eigenvalues
   * Dominant eigenvalues of opposite sign
   * A zero eigenvalue
   * A small, nonzero eigenvalue

   */
  Dune::FieldMatrix<ft,8,8> A = {
    { 611, 196, -192, 407, -8, -52, -49, 29 },
    { 196, 899, 113, -192, -71, -43, -8, -44 },
    { -192, 113, 899, 196, 61, 49, 8, 52 },
    { 407, -192, 196, 611, 8, 44, 59, -23 },
    { -8, -71, 61, 8, 411, -599, 208, 208 },
    { -52, -43, 49, 44, -599, 411, 208, 208 },
    { -49, -8, 8, 59, 208, 208, 99, -911 },
    { 29, -44, 52, -23, 208, 208, -911, 99}
  };

  // compute eigenvalues
  Dune::FieldVector<ft,8> eig;
  Dune::FMatrixHelp::eigenValues(A, eig);

  // test results
  Dune::FieldVector<ft,8> ref;
  /*
     reference solution computed with octave 3.2

     > format long e
     > eig(rosser())

   */
  ref = { -1.02004901843000e+03,
  -4.14362871168386e-14,
  9.80486407214362e-02,
  1.00000000000000e+03,
  1.00000000000000e+03,
  1.01990195135928e+03,
  1.02000000000000e+03,
  1.02004901843000e+03 };

  if( (ref - eig).two_norm() > 1e-10 )
  {
    DUNE_THROW(FMatrixError,"error computing eigenvalues");
  }

  std::cout << "Eigenvalues of Rosser matrix: " << eig << std::endl;
}

template< class K, int n >
void test_invert ()
{
  Dune::FieldMatrix< K, n, n > A( 1e-15 );
  for( int i = 0; i < n; ++i )
    A[ i ][ i ] = K( 1 );
  A.invert();
}

template <class M>
void checkNormNAN(M const &v, int line) {
  if (!std::isnan(v.frobenius_norm())) {
    std::cerr << "error: norm not NaN: frobenius_norm() on line "
              << line << " (type: " << Dune::className(v[0]) << ")"
              << std::endl;
    std::exit(-1);
  }
  if (!std::isnan(v.infinity_norm())) {
    std::cerr << "error: norm not NaN: infinity_norm() on line "
              << line << " (type: " << Dune::className(v[0]) << ")"
              << std::endl;
    std::exit(-1);
  }
}

// Make sure that matrices with NaN entries have norm NaN.
// See also bug flyspray/FS#1147
template <typename T>
void
test_nan(T const &mynan)
{
  T const n(0);
  {
    Dune::FieldMatrix<T, 2, 2> m = {
      { mynan, mynan },
      { mynan, mynan }
    };
    checkNormNAN(m, __LINE__);
  }
  {
    Dune::FieldMatrix<T, 2, 2> m = {
      { mynan, n },
      { n, n }
    };
    checkNormNAN(m, __LINE__);
  }
  {
    Dune::FieldMatrix<T, 2, 2> m = {
      { n, mynan },
      { n, n }
    };
    checkNormNAN(m, __LINE__);
  }
  {
    Dune::FieldMatrix<T, 2, 2> m = {
      { n, n },
      { mynan, n }
    };
    checkNormNAN(m, __LINE__);
  }
  {
    Dune::FieldMatrix<T, 2, 2> m = {
      { n, n },
      { n, mynan }
    };
    checkNormNAN(m, __LINE__);
  }
}

// The computation of infinity_norm_real() was flawed from r6819 on
// until r6915.
void
test_infinity_norms()
{
  using std::abs;

  std::complex<double> threefour(3.0, -4.0);
  std::complex<double> eightsix(8.0, -6.0);

  Dune::FieldMatrix<std::complex<double>, 2, 2> m;
  m[0] = threefour;
  m[1] = eightsix;
  assert(abs(m.infinity_norm()     -20.0) < 1e-10); // max(5+5, 10+10)
  assert(abs(m.infinity_norm_real()-28.0) < 1e-10); // max(7+7, 14+14)
}


template< class K, class K2, int rows, int cols >
void test_interface()
{
  typedef CheckMatrixInterface::UseFieldVector< K2, rows, cols > Traits;
  typedef Dune::FieldMatrix< K, rows, cols > FMatrix;

#if __GNUC__ != 5 || defined(__clang__)
  static_assert(
    !std::is_trivially_copyable<K>::value || std::is_trivially_copyable<FMatrix>::value,
    "FieldMatrix<T, ...> must be trivially copyable type when T is trivial type"
    );
#endif
  static_assert(
    std::is_standard_layout<FMatrix>::value,
    "FieldMatrix<...> must be a standard layout type"
    );

  FMatrix m( 1 );
  checkMatrixInterface< FMatrix >( m );
  checkMatrixInterface< FMatrix, Traits >( m );
}

void test_initialisation()
{
  [[maybe_unused]] Dune::FieldMatrix<int, 2, 2> const A = {
    { 1, 2 },
    { 3, 4 }
  };

  assert(A[0][0] == 1);
  assert(A[0][1] == 2);
  assert(A[1][0] == 3);
  assert(A[1][1] == 4);
}

int main()
{
  try {
    int errors = 0; // counts errors

    static_assert(
      std::is_same< Dune::FieldMatrix<double, 3, 3>, Dune::FieldMatrix<double, 3> >::value,
      "default parameter for square matrices"
      );

    {
      double nan = std::nan("");
      test_nan(nan);
    }
    {
      std::complex<double> nan( std::nan(""), 17 );
      test_nan(nan);
    }
    test_infinity_norms();
    test_initialisation();

    // test 1 x 1 matrices
    test_interface<float, float, 1, 1>();
    test_matrix<float, float, float, 1, 1>();
    ScalarOperatorTest<float>();
    test_matrix<double, double, double, 1, 1>();
    ScalarOperatorTest<double>();
#if HAVE_QUADMATH
    test_matrix<Dune::Float128, Dune::Float128, Dune::Float128, 1, 1>();
    ScalarOperatorTest<Dune::Float128>();
#endif
    // test n x m matrices
    test_interface<int, int, 10, 5>();
    test_matrix<int, int, int, 10, 5>();
    test_matrix<double, double, double, 5, 10>();
    test_interface<double, double, 5, 10>();
#if HAVE_QUADMATH
    test_matrix<Dune::Float128, Dune::Float128, Dune::Float128, 5, 10>();
    test_interface<Dune::Float128, Dune::Float128, 5, 10>();
#endif
    // mixed precision
    test_interface<float, float, 5, 10>();
    test_matrix<float, double, float, 5, 10>();
#if HAVE_QUADMATH
    test_matrix<float, double, Dune::Float128, 5, 10>();
#endif
    // test complex matrices
    test_matrix<std::complex<float>, std::complex<float>, std::complex<float>, 1, 1>();
    test_matrix<std::complex<double>, std::complex<double>, std::complex<double>, 5, 10>();
    // test complex/real matrices mixed case
    test_matrix<float, std::complex<float>, std::complex<float>, 1, 1>();
    test_matrix<std::complex<float>, float, std::complex<float>, 1, 1>();
#if HAVE_LAPACK
    // test eigemvalue computation
    test_ev<double>();
#endif
    // test high level methods
    errors += test_determinant< double >();
#if HAVE_VC
    errors += test_determinant< Vc::SimdArray<double, 8> >();
#endif

    //test LoopSIMD stuff
    errors += test_determinant< Dune::LoopSIMD<double, 8> >();

    test_invert< float, 34 >();
    test_invert< double, 34 >();
    test_invert< std::complex< long double >, 2 >();
    test_invert< std::complex< float >, 2 >();
    errors += test_invert_solve();

    {  // Test whether multiplying one-column matrices by scalars work
      FieldMatrix<double,3,1> A = {1,2,3};
      double v = 0;
      FieldVector<double,3> f = {2,3,4};
      double vT = 0;
      FieldVector<double,3> fT = {3,4,5};
      test_mult(A, v, f, vT, fT);
    }

    {  // Test whether result of multiplying a one-row matrix can be a scalar
      FieldMatrix<double,1,3> A = {{1,2,3}};
      FieldVector<double,3> v = {2,3,4};
      double f = 0;
      FieldVector<double,3> vT = {3,4,5};
      double fT = 0;
      test_mult(A, v, f, vT, fT);
    }

    {  // Test multiplication of 1x1 matrix with scalars
      FieldMatrix<double,1,1> A = {42};
      double v = 0;
      double f = 2;
      double vT = 0;
      double fT = 5;
      test_mult(A, v, f, vT, fT);
    }

    return (errors > 0 ? 1 : 0); // convert error count to unix exit status
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
    return 1;
  }
}
