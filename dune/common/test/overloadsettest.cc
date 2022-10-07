// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <tuple>

#include <dune/common/overloadset.hh>
#include <dune/common/hybridutilities.hh>

#include <dune/common/test/testsuite.hh>

struct Bar {
  int bar() const { return 0; }
};



int main()
{
  Dune::TestSuite test;

  {
    auto foo = Dune::overload(
            [](double /*i*/) { return 0; },
            [](int /*i*/) { return 1; },
            [](long /*i*/) { return 2; });

    test.check(foo(3.14) == 0)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(int(42)) == 1)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(long(42)) == 2)
      << "incorrect overload selected from OverloadSet";
  }

  {
    auto foo = Dune::orderedOverload(
            [](double /*i*/) { return 0; },
            [](int /*i*/) { return 1; },
            [](long /*i*/) { return 2; });

    test.check(foo(3.14) == 0)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(int(42)) == 0)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(long(42)) == 0)
      << "incorrect overload selected from OverloadSet";
  }

  {
    auto foo = Dune::overload(
            [](const int& /*i*/) { return 0; },
            [](int&& /*i*/) { return 1; });

    int i = 0;
    test.check(foo(long(42)) == 1)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(int(42)) == 1)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(i) == 0)
      << "incorrect overload selected from OverloadSet";
  }

  {
    auto foo = Dune::orderedOverload(
            [](const int& /*i*/) { return 0; },
            [](int&& /*i*/) { return 1; });

    int i = 0;
    test.check(foo(long(42)) == 0)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(int(42)) == 0)
      << "incorrect overload selected from OverloadSet";
    test.check(foo(i) == 0)
      << "incorrect overload selected from OverloadSet";
  }

  {
    auto t = std::make_tuple(42, "foo", 3.14);

    auto typeToName = Dune::overload(
            [](int) { return "int"; },
            [](long) { return "long"; },
            [](std::string) { return "string"; },
            [](float) { return "float"; },
            [](double) { return "double"; });

    std::string tupleTypes;
    Dune::Hybrid::forEach(t, [&](auto&& ti) {
      tupleTypes += typeToName(ti);
    });

    test.check(tupleTypes == "intstringdouble")
      << "traversal of tuple called incorrect overloads";
  }

  {
    // Check if templated and non-templed overloads work
    // nicely together.
    auto f = Dune::overload(
        [](const int& t) { (void) t;},
        [](const auto& t) { t.bar();});
    f(0);
  }


  return test.exit();
}
