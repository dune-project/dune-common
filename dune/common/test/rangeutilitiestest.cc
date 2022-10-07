// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#include <array>
#include <map>
#include <vector>
#include <numeric>
#include <type_traits>

#include <dune/common/hybridutilities.hh>
#include <dune/common/iteratorrange.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/diagonalmatrix.hh>
#include <dune/common/test/testsuite.hh>
#include <dune/common/test/iteratortest.hh>


template<class R>
auto checkRangeIterators(R&& r)
{
  auto it = r.begin();
  auto end = r.end();
  auto op = [](const auto& x){};
  return (testConstIterator(it, end, op)==0);
}

template<class R>
auto checkRangeSize(R&& r)
{
  std::size_t counter = 0;
  for([[maybe_unused]] auto&& dummy : r)
    ++counter;
  return (r.size()==counter);
}

template<class R, class V>
auto checkRandomAccessNumberRangeSums(R&& r, V sum, V first, V last)
{
  bool passed = true;
  passed = passed and (std::accumulate(r.begin(), r.end(), 0) == sum);
  passed = passed and (std::accumulate(r.begin()+1, r.end(), 0) == (sum-first));
  passed = passed and (std::accumulate(r.begin(), r.end()-1, 0) == (sum-last));
  passed = passed and (std::accumulate(r.begin()+1, r.end()-1, 0) == (sum-first-last));
  return passed;
}

template<class T>
struct is_const_reference : public std::conjunction<std::is_reference<T>, std::is_const<std::remove_reference_t<T>>>
{};

template<class T>
struct is_mutable_reference : public std::conjunction<std::is_reference<T>, std::negation<is_const_reference<T>>>
{};


auto testTransformedRangeView()
{
  Dune::TestSuite suite("Check transformedRangeView()");

  // Check transformedRangeView with container range
  Dune::Hybrid::forEach(std::make_tuple(std::array<int,3>({1,2,3}), std::vector<int>({1,2,3})), [&](auto&& a)
  {
    auto a_backup = a;
    // Pass original range by l-value, modify it, and then traverse.
    // This should traverse the modified original range.
    {
      auto r = Dune::transformedRangeView(a, [](auto&& x) { return 2*x;});
      a[0] = 2;
      suite.check(checkRandomAccessNumberRangeSums(r, 14, 4, 6))
        << "incorrect values in transformedRangeView of l-value";
      suite.check(checkRangeIterators(r))
        << "iterator test fails for transformedRangeView of l-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of l-value";
      a = a_backup;
    }
    // Pass original range by const l-value, modify it, and then traverse.
    // This should traverse the modified original range.
    {
      const auto& a_const = a;
      auto r = Dune::transformedRangeView(a_const, [](auto&& x) { return 2*x;});
      a[0] = 2;
      suite.check(checkRandomAccessNumberRangeSums(r, 14, 4, 6))
        << "incorrect values in transformedRangeView of const l-value";
      suite.check(checkRangeIterators(r))
        << "iterator test fails for transformedRangeView of const l-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of const l-value";
      a = a_backup;
    }
    // Modify original range, pass it by r-value, restore it, and then traverse.
    // This should traverse a modified copy of the range and not the restored
    // original one.
    {
      a[0] = 2;
      auto r = Dune::transformedRangeView(std::move(a), [](auto&& x) { return 2*x;});
      a = a_backup;
      suite.check(checkRandomAccessNumberRangeSums(r, 14, 4, 6))
        << "incorrect values in transformedRangeView of r-value";
      suite.check(checkRangeIterators(r))
        << "iterator test fails for transformedRangeView of r-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of r-value";
    }
    // Check if returning real references in the transformation works
    {
      auto r = Dune::transformedRangeView(a, [](auto&& x) -> decltype(auto) { return x;});
      suite.check(is_mutable_reference<decltype(*r.begin())>::value)
        << "iterator with mutable-reference returning transformation does not return mutable references";
      suite.check(&(*(r.begin())) == &(a[0]))
        << "reference points to wrong location";
      (*r.begin()) = 0;
      suite.check(a[0] == 0)
        << "modifying range by reference returning transformation failed";
      a = a_backup;
    }
    // Check if returning real references in the transformation works
    {
      const auto& a_const = a;
      auto r = Dune::transformedRangeView(a_const, [](auto&& x) -> decltype(auto) { return x;});
      suite.check(is_const_reference<decltype(*r.begin())>::value)
        << "iterator with const-reference returning transformation does not return const references";
      suite.check(&(*(r.begin())) == &(a[0]))
        << "reference points to wrong location";
    }
    // Check iterator based transformation
    {
      auto r = Dune::iteratorTransformedRangeView(a, [&](auto&& it) { return (*it)+(it-a.begin());});
      suite.check(checkRandomAccessNumberRangeSums(r, 9, 1, 5))
        << "incorrect values in transformedRangeView of l-value";
      suite.check(checkRangeIterators(r))
        << "iterator test fails for transformedRangeView of l-value";
      suite.check(checkRangeSize(r))
        << "checking size fails for transformedRangeView of l-value";
      a = a_backup;
    }
  });
  // Check transformedRangeView with on the fly range
  {
    auto r = Dune::transformedRangeView(Dune::range(10), [](auto&& x) { return 2*x;});
    suite.check(checkRandomAccessNumberRangeSums(r, 90, 0, 18))
      << "transformation of on-the-fly range gives incorrect results";
    suite.check(checkRangeIterators(r))
      << "iterator test fails for transformedRangeView";
    suite.check(checkRangeSize(r))
      << "checking size fails for transformedRangeView of on-the-fly range";
  }
  // Check if we can indirectly sort subrange via reference returning transformations
  {
    auto a = std::vector<int>{4,3,2,1,0};
    auto r = Dune::transformedRangeView(Dune::range(1,4), [&](auto&& i) -> decltype(auto){ return a[i];});
    std::sort(r.begin(), r.end());
    suite.check(a == std::vector<int>{4,1,2,3,0})
      << "sorting reference returning transformedRangeView failed";

    auto r2 = Dune::transformedRangeView(std::array<int,3>{0, 2, 4}, [&](auto&& i) -> decltype(auto){ return a[i];});
    std::sort(r2.begin(), r2.end());
    suite.check(a == std::vector<int>{0,1,2,3,4})
      << "sorting reference returning transformedRangeView failed";

    // Remap values of certain keys in a std::map such that
    // they are sorted according to the keys.
    auto m = std::map<int, int>{{-1,5},{0,4}, {1,3}, {2,2}};
    auto r3 = Dune::transformedRangeView(std::array<int,3>{1, -1, 2}, [&](auto&& i) -> decltype(auto){ return m[i];});
    std::sort(r3.begin(), r3.end());
    suite.check(m == std::map<int, int>{{1,2},{-1,3}, {2,5},{0,4}})
      << "sorting reference returning transformedRangeView failed";
  }
  return suite;
}


auto testSparseRange()
{
  Dune::TestSuite suite("Check sparseRange()");

  auto checkWithMatrix = [&suite](auto&& M) {
    for(std::size_t i=0; i<M.size(); ++i)
    {
      auto it = M[i].begin();
      auto end = M[i].end();
      for(auto&& [M_ij, j] : Dune::sparseRange(M[i]))
      {
        suite.check(it!=end)
          << "sparseRange() contains more entries than the original range";
        suite.check(&M_ij == &M[i][j])
          << "Entry obtained by sparseRange() does not point to actual range entry";
        suite.check(&M_ij == &(*it))
          << "Entry obtained by sparseRange() does not point to actual range entry";
        ++it;
      }
      suite.check(it==end)
        << "sparseRange() contains less entries than the original range";
    }
  };

  auto M1 = Dune::DiagonalMatrix<double,1>({42});
  checkWithMatrix(M1);
  checkWithMatrix(std::as_const(M1));

  auto M2 = Dune::DiagonalMatrix<double,2>({42, 41});
  checkWithMatrix(M2);
  checkWithMatrix(std::as_const(M2));

  auto M3 = Dune::DiagonalMatrix<double,3>({42, 41, 40});
  checkWithMatrix(M3);
  checkWithMatrix(std::as_const(M3));

  return suite;
}



int main()
{
  // Check IsIterable<> for https://gitlab.dune-project.org/core/dune-common/issues/58
  static_assert(Dune::IsIterable< std::array<int, 3> >::value, "std::array<int> must be a range");
  static_assert(Dune::IsIterable< Dune::IteratorRange<int*> >::value, "IteratorRange must be a range");
  static_assert(!Dune::IsIterable< int >::value, "int must not be a range");

  Dune::TestSuite suite;

  // max_value, min_value
  {
    const int value = 12;
    suite.check(Dune::max_value(value) == value);
    suite.check(Dune::min_value(value) == value);

    std::array<int, 3> values{-42, 0, 42};
    suite.check(Dune::max_value(values) == 42)
      << "maximum of values is 42, but got " << Dune::max_value(values);
    suite.check(Dune::min_value(values) == -42)
      << "minimum of values is -42, but got " << Dune::min_value(values);

    std::array<int, 3> positiveValues{1, 2, 3};
    suite.check(Dune::max_value(positiveValues) == 3)
      << "maximum of positiveValues is 3, but got " << Dune::max_value(positiveValues);
    suite.check(Dune::min_value(positiveValues) == 1)
      << "minimum of positiveValues is 1, but got " << Dune::min_value(positiveValues);

    std::array<int, 3> negativeValues{-1, -3, -1};
    suite.check(Dune::max_value(negativeValues) == -1)
      << "maximum of negativeValues is -1, but got " << Dune::max_value(negativeValues);
    suite.check(Dune::min_value(negativeValues) == -3)
      << "minimum of negativeValues is -3, but got " << Dune::min_value(negativeValues);
  }

  // any_true, all_true
  {
    const std::array<bool, 3> allTrue{true, true, true};
    const std::array<bool, 3> allFalse{false, false, false};
    const std::array<bool, 3> someTrue{false, true, false};

    suite.check(Dune::any_true(allTrue))
      << "any_true(allTrue) must be true";
    suite.check(!Dune::any_true(allFalse))
      << "any_true(allFalse) must be false";
    suite.check(Dune::any_true(someTrue))
      << "any_true(someTrue) must be true";

    suite.check(Dune::all_true(allTrue))
      << "all_true(allTrue) must be true";
    suite.check(!Dune::all_true(allFalse))
      << "all_true(allFalse) must be false";
    suite.check(!Dune::all_true(someTrue))
      << "all_true(someTrue) must be false";

    const bool t = true;
    const bool f = false;

    suite.check(Dune::any_true(t))
      << "any_true(true) must be true";
    suite.check(!Dune::any_true(f))
      << "any_true(false) must be false";

    suite.check(Dune::all_true(t))
      << "all_true(true) must be true";
    suite.check(!Dune::all_true(f))
      << "all_true(false) must be false";
  }

  // integer ranges
  using Dune::range;
  std::vector<int> numbers(range(6).begin(), range(6).end());
  int sum = 0;
  for( auto i : range(numbers.size()) )
    sum += numbers[i];
  suite.check(sum == 15) << "sum over range( 0, 6 ) must be 15.";
  suite.check(range(sum, 100)[5] == 20) << "range(sum, 100)[5] must be 20.";
  sum = 0;
  for( auto i : range(-10, 11) )
    sum += i;
  suite.check(sum == 0) << "sum over range( -10, 11 ) must be 0.";

  static_assert(std::is_same<decltype(range(std::integral_constant<int, 4>()))::integer_sequence, std::make_integer_sequence<int, 4>>::value,
                "decltype(range(std::integral_constant<int, 4>))::integer_sequence must be the same as std::make_integer_sequence<int, 4>");

  // Hybrid::forEach for integer ranges
  Dune::Hybrid::forEach(range(std::integral_constant<int, 1>()), [] (auto &&i) {
      static_assert(std::is_same<std::decay_t<decltype(i)>, std::integral_constant<int, 0>>::value,
                    "Hybrid::forEach(range(std::integral_constant<int, 1>()), ...) should only visit std::integral_constant<int, 0>.");
    });


  {
    auto r = range(-10,11);
    auto it = r.begin();
    auto end = r.end();
    auto op = [](const auto& x){};
    suite.check(testConstIterator(it, end, op)==0)
      << "iterator test fails for range(-10,11)";
  }

  suite.subTest(testTransformedRangeView());

  suite.subTest(testSparseRange());

  return suite.exit();

}
