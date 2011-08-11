// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/static_assert.hh>
#include <dune/common/classname.hh>
#include <iostream>
#include <complex>
#include <typeinfo>


using Dune::FieldVector;
using std::complex;

template<class ft, class rt, int d>
struct FieldVectorMainTest
{
  FieldVectorMainTest() {
    ft a = 1;
    FieldVector<ft,d> v(1);
    FieldVector<ft,d> w(2);
    FieldVector<ft,d> z(2);
    const FieldVector<ft,d> x(z);
    a = x[0];
    bool b;
    rt n;

    std::cout << __func__ << "\t ( " << className(v) << " )" << std::endl;

    // test traits
    dune_static_assert(
      ( Dune::is_same< typename Dune::FieldTraits<
                FieldVector<ft,d> >::field_type, ft >::value ),
      "FieldTraits<FieldVector> yields wrong field_type"
      );
    dune_static_assert(
      ( Dune::is_same< typename Dune::FieldTraits<ft>::real_type, rt >::value ),
      "FieldTraits<field_type> yields wrong real_type"
      );
    dune_static_assert(
      ( Dune::is_same< typename Dune::FieldTraits<
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
    FieldVector<ft,d> z2 = v + w;
    w -= v;
    w += v;

    // test op(vec,scalar)
    w +=a;
    w -= a;
    w *= a;
    w /= a;

    // test scalar product, axpy
    a = v * w;
    z = v.axpy(a,w);

    // test comparison
    b = (w != v);
    b = (w == v);

    // assignment to vector of complex
    FieldVector< std::complex<rt> ,d> cv = v;
    cv = a;
    const FieldVector< std::complex<rt> ,d> ccv = x;

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
#if DUNE_COMMON_FIELDVECTOR_SIZE_IS_METHOD
    size = w.size();
#endif
  }
};


template<class ft>
struct ScalarOperatorTest
{
  ScalarOperatorTest()
  {
    ft a = 1;
    ft c = 2;
    FieldVector<ft,1> v(2);
    FieldVector<ft,1> w(2);
    bool b;

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
    a += (ft)1; // make sure a!=0
    a = v / a;

    v = v + a;
    v = v - a;
    v = v * a;
    a += (ft)1; // make sure a!=0
    v = v / a;

    a = a + v;
    a = a - v;
    a = a * v;
    v += (ft)1; // make sure v!=0
    a = a / v;

    v = a + v;
    v = a - v;
    v = a * v;
    v += (ft)1; // make sure v!=0
    v = a / v;

    v -= v;
    v -= a;
    v += v;
    v += a;
    v *= a;
    a += (ft)1; // make sure a!=0
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
    bool b;

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

template<class ft, int d>
struct FieldVectorTest
{
  FieldVectorTest()
  {
    // --- test complex and real valued vectors
    FieldVectorMainTest<ft,ft,d>();
    FieldVectorMainTest<complex<ft>,ft,d>();
    // --- test next lower dimension
    FieldVectorTest<ft,d-1>();
  }
};

// specialization for 1d vector
template<class ft>
class FieldVectorTest<ft,1>
{
public:
  FieldVectorTest()
  {
    // --- real valued
    FieldVectorMainTest<ft,ft,1>();
    ScalarOperatorTest<ft>();
    ScalarOrderingTest<ft>();
    // --- complex valued
    FieldVectorMainTest<complex<ft>,ft,1>();
    ScalarOperatorTest< complex<ft> >();
    // ordering doesn't work for complex numbers

    // --- test next lower dimension
    FieldVectorMainTest<ft,ft,0>();
  }
};

int main()
{
  try {
    FieldVectorTest<int, 3>();
    FieldVectorTest<float, 3>();
    FieldVectorTest<double, 3>();
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
