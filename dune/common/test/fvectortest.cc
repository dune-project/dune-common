// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/classname.hh>
#include <dune/common/gmpfield.hh>
#include <iostream>
#include <complex>
#include <typeinfo>
#include <cassert>


using Dune::FieldVector;
using std::complex;

// Tests that can be run without the construction of complex<rt>
template<class ft, class rt, int d>
struct FieldVectorMainTestCommons
{
  FieldVectorMainTestCommons() {
    ft a = 1;
    FieldVector<ft,d> v(1);
    FieldVector<ft,d> w(2);
    FieldVector<ft,d> z(2);
    const FieldVector<ft,d> x(z);
    if (x.size()>0)
      a = x[0];
    bool b DUNE_UNUSED;
    rt n DUNE_UNUSED;

    std::cout << __func__ << "\t ( " << className(v) << " )" << std::endl;

    // test exported types
    static_assert(
      std::is_same<ft,typename FieldVector<ft,d>::value_type>::value,
      "FieldVector::value_type is not the correct type"
    );

    // test traits
    static_assert(
      ( std::is_same< typename Dune::FieldTraits<
                FieldVector<ft,d> >::field_type, ft >::value ),
      "FieldTraits<FieldVector> yields wrong field_type"
      );
    static_assert(
      ( std::is_same< typename Dune::FieldTraits<ft>::real_type, rt >::value ),
      "FieldTraits<field_type> yields wrong real_type"
      );
    static_assert(
      ( std::is_same< typename Dune::FieldTraits<
                FieldVector<ft,d> >::real_type, rt >::value ),
      "FieldTraits<FieldVector> yields wrong real_type"
      );

    // Test whether the norm methods compile
    n = (w+v).two_norm();
    n = (w+v).two_norm2();
    n = (w+v).one_norm();
    n = (w+v).one_norm_real();
    n = (w+v).infinity_norm();
    n = (w+v).infinity_norm_real();

    // test op(vec,vec)
    z = v + w;
    z = v - w;
    FieldVector<ft,d> z2 DUNE_UNUSED = v + w;
    w -= v;
    w += v;

    // test op(vec,scalar)
    w +=a;
    w -= a;
    w *= a;
    w /= a;

    // test scalar product, axpy
    a = v * w;
    a = v.dot(w);
    z = v.axpy(a,w);

    // test comparison
    b = (w != v);
    b = (w == v);

    // test istream operator
    std::stringstream s;
    for (int i=0; i<d; i++)
    {
      s << i << " ";
      v[i] = i;
    }
    s >> w;
    assert(v == w);

    // test container methods
    typename FieldVector<ft,d>::size_type size = FieldVector<ft,d>::dimension;
    assert(size == w.size());
  }
};

// Additional tests for floating point types, for which complex<rt> will work
template<class ft, class rt, int d,
         bool floating_point = std::is_floating_point<rt>::value>
struct FieldVectorMainTest
  : FieldVectorMainTestCommons<ft,rt,d>
{
  FieldVectorMainTest()
    : FieldVectorMainTestCommons<ft,rt,d>()
  {
    ft a = 1;
    FieldVector<ft,d> v(1);
    FieldVector<ft,d> z(2);
    const FieldVector<ft,d> x(z);

    // assignment to vector of complex
    FieldVector< std::complex<rt> ,d> cv = v;
    cv = a;
    const FieldVector< std::complex<rt> ,d> ccv DUNE_UNUSED = x;
  }
};

template<class ft, class rt, int d>
struct FieldVectorMainTest<ft,rt,d,false>
  : FieldVectorMainTestCommons<ft,rt,d>
{
  FieldVectorMainTest()
    : FieldVectorMainTestCommons<ft,rt,d>()
  {}
};

template<class ft, class testft=ft>
struct ScalarOperatorTest
{
  ScalarOperatorTest()
  {
    // testft has to initializable with an int
    testft a = 1;
    testft c = 2;
    FieldVector<ft,1> v(2);
    FieldVector<ft,1> w(2);
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
    a += 1; // make sure a!=0
    a = v / a;

    v = v + a;
    v = v - a;
    v = v * a;
    a += 1; // make sure a!=0
    v = v / a;

    a = a + v;
    a = a - v;
    a = a * v;
    v += 1; // make sure v!=0
    a = a / v;

    v = a + v;
    v = a - v;
    v = a * v;
    v += 1; // make sure v!=0
    v = a / v;

    v -= v;
    v -= a;
    v += v;
    v += a;
    v *= a;
    a += 1; // make sure a!=0
    v /= a;

    b = (v == a);
    b = (v != a);
    b = (a == v);
    b = (a != v);

  }
};

// scalar ordering doesn't work for complex numbers
template<class ft>
struct ScalarOrderingTest
{
  ScalarOrderingTest()
  {
    ft a = 1;
    ft c = 2;
    FieldVector<ft,1> v(2);
    FieldVector<ft,1> w(2);
    bool b DUNE_UNUSED;

    std::cout << __func__ << "\t ( " << className(v) << " )" << std::endl;

    b = (a <  c);
    b = (a <= c);
    b = (a >= c);
    b = (a >  c);

    b = (v == a);
    b = (v != a);
    b = (a == v);
    b = (a != v);

    b = (v <  a);
    b = (v <= a);
    b = (v >= a);
    b = (v >  a);

    b = (v <  w);
    b = (v <= w);
    b = (v >= w);
    b = (v >  w);

    b = (a <  w);
    b = (a <= w);
    b = (a >= w);
    b = (a >  w);
  }
};

template<typename T>
struct Epsilon
{
  static T value() { return T(1e-6); }
};

template<>
struct Epsilon<int>
{
  static int value() { return 0; }
};

// scalar ordering doesn't work for complex numbers
template <class rt, int d,
          bool floating_point = std::is_floating_point<rt>::value>
struct DotProductTest
{
  DotProductTest() {
    typedef std::complex<rt> ct;
    DUNE_UNUSED const rt myEps = Epsilon<rt>::value();

    static_assert(
      ( std::is_same< typename Dune::FieldTraits<rt>::real_type, rt>::value ),
      "DotProductTest requires real data type as template parameter!"
      );

    const ct I(0.,1.); // imaginary unit
    const FieldVector<rt,d> one(1.); // vector filled with 1
    const FieldVector<ct,d> iVec(ct(0.,1.)); // vector filled with I

    std::cout << __func__ << "\t \t ( " << Dune::className(one) << " and " << Dune::className(iVec) << ")" << std::endl;

    const bool isRealOne = std::is_same<typename Dune::FieldTraits<rt>::field_type,typename Dune::FieldTraits<rt>::real_type>::value;
    const bool isRealIVec = std::is_same<typename Dune::FieldTraits<ct>::field_type,typename Dune::FieldTraits<ct>::real_type> ::value;
    static_assert(isRealOne,"1-vector expected to be real");
    static_assert(!isRealIVec,"i-vector expected to be complex");

    ct result = ct();
    ct length = ct(d);


    // one^H*one should equal d
    result = dot(one,one);
    assert(std::abs(result-length)<= myEps);
    result = one.dot(one);
    assert(std::abs(result-length)<= myEps);


    // iVec^H*iVec should equal d
    result = dot(iVec,iVec);
    assert(std::abs(result-length)<= myEps);
    result = iVec.dot(iVec);
    assert(std::abs(result-length)<= myEps);


    // test that we do conjugate first argument
    result = dot(one,iVec);
    assert(std::abs(result-length*I)<= myEps);
    result = dot(one,iVec);
    assert(std::abs(result-length*I)<= myEps);


    // test that we do not conjugate second argument
    result = dot(iVec,one);
    assert(std::abs(result+length*I)<= myEps);
    result = iVec.dot(one);
    assert(std::abs(result+length*I)<= myEps);


    // test that dotT does not conjugate at all
    result = dotT(one,one) + one*one;
    assert(std::abs(result-ct(2)*length)<= myEps);
    result = dotT(iVec,iVec) + iVec*iVec;
    assert(std::abs(result+ct(2)*length)<= myEps);
    result = dotT(one,iVec) + one*iVec;
    assert(std::abs(result-ct(2)*length*I)<= myEps);
    result = dotT(iVec,one) + iVec*one;
    assert(std::abs(result-ct(2)*length*I)<= myEps);

  }
};

// scalar ordering doesn't work for complex numbers
template <class rt, int d>
struct DotProductTest<rt, d, false>
{
  DotProductTest() {
    DUNE_UNUSED const rt myEps = Epsilon<rt>::value();

    static_assert(
      ( std::is_same< typename Dune::FieldTraits<rt>::real_type, rt>::value ),
      "DotProductTest requires real data type as template parameter!"
      );

    const FieldVector<rt,d> one(1.); // vector filled with 1

    std::cout << __func__ << "\t \t ( " << Dune::className(one) << " only)" << std::endl;

    const bool isRealOne = std::is_same<typename Dune::FieldTraits<rt>::field_type,typename Dune::FieldTraits<rt>::real_type>::value;
    static_assert(isRealOne,"1-vector expected to be real");

    rt result = rt();
    rt length = rt(d);

    // one^H*one should equal d
    result = dot(one,one);
    assert(abs(result-length)<= myEps);
    result = one.dot(one);
    assert(abs(result-length)<= myEps);

    // test that dotT does not conjugate at all
    result = dotT(one,one) + one*one;
    assert(abs(result-rt(2)*length)<= myEps);
  }
};

template<class ft, int d,
         bool floating_point = std::is_floating_point<ft>::value>
struct FieldVectorTest
{
  FieldVectorTest()
  {
    // --- test complex and real valued vectors
    FieldVectorMainTest<ft,ft,d>();
    FieldVectorMainTest<complex<ft>,ft,d>();
    DotProductTest<ft,d>();
    // --- test next lower dimension
    FieldVectorTest<ft,d-1>();
  }
};

// specialisation for non-floating-point vectors
template<class ft, int d>
struct FieldVectorTest<ft, d, false>
{
  FieldVectorTest()
  {
    // --- test real valued vectors
    FieldVectorMainTest<ft,ft,d>();
    DotProductTest<ft,d>();
    // --- test next lower dimension
    FieldVectorTest<ft,d-1>();
  }
};

// specialization for 1d floating point vector
template<class ft>
class FieldVectorTest<ft,1,true>
{
public:
  FieldVectorTest()
  {
    // --- real valued
    FieldVectorMainTest<ft,ft,1>();
    ScalarOperatorTest<ft>();
    ScalarOrderingTest<ft>();
    DotProductTest<ft,1>();
    // --- complex valued
    FieldVectorMainTest<complex<ft>,ft,1>();
    ScalarOperatorTest< complex<ft> >();
    // ordering doesn't work for complex numbers

    // --- test with an integer
    ScalarOperatorTest< ft, int >();
    // --- test next lower dimension
    FieldVectorMainTest<ft,ft,0>();
  }
};

// specialization for other 1d vectors
template<class ft>
class FieldVectorTest<ft,1,false>
{
public:
  FieldVectorTest()
  {
    // --- real valued
    FieldVectorMainTest<ft,ft,1>();
    ScalarOperatorTest<ft>();
    ScalarOrderingTest<ft>();
    DotProductTest<ft,1>();

    // --- test with an integer
    ScalarOperatorTest< ft, int >();
    // --- test next lower dimension
    FieldVectorMainTest<ft,ft,0>();
  }
};

template <class V>
void checkNormNAN(V const &v, int line) {
  if (!std::isnan(v.one_norm())) {
    std::cerr << "error: norm not NaN: one_norm() on line "
              << line << " (type: " << Dune::className(v[0]) << ")"
              << std::endl;
    std::exit(-1);
  }
  if (!std::isnan(v.two_norm())) {
    std::cerr << "error: norm not NaN: two_norm() on line "
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

// Make sure that vectors with NaN entries have norm NaN.
// See also bug flyspray/FS#1147
template <typename T>
void
test_nan(T const &mynan)
{
  {
    Dune::FieldVector<T, 2> v = { mynan, mynan };
    checkNormNAN(v, __LINE__);
  }
  {
    Dune::FieldVector<T, 2> v = { mynan, 0 };
    checkNormNAN(v, __LINE__);
  }
  {
    Dune::FieldVector<T, 2> v = { 0, mynan };
    checkNormNAN(v, __LINE__);
  }
}

void
test_infinity_norms()
{
  std::complex<double> threefour(3.0, -4.0);
  std::complex<double> eightsix(8.0, -6.0);

  Dune::FieldVector<std::complex<double>, 2> v;
  v[0] = threefour;
  v[1] = eightsix;
  assert(std::abs(v.infinity_norm()     -10.0) < 1e-10); // max(5,10)
  assert(std::abs(v.infinity_norm_real()-14.0) < 1e-10); // max(7,14)
}

void
test_initialisation()
{
  DUNE_UNUSED Dune::FieldVector<int, 2> const b = { 1, 2 };

  assert(b[0] == 1);
  assert(b[1] == 2);
}

int main()
{
  try {
    FieldVectorTest<int, 3>();
    FieldVectorTest<float, 3>();
    FieldVectorTest<double, 3>();
    FieldVectorTest<long double, 3>();
#if HAVE_GMP
    // we skip the complex test and the int test, as these will be very hard to implement with GMPField
    typedef Dune::GMPField<128u> ft;
    FieldVectorMainTest<ft,ft,3>();
    FieldVectorMainTest<ft,ft,2>();
    FieldVectorMainTest<ft,ft,1>();
    FieldVectorMainTest<ft,ft,0>();
    ScalarOperatorTest<ft>();
    ScalarOrderingTest<ft>();
    DotProductTest<ft,3>();
#endif // HAVE_GMP

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
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
