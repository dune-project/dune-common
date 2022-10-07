// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/typetraits.hh>
#include <dune/common/test/testsuite.hh>

int main()
{
  Dune::TestSuite test;

  {
    auto f = [](int /*i*/) { return 0; };
    using F = decltype(f);

    test.check(Dune::IsCallable<F(int)>() == true)
      << "Dune::IsCallable does not accept copy from r-value";
    test.check(Dune::IsCallable<F(int&)>() == true)
      << "Dune::IsCallable does not accept copy from l-value reference";
    test.check(Dune::IsCallable<F(int&&)>() == true)
      << "Dune::IsCallable does not accept copy from r-value reference";

    test.check(Dune::IsCallable<F(std::string)>() == false)
      << "Dune::IsCallable accepts invalid argument type";
    test.check(Dune::IsCallable<F(int, int)>() == false)
      << "Dune::IsCallable accepts invalid argument count";
    test.check(Dune::IsCallable<F(int), int>() == true)
      << "Dune::IsCallable does not accept valid return type";
    test.check(Dune::IsCallable<F(int), std::string>() == false)
      << "Dune::IsCallable accepts invalid return type";
  }

  {
    auto f = [](const int& /*i*/) {};
    using F = decltype(f);

    test.check(Dune::IsCallable<F(int)>() == true)
      << "Dune::IsCallable does not accept const& temporary from r-value";
    test.check(Dune::IsCallable<F(int&)>() == true)
      << "Dune::IsCallable does not accept const& temporary from l-value reference";
    test.check(Dune::IsCallable<F(int&&)>() == true)
      << "Dune::IsCallable does not accept const& temporary from r-value reference";
  }

  {
    auto f = [](int& /*i*/) {};
    using F = decltype(f);

    test.check(Dune::IsCallable<F(int)>() == false)
      << "Dune::IsCallable accepts l-value reference from r-value";
    test.check(Dune::IsCallable<F(int&)>() == true)
      << "Dune::IsCallable does not accept l-value reference from l-value reference";
    test.check(Dune::IsCallable<F(int&&)>() == false)
      << "Dune::IsCallable accepts l-value reference from r-value reference";
  }

  {
    auto f = [](int&& /*i*/) {};
    using F = decltype(f);

    test.check(Dune::IsCallable<F(int)>() == true)
      << "Dune::IsCallable does not accept r-value reference from r-value";
    test.check(Dune::IsCallable<F(int&)>() == false)
      << "Dune::IsCallable accepts r-value reference from l-value reference";
    test.check(Dune::IsCallable<F(int&&)>() == true)
      << "Dune::IsCallable does not accept r-value reference from r-value reference";
  }

  return test.exit();
}
