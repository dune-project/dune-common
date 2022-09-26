// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dune/common/std/functional.hh>
#include <iostream>
#include <cassert>

struct Foo {
  static int count;
  Foo() { ++count; std::cout << "construct" << std::endl; }
  Foo(const Foo&) { ++count; std::cout << "copy construct" << std::endl; }
  Foo(Foo&&) { ++count; std::cout << "move construct" << std::endl; }
  ~Foo() { --count; std::cout << "destruct" << std::endl; }
};
int Foo::count = 0;

template<typename T>
T&& assert_count(T&& arg, int count)
{
  std::cout << std::decay_t<T>::count << std::endl;
  if (std::decay_t<T>::count != count)
    std::cerr << "Passed count does not match state of the argument" << std::endl;
  return std::forward<T>(arg);
}

int main()
{
  auto id = Dune::Std::identity();

  assert_count(id(Foo()),1); // pass an r-value to identity, still constructed on the assert

  const auto& foo0 = id(Foo()); // pass an r-value to identity
  assert_count(foo0,0); // id(Foo()) is already destructed at this point

  auto foo1 = id(Foo()); // pass an r-value to identity and move it to foo1
  assert_count(foo1,1); // foo0 is already destructed at this point

  Foo foo2;
  assert_count(id(foo2),2); // pass an l-value to identity

  const auto& foo3 = id(foo2); // pass an l-value to identity
  assert_count(foo3,2); // foo still exist at this point

  auto foo4 = id(foo2); // pass an l-value to identity and copy its result
  assert_count(foo4,3); // copy of foo still exist at this point
}
