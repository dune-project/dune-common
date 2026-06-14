// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <limits>

#include <dune/common/bigfloat.hh>
#include <dune/common/float_cmp.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/fvector.hh>
#include <dune/common/math.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/test/testsuite.hh>

template <class T>
struct Comparator
{
  Comparator(T tol)
    : tol_(tol)
  {}

  bool operator()(T x, T y) const
  {
    return Dune::FloatCmp::eq<T,Dune::FloatCmp::absolute>(x, y, tol_);
  }

private:
  T tol_;
};

int main ()
{
  Dune::TestSuite test;

  // check mathematical constants

  using F = double;
  auto e0 = Dune::StandardMathematicalConstants<F>::e();
  auto pi0 = Dune::StandardMathematicalConstants<F>::pi();

  using G = Dune::BigFloat<128>;
  auto e = Dune::StandardMathematicalConstants<G>::e();
  auto pi = Dune::StandardMathematicalConstants<G>::pi();

  Comparator<G> cmp{std::numeric_limits<G>::epsilon() * 8};
  Comparator<G> weakcmp{cbrt(std::numeric_limits<G>::epsilon())};
  auto tol = std::numeric_limits<F>::epsilon();

  using std::abs;
  test.check(abs(e0 - e) < tol);
  test.check(abs(pi0 - pi) < tol);

  // implicit conversion
  G x1 = int(3);
  G x2 = float(pi);
  G x3 = double(pi);
  G x4 = (long double)(pi);
#if HAVE_QUADMATH
  G x5 = Dune::StandardMathematicalConstants<Dune::Float128>::pi();
#endif

  [[maybe_unused]] int z1 = int(x1);
  [[maybe_unused]] float z2 = float(x2);
  [[maybe_unused]] double z3 = double(x3);
  [[maybe_unused]] long double z4 = (long double)(x4);
#if HAVE_QUADMATH
  [[maybe_unused]] Dune::Float128 z5 = (Dune::Float128)(x5);
#endif

  // assignment
  x1 = z1;
  x2 = z2;
  x3 = z3;
  x4 = z4;
#if HAVE_QUADMATH
  x5 = z5;
#endif

  [[maybe_unused]] G xx1 = z1;
  [[maybe_unused]] G xx2 = z2;
  [[maybe_unused]] G xx3 = z3;
  [[maybe_unused]] G xx4 = z4;
#if HAVE_QUADMATH
  [[maybe_unused]] G xx5 = z5;
#endif

  test.check(abs(x1 - xx1) < tol);
  test.check(abs(x2 - xx2) < tol);
  test.check(abs(x3 - xx3) < tol);
  test.check(abs(x4 - xx4) < tol);
#if HAVE_QUADMATH
  test.check(abs(x5 - xx5) < tol);
#endif

  // mixed type operation
  test.check(abs(z2 - xx2) < tol);

  // field-vector
  Dune::FieldVector<G,3> v{1,2,3}, x;
  Dune::FieldMatrix<G,3,3> M{ {1,2,3}, {2,3,4}, {3,4,6} }, A;
  Dune::FieldMatrix<G,3,3> M2{ {1,2,3}, {2,3,4}, {3,4,7} };

  auto y1 = v.one_norm();
  test.check(cmp(y1, 6.0), "vec.one_norm()");

  auto y2 = v.two_norm();
  test.check(cmp(y2, sqrt(G(14.0))), "vec.two_norm()");

  auto y3 = v.infinity_norm();
  test.check(cmp(y3, 3.0), "vec.infinity_norm()");

  M.mv(v, x);   // x = M*v
  M.mtv(v, x);  // x = M^T*v
  M.umv(v, x);  // x+= M*v
  M.umtv(v, x); // x+= M^T*v
  M.mmv(v, x);  // x-= M*v
  M.mmtv(v, x); // x-= M^T*v

  auto w1 = M.infinity_norm();
  test.check(cmp(w1, 13.0), "mat.infinity_norm()");

  auto w2 = M.determinant();
  test.check(cmp(w2, -1.0), "mat.determinant()");

  M.solve(v, x);  // x = M^(-1)*v

  [[maybe_unused]] auto M3 = M.leftmultiplyany(M2);
  [[maybe_unused]] auto M4 = M.rightmultiplyany(M2);

  using namespace Dune::FMatrixHelp;
  invertMatrix(M,A);

  // test cmath functions for BigFloat type
  test.check(cmp(G(0.5), G("0.5")), "string constructor");

  test.check(cmp(abs(G{-1}),G{1}), "abs");
  test.check(cmp(fabs(G{-1}),G{1}), "fabs");

  test.check(cmp(cos(acos(G{0.5})),G{0.5}), "cos(acos)");
  test.check(cmp(cosh(acosh(G{1.5})),G{1.5}), "cosh(acosh)");
  test.check(cmp(sin(asin(G{0.5})),G{0.5}), "sin(asin)");
  test.check(cmp(sinh(asinh(G{0.5})),G{0.5}), "sinh(asinh)");
  test.check(cmp(tan(atan(G{0.5})),G{0.5}), "tan(atan)");
  test.check(cmp(atan2(G{1},G{2}), atan(G{0.5})), "atan2");
  test.check(cmp(tanh(atanh(G{0.5})),G{0.5}), "tanh(atanh)");

  // test.check(cmp(fdim(G{4},G{1}),G{3}), "fdim"); // a > b ? a - b : +0
  test.check(cmp(fma(G{0.5},G{0.4},G{1.8}),(G{0.5} * G{0.4}) + G{1.8}), "fma");
  test.check(cmp(fmax(G{0.6},G{0.4}),G{0.6}), "fmax");
  test.check(cmp(fmin(G{0.6},G{0.4}),G{0.4}), "fmin");
  test.check(cmp(hypot(G{1.6}, G{2.3}), sqrt(G{1.6}*G{1.6} + G{2.3}*G{2.3})), "hypot");
  // ilogb
  // test.check(cmp(llrint(G{2.3}),(long long int)(2)), "llrint");
  // test.check(cmp(lrint(G{2.3}),(long int)(2)), "lrint");
  test.check(cmp(rint(G{2.3}),G{2}), "lrint");
  test.check(cmp(llround(G{2.3}),(long long int)(2)), "llround");
  test.check(cmp(lround(G{2.3}),(long int)(2)), "lround");
  test.check(cmp(round(G{2.3}),G{2}), "round");
  // test.check(cmp(nearbyint(G{2.3}),G{2}), "nearbyint");
  test.check(cmp(trunc(G{2.7}),G{2}), "trunc");
  test.check(cmp(ceil(G{1.6}),G{2}), "ceil");
  test.check(cmp(floor(G{1.6}),G{1}), "floor");

  test.check(cmp(log(exp(G{1.5})),G{1.5}), "log(exp)");
  test.check(cmp(exp(G{0.2}+G{0.4}), exp(G{0.2})*exp(G{0.4})), "exp"); // exp(a+b) = exp(a)*exp(b)
  test.check(cmp(expm1(G{0.6}),exp(G{0.6})-G{1}), "expm1");
  test.check(cmp(log10(G{1000}),G{3}), "log10");
  test.check(cmp(log2(G{8}),G{3}), "log2");
  test.check(cmp(log1p(G{1.6}),log(G{1} + G{1.6})), "log1p");
  // nextafter

  // these two functions produce larger errors
  test.check(weakcmp(fmod(G{5.1},G{3}),G{2.1}), "fmod");
  test.check(weakcmp(remainder(G{5.1},G{3}),G{-0.9}), "remainder");

  test.check(cmp(pow(G{2},G{3}),G{8}), "pow");
  test.check(cmp(pow(pi,G{3}),pow(pi,3)), "pow"); // compare pow with float exponent and integer exponent
  test.check(cmp(cbrt(G{0.5*0.5*0.5}),G{0.5}), "cbrt");
  test.check(cmp(sqrt(G{4}),G{2}), "sqrt");

  test.check(cmp(erf(G{0}),G{0}), "erf");
  test.check(cmp(erfc(G{0.6}), G{1}-erf(G{0.6})), "erfc");
  test.check(cmp(lgamma(G{3}),log(G{2})), "lgamma");
  test.check(cmp(tgamma(G{3}),G{2}), "tgamma");

  return test.exit();
}
