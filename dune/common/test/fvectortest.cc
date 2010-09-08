// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dune/common/fvector.hh>
#include <dune/common/exceptions.hh>
#include <iostream>

using Dune::FieldVector;

template<class ct, int d>
class FieldVectorMainTest
{
protected:
  FieldVectorMainTest() {
    ct a = 1;
    FieldVector<ct,d> v(1);
    FieldVector<ct,d> w(2);
    FieldVector<ct,d> z(2);
    bool b;

    // Test whether the norm methods compile
    (w+v).two_norm();
    (w+v).two_norm2();
    (w+v).one_norm();
    (w+v).one_norm_real();
    (w+v).infinity_norm();
    (w+v).infinity_norm_real();

    // test op(vec,vec)
    z = v + w;
    z = v - w;
    FieldVector<ct,d> z2 = v + w;
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

template<class ct, int d>
class FieldVectorTest : public FieldVectorMainTest<ct,d>
{
public:
  FieldVectorTest() : FieldVectorMainTest<ct,d>() {}
};

template<class ct>
class FieldVectorTest<ct,1>: public FieldVectorMainTest<ct,1>
{
public:
  FieldVectorTest() : FieldVectorMainTest<ct,1>()
  {
    ct a = 1;
    FieldVector<ct,1> v(2);
    FieldVector<ct,1> w(2);
    bool b;

    v = a;
    v = w = v;
    a = v;

    b = (v == a);
    b = (a == v);

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
  }
};

int main()
{
  try {
    FieldVectorTest<int, 0>();
    FieldVectorTest<int, 1>();
    FieldVectorTest<int, 2>();
    FieldVectorTest<int, 3>();
    FieldVectorTest<float, 0>();
    FieldVectorTest<float, 1>();
    FieldVectorTest<float, 2>();
    FieldVectorTest<float, 3>();
    FieldVectorTest<double, 0>();
    FieldVectorTest<double, 1>();
    FieldVectorTest<double, 2>();
    FieldVectorTest<double, 3>();
  } catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }
}
