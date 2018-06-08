// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "config.h"

#include <cassert>
#include <iostream>

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/float_cmp.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/test/testsuite.hh>

using namespace Dune;

template <class T>
struct Comparator
{
  bool operator()(T const& x, T const& y) { return Dune::FloatCmp::eq(x, y); }
};


int main()
{
  // check vector and matrix type with Float128 field type
  TestSuite test{};
  Comparator<Float128> cmp{};

  FieldVector<Float128,3> v{1,2,3}, x;
  FieldMatrix<Float128,3,3> M{ {1,2,3}, {2,3,4}, {3,4,6} }, A;
  FieldMatrix<Float128,3,3> M2{ {1,2,3}, {2,3,4}, {3,4,7} };

  auto y1 = v.one_norm();
  test.check(cmp(y1, 6.q), "vec.one_norm()");

  auto y2 = v.two_norm();
  test.check(cmp(y2, sqrtq(14.q)), "vec.two_norm()");

  auto y3 = v.infinity_norm();
  test.check(cmp(y3, 3.q), "vec.infinity_norm()");

  M.mv(v, x);   // x = M*v
  M.mtv(v, x);  // x = M^T*v
  M.umv(v, x);  // x+= M*v
  M.umtv(v, x); // x+= M^T*v
  M.mmv(v, x);  // x-= M*v
  M.mmtv(v, x); // x-= M^T*v

  auto w1 = M.infinity_norm();
  test.check(cmp(w1, 13.q), "mat.infinity_norm()");

  auto w2 = M.determinant();
  test.check(cmp(w2, -1.q), "mat.determinant()");

  M.solve(v, x);  // x = M^(-1)*v

  auto M3 = M.leftmultiplyany(M2);
  auto M4 = M.rightmultiplyany(M2);

  using namespace FMatrixHelp;

  invertMatrix(M,A);

  constexpr Float128 y = 0.5;

  // test cmath functions for Float128 type
  test.check(cmp(abs(Float128{-1}),Float128{1}), "abs");
  test.check(cmp(cos(acos(Float128{0.5})),Float128{0.5}), "cos(acos)");
  test.check(cmp(cosh(acosh(Float128{1.5})),Float128{1.5}), "cosh(acosh)");
  test.check(cmp(sin(asin(Float128{0.5})),Float128{0.5}), "sin(asin)");
  test.check(cmp(sinh(asinh(Float128{0.5})),Float128{0.5}), "sinh(asinh)");
  test.check(cmp(tan(atan(Float128{0.5})),Float128{0.5}), "tan(atan)");
  test.check(cmp(atan2(Float128{1},Float128{2}), atan(Float128{0.5})), "atan2");
  test.check(cmp(tanh(atanh(Float128{0.5})),Float128{0.5}), "tanh(atanh)");
  test.check(cmp(cbrt(Float128{0.5*0.5*0.5}),Float128{0.5}), "cbrt");
  test.check(cmp(ceil(Float128{0.6}),Float128{1}), "ceil");
  // TODO: add checks for all the other cmath functions

}
