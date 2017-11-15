// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
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
#include <dune/common/rangeutilities.hh>
#include <dune/common/simd.hh>
#include <dune/common/unused.hh>
#include <dune/common/vc.hh>

#include "checkmatrixinterface.hh"

using namespace Dune;

template<typename T, std::size_t n>
int test_invert_solve(Dune::FieldMatrix<double, n, n> &A,
                      Dune::FieldMatrix<double, n, n> &inv,
                      Dune::FieldVector<double, 3> &x,
                      Dune::FieldVector<double, 3> &b)
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
  if (prod.infinity_norm() > 1e-6) {
    std::cerr<<"Given inverse wrong"<<std::endl;
    equal=false;
  }

  FieldMatrix<T,n,n> copy(A);
  A.invert();

  calced_inv = A;
  A-=inv;


  double singthres = FMatrixPrecision<>::singular_limit()*10;
  for(size_t i =0; i < n; ++i)
    for(size_t j=0; j <n; ++j)
      if(abs(A[i][j])>singthres) {
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
    if(abs(xcopy[i])>singthres) {
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
  return ret + test_invert_solve<double, 3>(A_data2, inv_data2, x2, b2);
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
    // Test that operator= and opeator-= work before we can test anything else
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
    FM DUNE_UNUSED A3 = (A2 *= 3); // A2 == A3 == 6*A
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
  if (any_true(abs(B.determinant() + 2.0) > 1e-12))
  {
    std::cerr << "Determinant 1 test failed (" << Dune::className<T>() << ")"
              << std::endl;
    ++ret;
  }

  B[0][0] =  3.0; B[0][1] =  0.0; B[0][2] =  1.0; B[0][3] =  0.0;
  B[1][0] = -1.0; B[1][1] =  3.0; B[1][2] =  0.0; B[1][3] =  0.0;
  B[2][0] = -3.0; B[2][1] =  0.0; B[2][2] = -1.0; B[2][3] =  2.0;
  B[3][0] = -1.0; B[3][1] =  3.0; B[3][2] =  0.0; B[3][3] =  2.0;
  if (any_true(B.determinant() != 0.0))
  {
    std::cerr << "Determinant 2 test failed (" << Dune::className<T>() << ")"
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
    bool b DUNE_UNUSED;

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

    v -= v;
    v -= a;
    v += v;
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

  FMatrix m( 1 );
  checkMatrixInterface< FMatrix >( m );
  checkMatrixInterface< FMatrix, Traits >( m );
}

void test_initialisation()
{
  DUNE_UNUSED Dune::FieldMatrix<int, 2, 2> const A = {
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
    // test n x m matrices
    test_interface<int, int, 10, 5>();
    test_matrix<int, int, int, 10, 5>();
    test_matrix<double, double, double, 5, 10>();
    test_interface<double, double, 5, 10>();
    // mixed precision
    test_interface<float, float, 5, 10>();
    test_matrix<float, double, float, 5, 10>();
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
    test_invert< float, 34 >();
    test_invert< double, 34 >();
    test_invert< std::complex< long double >, 2 >();
    errors += test_invert_solve();

    return (errors > 0 ? 1 : 0); // convert error count to unix exit status
  }
  catch (Dune::Exception & e)
  {
    std::cerr << "Exception: " << e << std::endl;
    return 1;
  }
}
