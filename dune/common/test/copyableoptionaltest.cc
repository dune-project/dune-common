// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/copyableoptional.hh>

#if __cpp_concepts >= 201907L && __cpp_lib_concepts >= 201806L
  #include <concepts>
  #define DUNE_HAVE_CXX_CONCEPTS 1
#endif

template <class F>
class FunctionWrapper
{
public:
  FunctionWrapper (const F& fct)
    : fct_{fct}
  {}

  // implement the evaluation operator to provide a functor interface
  template <class... Args>
  decltype(auto) operator() (Args&&... args) const
  {
    return (*fct_)(std::forward<Args>(args)...);
  }

private:
  Dune::CopyableOptional<F> fct_;
};

class A
{
public:
  int value_;

  A() : value_{5} {}
  explicit A(int value) : value_{value} {}
  A(A const&) = default;
  A(A&&) = delete;

  A& operator=(A const&) = delete;
  A& operator=(A&&) = delete;
};


class B
{
public:
  int value1_;
  int value2_;

  B(int value1, int value2) : value1_{value1}, value2_{value2} {}
  B(B const&) = default;
  B(B&&) = default;

  B& operator=(B const&) = delete;
  B& operator=(B&&) = delete;
};

int main()
{
  using namespace Dune;

  int c = 7;
  FunctionWrapper f{[&c](auto x) { return c*x; }};

  // evaluation
  [[maybe_unused]] auto y = f(6);
  assert(y == 42);

  // copy and move operations
  FunctionWrapper f2 = f;
  FunctionWrapper f3 = std::move(f2);
  f2 = f3;
  f3 = std::move(f2);
  assert(f3(6) == 42);

  using CopyableA = Dune::CopyableOptional<A>;

  // forward of constructor and assignment
  CopyableA co{7};
  assert(co->value_ == 7);
  co = CopyableA{91};
  assert(co->value_ == 91);
  co = 42;
  assert(co->value_ == 42);

  // CopyableA ca = 35; // ERROR: conversion from int to CopyableA (correct error)

  // default initialization
  CopyableA sro{};
  assert(sro->value_ == 5);

  using CopyableB = Dune::CopyableOptional<B>;
  CopyableB cb{7,13};

#if DUNE_HAVE_CXX_CONCEPTS
  static_assert(std::copyable<CopyableA>);
  static_assert(std::copyable<decltype(f)>);
#endif

  return 0;
}
