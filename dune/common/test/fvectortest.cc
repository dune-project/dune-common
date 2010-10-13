// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/static_assert.hh>
#include <iostream>

using Dune::FieldVector;

template<class ft, class rt, int d>
class FieldVectorMainTest
{
protected:
  FieldVectorMainTest() {
    ft a = 1;
    FieldVector<ft,d> v(1);
    FieldVector<ft,d> w(2);
    FieldVector<ft,d> z(2);
    bool b;
    rt n;

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

    // test istream operator
    std::stringstream s;
    for (int i=0; i<d; i++)
    {
      s << i << " ";
      v[i] = i;
    }
    s >> w;
    assert(v == w);
  }
};

template<class ft, class rt, int d>
class FieldVectorTest : public FieldVectorMainTest<ft,rt,d>
{
public:
  FieldVectorTest() : FieldVectorMainTest<ft,rt,d>() {}
};

template<class ft, class rt>
class FieldVectorTest<ft,rt,1>: public FieldVectorMainTest<ft,rt,1>
{
public:
  FieldVectorTest() : FieldVectorMainTest<ft,rt,1>()
  {
    ft a = 1;
    FieldVector<ft,1> v(2);
    FieldVector<ft,1> w(2);
    bool b;

    v = a;
    v = w = v;
    a = v;

    b = (v == a);
    b = (v != a);
    b = (a == v);
    b = (a != v);

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

    b = (v<a);
    b = (v<=a);
    b = (v>=a);
    b = (v>a);

    b = (v<w);
    b = (v<=w);
    b = (v>=w);
    b = (v>w);

    b = (a<w);
    b = (a<=w);
    b = (a>=w);
    b = (a>w);
  }
};

int main()
{
  try {
    FieldVectorTest<int, int, 0>();
    FieldVectorTest<int, int, 1>();
    FieldVectorTest<int, int, 2>();
    FieldVectorTest<int, int, 3>();
    FieldVectorTest<float, float, 0>();
    FieldVectorTest<float, float, 1>();
    FieldVectorTest<float, float, 2>();
    FieldVectorTest<float, float, 3>();
    FieldVectorTest<double, double, 0>();
    FieldVectorTest<double, double, 1>();
    FieldVectorTest<double, double, 2>();
    FieldVectorTest<double, double, 3>();
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
