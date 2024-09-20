// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
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
    ifElse(equal_to(i, _0), [&](auto id) {
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

// Test switchCases
template <class StaticContainer, class DynamicContainer, class Cases>
void testSwitchCases(Dune::TestSuite& test,
                     const StaticContainer& staticContainer,  //< The container to access
                     const DynamicContainer& dynContainer,    //< A storage of reference values
                     const Cases& indexRange)                 //< The range of possible indices
{
  using namespace Dune::Indices;
  assert(staticContainer.size() == dynContainer.size());
  assert(staticContainer.size() < 8);

  // Try to set foundValue to entry of staticContainer. Because the latter is a tuple,
  // we let Hybrid::switchCases do a dynamic-to-static dispatch. This requires
  // to pass a range of indices to check and optionally a default branch to be
  // used if no case matches.

  // First check with default branch and return value
  auto getContainerValue = [&](auto index) {
    return staticContainer[index];
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
    test.check(foundValue == dynContainer[2])
      << "Hybrid::switchCases with matching index and with default failed.";
  }

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, 0, getContainerValue, getDefaultValue);
    test.check(foundValue == dynContainer[0])
      << "Hybrid::switchCases with matching index and with default failed.";
  }

  // Now check without default branch. Since this does not allow to determine a return
  // value, it only implements a void case and thus modifies an external variable.

  int foundValue;

  auto setFoundValue = [&](auto index) {
    foundValue = getContainerValue(index);
  };

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, 2, setFoundValue);
  test.check(foundValue == dynContainer[2])
    << "Hybrid::switchCases with matching index and without default failed.";

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, 0, setFoundValue);
  test.check(foundValue == dynContainer[0])
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
    test.check(foundValue == dynContainer[2])
      << "Hybrid::switchCases with matching integral_constant index and with default failed.";
  }

  {
    auto foundValue = Dune::Hybrid::switchCases(indexRange, _0, getContainerValue, getDefaultValue);
    test.check(foundValue == dynContainer[0])
      << "Hybrid::switchCases with matching integral_constant index and with default failed.";
  }

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, _2, setFoundValue);
  test.check(foundValue == dynContainer[2])
    << "Hybrid::switchCases with matching integral_constant index and without default failed.";

  foundValue= -1;
  Dune::Hybrid::switchCases(indexRange, _0, setFoundValue);
  test.check(foundValue == dynContainer[0])
    << "Hybrid::switchCases with matching integral_constant index and without default failed.";
}


int main()
{
  std::size_t one = 1; // run-time value
  auto vector = std::vector<int>{1, 2, 3};
  auto fieldVector = Dune::FieldVector<double,3>({1, 2, 3});
  auto numberTuple = Dune::makeTupleVector(0.1, 2, 3);

  Dune::TestSuite test;

  using namespace Dune::Indices;
  static_assert(Dune::Hybrid::equal_to(_1, _1));
  static_assert(not Dune::Hybrid::equal_to(_1, _2));

  static_assert(Dune::Hybrid::equal_to(1, _1));
  test.check(Dune::Hybrid::equal_to(one, one)) << "Runtime Hybrid::equal_to failed.";

  static_assert(Dune::Hybrid::equal_to(_3, Dune::Hybrid::max(_1,_2,_3)));
  test.check(Dune::Hybrid::equal_to(3, Dune::Hybrid::max(one,_2,_3)))
    << "Runtime Hybrid::max failed.";

  static_assert(Dune::Hybrid::equal_to(_1, Dune::Hybrid::min(_1,_2,_3)));
  test.check(Dune::Hybrid::equal_to(one, Dune::Hybrid::min(one,_2,_3)))
    << "Runtime Hybrid::min failed.";

  static_assert(Dune::Hybrid::equal_to(_4, Dune::Hybrid::plus(_1,_3)));
  test.check(Dune::Hybrid::equal_to(4, Dune::Hybrid::plus(one,_3)))
    << "Runtime Hybrid::plus failed.";

  static_assert(Dune::Hybrid::equal_to(_2, Dune::Hybrid::minus(_3,_1)));
  test.check(Dune::Hybrid::equal_to(2, Dune::Hybrid::minus(_3,one)))
    << "Runtime Hybrid::minus failed.";

  incrementAll(vector);
  test.check(vector == std::vector<int>{2, 3, 4})
    << "Incrementing vector entries with Hybrid::forEach failed.";

  incrementAll(numberTuple);
  test.check(numberTuple == Dune::makeTupleVector(1.1, 3, 4))
    << "Incrementing tuple entries with Hybrid::forEach failed.";

  incrementAll(fieldVector);
  test.check(fieldVector == Dune::FieldVector<double,3>({2, 3, 4}))
    << "Incrementing FieldVector entries with Hybrid::forEach failed.";

  addIndex(vector);
  test.check(vector == std::vector<int>{2, 4, 6})
    << "Adding indices to vector entries with Hybrid::forEach failed.";

  addIndex(numberTuple);
  test.check(numberTuple == Dune::makeTupleVector(1.1, 4, 6))
    << "Adding indices to vector entries with Hybrid::forEach failed.";
  test.check(Dune::IsIntegralConstant<decltype(Dune::Hybrid::size(numberTuple))>::value)
    << "Hybrid::size() of std::tuple is not an std::integral_constant.";

  addIndex(fieldVector);
  test.check(fieldVector == Dune::FieldVector<double,3>({2, 4, 6}))
    << "Adding indices to FieldVector entries with Hybrid::forEach failed.";
  test.check(Dune::IsIntegralConstant<decltype(Dune::Hybrid::size(fieldVector))>::value)
    << "Hybrid::size() of FieldVector is not an std::integral_constant.";

  auto mixedTuple = Dune::makeTupleVector(std::string("1"), 2, 3);
  incAndAppendToFirst(mixedTuple);
  test.check(mixedTuple == Dune::makeTupleVector(std::string("1+1"), 3, 4))
    << "Adding indices to vector entries with Hybrid::forEach failed.";

  constexpr auto values = std::make_integer_sequence<std::size_t, 30>();
  test.check((30*29)/2 == sum(values))
    << "accumulate() yields incorrect result.";

  test.check((29*28)/2 == sumSubsequence(values, std::make_integer_sequence<std::size_t, 29>()))
    << "Summing up subsequence failed.";


  { // check switchCases utility

    // the reference values for comparison
    auto a = std::array{40,41,42,43};

    // check a static container, cases given as index_sequence
    testSwitchCases(test, Dune::makeTupleVector(40,41,42,43), a, std::make_index_sequence<4>());

    // check a static container, cases given as StaticIntegralRange
    testSwitchCases(test, Dune::makeTupleVector(40,41,42,43), a, Dune::StaticIntegralRange<std::size_t,4>());

    // check a dynamic container, cases given as IntegralRange
    testSwitchCases(test, a, a, Dune::IntegralRange<std::size_t>(4));

    // check a dynamic container, cases given as StaticIntegralRange
    testSwitchCases(test, a, a, Dune::StaticIntegralRange<std::size_t,4>());
  }


  // Compile time checks
  static_assert(sum(values) == (30*29)/2, "Wrong compile time sum!");
  constexpr auto numberTupleConstexpr = Dune::makeTupleVector(0.25, 2, 3);
  static_assert(sum(numberTupleConstexpr) == 5.25, "Wrong compile time sum!");

  { // CTAD tests

    auto tv0 = Dune::TupleVector{};

    // test construction with values
    static_assert(std::is_same_v<decltype(tv0),Dune::TupleVector<>>);
    auto tv1 = Dune::TupleVector{1,2.0,3.0f,4u};
    static_assert(std::is_same_v<decltype(tv1),Dune::TupleVector<int,double,float,unsigned int>>);
    auto tv2 = Dune::TupleVector{std::tuple{1,2.0,3.0f,4u}};
    static_assert(std::is_same_v<decltype(tv2),Dune::TupleVector<int,double,float,unsigned int>>);
    auto tv3 = Dune::TupleVector{std::pair{1,2.0}};
    static_assert(std::is_same_v<decltype(tv3),Dune::TupleVector<int,double>>);

    // test construction with l-values
    Dune::FieldVector<double,2> arg1{};
    std::vector<float> arg2{};
    auto tv4 = Dune::TupleVector{arg1,arg2};
    static_assert(std::is_same_v<decltype(tv4),Dune::TupleVector<Dune::FieldVector<double,2>,std::vector<float>>>);

    // test construction with allocators
    auto tv5 = Dune::TupleVector{std::allocator_arg_t{}, std::allocator<float>{}, arg1, arg2};
    static_assert(std::is_same_v<decltype(tv4),decltype(tv5)>);
    auto tv6 = Dune::TupleVector{std::allocator_arg_t{}, std::allocator<float>{}, std::tuple{arg1, arg2}};
    static_assert(std::is_same_v<decltype(tv4),decltype(tv6)>);
    auto tv7 = Dune::TupleVector{std::allocator_arg_t{}, std::allocator<float>{}, std::pair{arg1, arg2}};
    static_assert(std::is_same_v<decltype(tv4),decltype(tv7)>);
  }

  return test.exit();
}
