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
  std::size_t one = 1; // run-time value
  auto vector = std::vector<int>{1, 2, 3};
  auto numberTuple = Dune::makeTupleVector(0.1, 2, 3);

  Dune::TestSuite test;

  using namespace Dune::Indices;
  static_assert(Dune::Hybrid::equals(_1, _1));
  static_assert(not Dune::Hybrid::equals(_1, _2));

  static_assert(Dune::Hybrid::equals(1, _1));
  test.check(Dune::Hybrid::equals(one, one)) << "Runtime Hybrid::equals failed.";

  static_assert(Dune::Hybrid::equals(_3, Dune::Hybrid::max(_1,_2,_3)));
  test.check(Dune::Hybrid::equals(3, Dune::Hybrid::max(one,_2,_3)))
    << "Runtime Hybrid::max failed.";

  static_assert(Dune::Hybrid::equals(_1, Dune::Hybrid::min(_1,_2,_3)));
  test.check(Dune::Hybrid::equals(one, Dune::Hybrid::min(one,_2,_3)))
    << "Runtime Hybrid::min failed.";

  static_assert(Dune::Hybrid::equals(_4, Dune::Hybrid::plus(_1,_3)));
  test.check(Dune::Hybrid::equals(4, Dune::Hybrid::plus(one,_3)))
    << "Runtime Hybrid::plus failed.";

  static_assert(Dune::Hybrid::equals(_2, Dune::Hybrid::minus(_3,_1)));
  test.check(Dune::Hybrid::equals(2, Dune::Hybrid::minus(_3,one)))
    << "Runtime Hybrid::minus failed.";

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


  // Test switchCases

  auto hybridContainer = std::make_tuple(40, 41, 42,43);

  auto indexRange = std::make_index_sequence<4>();

  // Try to set foundValue to entry of hybridContainer. Because the latter is a tuple,
  // we let Hybrid::switchCases do a dynamic-to-static dispatch. This requires
  // to pass a range of indices to check and optionally a default branch to be
  // used if no case matches.

  // First check with default branch and return value
  auto getContainerValue = [&](auto index) {
    return std::get<decltype(index)::value>(hybridContainer);
  };

  auto getDefaultValue = [&]() {
    return -2;
  };

  // First check with default branch and return value

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, 7, getContainerValue, getDefaultValue);
    test.check(foundValue == -2)
      << "Hybrid::switchCases with non-matching index and with default failed.";
  }

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, 2, getContainerValue, getDefaultValue);
    test.check(foundValue == 42)
      << "Hybrid::switchCases with matching index and with default failed.";
  }

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, 0, getContainerValue, getDefaultValue);
    test.check(foundValue == 40)
      << "Hybrid::switchCases with matching index and with default failed.";
  }

  // Now check without default branch. Since this does not allow to determine a return
  // value, it only implements a void case and thus modifies an external variable.

  int foundValue;

  auto setFoundValue = [&](auto index) {
    foundValue = getContainerValue(index);
  };

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, 7, setFoundValue);
  test.check(foundValue == -1)
    << "Hybrid::switchCases with non-matching index and without default failed.";

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, 2, setFoundValue);
  test.check(foundValue == 42)
    << "Hybrid::switchCases with matching index and without default failed.";

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, 0, setFoundValue);
  test.check(foundValue == 40)
    << "Hybrid::switchCases with matching index and without default failed.";

  // Now do the same checks with integral_constant index
  // Notice that we cannot return the value as std::integral_constant,
  // because this would be incompatible with the dynamic version, where
  // the return type is always deduced from the default branch.

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, _7, getContainerValue, getDefaultValue);
    test.check(foundValue == -2)
      << "Hybrid::switchCases with non-matching integral_constant index and with default failed.";
  }

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, _2, getContainerValue, getDefaultValue);
    test.check(foundValue == 42)
      << "Hybrid::switchCases with matching integral_constant index and with default failed.";
  }

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, _0, getContainerValue, getDefaultValue);
    test.check(foundValue == 40)
      << "Hybrid::switchCases with matching integral_constant index and with default failed.";
  }

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, _7, setFoundValue);
  test.check(foundValue == -1)
    << "Hybrid::switchCases with non-matching integral_constant index and without default failed.";

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, _2, setFoundValue);
  test.check(foundValue == 42)
    << "Hybrid::switchCases with matching integral_constant index and without default failed.";

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, _0, setFoundValue);
  test.check(foundValue == 40)
    << "Hybrid::switchCases with matching integral_constant index and without default failed.";

  // Compile time checks
  static_assert(sum(values) == (30*29)/2, "Wrong compile time sum!");
  constexpr auto numberTupleConstexpr = Dune::makeTupleVector(0.1, 2, 3);
  static_assert(sum(numberTupleConstexpr) == 5.1, "Wrong compile time sum!");

  return test.exit();
}
