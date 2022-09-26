// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>
#include <vector>

#include <dune/common/hybridutilities.hh>
#include <dune/common/tuplevector.hh>
#include <dune/common/test/testsuite.hh>



template<class C>
auto incrementAll(C&& c)
{
  using namespace Dune::Hybrid;
  forEach(c, [](auto&& ci) {
    ++ci;
  });
}

template<class C>
auto addIndex(C&& c)
{
  using namespace Dune::Hybrid;
  forEach(integralRange(Dune::Hybrid::size(c)), [&](auto&& i) {
    c[i] += i;
  });
}

template<class C>
auto incAndAppendToFirst(C&& c)
{
  using namespace Dune::Hybrid;
  forEach(integralRange(Dune::Hybrid::size(c)), [&](auto&& i) {
    using namespace Dune::Hybrid;
    using namespace Dune::Indices;
    ifElse(equals(i, _0), [&](auto id) {
      id(c[i]).append("+1");
    }, [&](auto id) {
      ++id(c[i]);
    });
  });
}

template<class C>
constexpr auto sum(C&& c)
{
  using namespace Dune::Hybrid;
  using namespace Dune::Indices;
  return accumulate(c, 0.0, [](auto&& a, auto&& b) {
    return a+b;
  });
}

template<class C, class I>
auto sumSubsequence(C&& c, I&& indices)
{
  using namespace Dune::Hybrid;
  double result = 0;
  forEach(indices, [&](auto i) {
    result += Dune::Hybrid::elementAt(c, i);
  });
  return result;
}



int main()
{
  auto vector = std::vector<int>{1, 2, 3};
  auto numberTuple = Dune::makeTupleVector(0.1, 2, 3);

  Dune::TestSuite test;

  incrementAll(vector);
  test.check(vector == std::vector<int>{2, 3, 4})
    << "Incrementing vector entries with Hybrid::forEach failed.";

  incrementAll(numberTuple);
  test.check(numberTuple == Dune::makeTupleVector(1.1, 3, 4))
    << "Incrementing tuple entries with Hybrid::forEach failed.";

  addIndex(vector);
  test.check(vector == std::vector<int>{2, 4, 6})
    << "Adding indices to vector entries with Hybrid::forEach failed.";

  addIndex(numberTuple);
  test.check(numberTuple == Dune::makeTupleVector(1.1, 4, 6))
    << "Adding indices to vector entries with Hybrid::forEach failed.";


  auto mixedTuple = Dune::makeTupleVector(std::string("1"), 2, 3);
  incAndAppendToFirst(mixedTuple);
  test.check(mixedTuple == Dune::makeTupleVector(std::string("1+1"), 3, 4))
    << "Adding indices to vector entries with Hybrid::forEach failed.";

  constexpr auto values = std::make_integer_sequence<std::size_t, 30>();
  test.check((30*29)/2 == sum(values))
    << "accumulate() yields incorrect result.";

  test.check((29*28)/2 == sumSubsequence(values, std::make_integer_sequence<std::size_t, 29>()))
    << "Summing up subsequence failed.";

  // Compile time checks
  static_assert(sum(values) == (30*29)/2, "Wrong compile time sum!");
  constexpr auto numberTupleConstexpr = Dune::makeTupleVector(0.1, 2, 3);
  static_assert(sum(numberTupleConstexpr) == 5.1, "Wrong compile time sum!");

  return test.exit();
}
