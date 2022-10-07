// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <utility>
#include <sstream>

#include <dune/common/std/apply.hh>

#include <dune/common/test/testsuite.hh>


template<class T>
void nop(std::initializer_list<T>&&)
{}


int main()
{
  auto test_args = std::make_tuple(true, 2, 3, "abc");

  Dune::TestSuite test;

  auto concat = [](auto&&... args) {
            bool first = true;
            std::stringstream stream;
            nop({(stream << (first ? "":",") << args, first = false)...});
            return stream.str();
          };

  test.check(Dune::Std::apply(concat, test_args) == "1,2,3,abc") << "Dune::Std::apply failed with concat lambda";

  auto makeTuple = [](auto&&... args) {
            return std::make_tuple(args...);
          };

  test.check(Dune::Std::apply(makeTuple, test_args) == test_args) << "Dune::Std::apply failed with makeTuple lambda";

  auto intTuple = std::make_tuple(1,2,3);
  auto&& intTuple0 = Dune::Std::apply([](auto&& arg0, auto&&... /*args*/) -> decltype(auto) { return arg0; }, intTuple);
  intTuple0 = 42;

  test.check(std::get<0>(intTuple) == intTuple0) << "Dune::Std::apply does not properly return references";

  // transformTuple implemented using Std::apply
  auto transformTuple = [](auto&& t, auto&& f) {
    return Dune::Std::apply([&](auto&&... args) {
      return std::make_tuple((f(std::forward<decltype(args)>(args)))...);
    }, t);
  };

  auto t1 = std::make_tuple(1, 0.2);
  auto t2 = transformTuple(t1, [](auto&& x) { return 1.0/x; });

  test.check(t2 == std::make_tuple(1.0, 5.0)) << "transformTuple implementation based on Dune::Std::apply fails";

  return test.exit();
}
