// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
  Comparator(T tol)
    : tol_(tol)
  {}

  bool operator()(T const& x, T const& y)
  {
    return Dune::FloatCmp::eq<T,FloatCmp::absolute>(x, y, tol_);
  }

private:
  T tol_;
};

int main()
{
  // check vector and matrix type with Float128 field type
  TestSuite test{};
  Comparator<Float128> cmp{std::numeric_limits<Float128>::epsilon() * 8};
  Comparator<Float128> weakcmp{cbrt(std::numeric_limits<Float128>::epsilon())};

  // implicit conversion
  Float128 x1 = 1;
  Float128 x2 = 1.0f;
  Float128 x3 = 1.0;
  Float128 x4 = 1.0l;

  [[maybe_unused]] int z1 = x1;
  [[maybe_unused]] float z2 = x2;
  [[maybe_unused]] double z3 = x3;
  [[maybe_unused]] long double z4 = x4;

  // field-vector
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

  [[maybe_unused]] auto M3 = M.leftmultiplyany(M2);
  [[maybe_unused]] auto M4 = M.rightmultiplyany(M2);

  using namespace FMatrixHelp;

  invertMatrix(M,A);

  // test cmath functions for Float128 type
  using T = Float128;
  test.check(cmp(T(0.5), T("0.5")), "string constructor");

  test.check(cmp(abs(T{-1}),T{1}), "abs");
  test.check(cmp(fabs(T{-1}),T{1}), "fabs");

  test.check(cmp(cos(acos(T{0.5})),T{0.5}), "cos(acos)");
  test.check(cmp(cosh(acosh(T{1.5})),T{1.5}), "cosh(acosh)");
  test.check(cmp(sin(asin(T{0.5})),T{0.5}), "sin(asin)");
  test.check(cmp(sinh(asinh(T{0.5})),T{0.5}), "sinh(asinh)");
  test.check(cmp(tan(atan(T{0.5})),T{0.5}), "tan(atan)");
  test.check(cmp(atan2(T{1},T{2}), atan(T{0.5})), "atan2");
  test.check(cmp(tanh(atanh(T{0.5})),T{0.5}), "tanh(atanh)");

  test.check(cmp(fdim(T{4},T{1}),T{3}), "fdim"); // a > b ? a - b : +0
  test.check(cmp(fma(T{0.5},T{0.4},T{1.8}),(T{0.5} * T{0.4}) + T{1.8}), "fma");
  test.check(cmp(fmax(T{0.6},T{0.4}),T{0.6}), "fmax");
  test.check(cmp(fmin(T{0.6},T{0.4}),T{0.4}), "fmin");
  test.check(cmp(hypot(T{1.6}, T{2.3}), sqrt(T{1.6}*T{1.6} + T{2.3}*T{2.3})), "hypot");
  // ilogb
  test.check(cmp(llrint(T{2.3}),(long long int)(2)), "llrint");
  test.check(cmp(lrint(T{2.3}),(long int)(2)), "lrint");
  test.check(cmp(rint(T{2.3}),T{2}), "lrint");
  test.check(cmp(llround(T{2.3}),(long long int)(2)), "llround");
  test.check(cmp(lround(T{2.3}),(long int)(2)), "lround");
  test.check(cmp(round(T{2.3}),T{2}), "round");
  test.check(cmp(nearbyint(T{2.3}),T{2}), "nearbyint");
  test.check(cmp(trunc(T{2.7}),T{2}), "trunc");
  test.check(cmp(ceil(T{1.6}),T{2}), "ceil");
  test.check(cmp(floor(T{1.6}),T{1}), "floor");

  test.check(cmp(log(exp(T{1.5})),T{1.5}), "log(exp)");
  test.check(cmp(exp(T{0.2}+T{0.4}), exp(T{0.2})*exp(T{0.4})), "exp"); // exp(a+b) = exp(a)*exp(b)
  test.check(cmp(expm1(T{0.6}),exp(T{0.6})-T{1}), "expm1");
  test.check(cmp(log10(T{1000}),T{3}), "log10");
  test.check(cmp(log2(T{8}),T{3}), "log2");
  test.check(cmp(log1p(T{1.6}),log(T{1} + T{1.6})), "log1p");
  // nextafter

  // these two functions produce larger errors
  test.check(weakcmp(fmod(T{5.1},T{3}),T{2.1}), "fmod");
  test.check(weakcmp(remainder(T{5.1},T{3}),T{-0.9}), "remainder");

  test.check(cmp(pow(T{2},T{3}),T{8}), "pow");
  test.check(cmp(pow(T{M_PIq},T{3}),pow(T{M_PIq},3)), "pow"); // compare pow with float exponent and integer exponent
  test.check(cmp(cbrt(T{0.5*0.5*0.5}),T{0.5}), "cbrt");
  test.check(cmp(sqrt(T{4}),T{2}), "sqrt");

  test.check(cmp(erf(T{0}),T{0}), "erf");
  test.check(cmp(erfc(T{0.6}), T{1}-erf(T{0.6})), "erfc");
  test.check(cmp(lgamma(T{3}),log(T{2})), "lgamma");
  test.check(cmp(tgamma(T{3}),T{2}), "tgamma");
}
