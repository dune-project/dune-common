// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#include <iostream>

#include <dune/common/hybridutilities.hh>
#include <dune/common/indices.hh>
#include <dune/common/math.hh>
#include <dune/common/test/testsuite.hh>

#include <dune/common/math.hh>


using namespace Dune::Hybrid;
using namespace Dune::Indices;
using Dune::TestSuite;

template<class T, T n>
constexpr inline static auto next(std::integral_constant<T, n>)
  -> std::integral_constant<T, n+1>
{
  return {};
}

template<class T, T k>
auto testStaticFactorial (std::integral_constant<T, k> _k = {}) -> TestSuite
{
  TestSuite t;

  std::cout << "test factorial\n{";

  forEach(integralRange(_k), [&](auto _i) {

    const auto value = Dune::factorial(_i);
    const auto control = _i() == 0 ? 1 : _i() * Dune::factorial(_i() - 1);

    t.check( value() == control );

    std::cout<< ' ' << value() << ',';
  });

  std::cout << "};\n\n";

  return t;
}

template<class T, T k>
auto testStaticBinomial (std::integral_constant<T, k> _k = {}) -> TestSuite
{
  TestSuite t;

  std::cout << "test binomial\n";

  forEach(integralRange(_k), [&](auto _i) {
    std::cout << "{";
    forEach(integralRange(next(_i)), [&](auto _j) {

      const auto value = Dune::binomial(_i, _j);
      const auto control = Dune::factorial(_i) / ( Dune::factorial(_j) * Dune::factorial(_i() - _j()) );

      t.check( value() == control );

      std::cout<< ' ' << value() << ',';
    });

    std::cout << "};\n";
  });

  std::cout << "\n";

  return t;
}

int main(int argc, char** argv)
{
  TestSuite t;

  t.subTest(testStaticFactorial(_5));
  t.subTest(testStaticBinomial(_5));

  return t.exit();
}
