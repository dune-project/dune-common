// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <type_traits>
#include <dune/common/indices.hh>
#include <dune/common/typetraits.hh>

// no types and additional constexpr functions are defined
struct A {
  static constexpr int value = 42;
};

// non constexpr implementation
struct B {
  using type = B;
  using value_type = int;
  static int value;
  operator int () const { return 42; }
  int operator()() const { return 42; }
};
int B::value = 42;

int main()
{
  using namespace Dune;

  static_assert(IsCompileTimeConstant<std::integral_constant<int,5>>::value);
  static_assert(IsCompileTimeConstant<index_constant<5>>::value);
  static_assert(IsCompileTimeConstant<std::bool_constant<true>>::value);
  static_assert(IsCompileTimeConstant<std::is_same<int,double>>::value);

  static_assert(not IsCompileTimeConstant<int>::value);
  static_assert(not IsCompileTimeConstant<A>::value);
  static_assert(not IsCompileTimeConstant<B>::value);
}