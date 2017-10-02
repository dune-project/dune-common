#include "config.h"

#include <array>
#include <vector>

#include <dune/common/hybridutilities.hh>
#include <dune/common/iteratorrange.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/test/testsuite.hh>


int main()
{
  // Check is_range<> for https://gitlab.dune-project.org/core/dune-common/issues/58
  static_assert(Dune::is_range< std::array<int, 3> >::value, "std::array<int> must be a range");
  static_assert(Dune::is_range< Dune::IteratorRange<int*> >::value, "IteratorRange must be a range");
  static_assert(!Dune::is_range< int >::value, "int must not be a range");

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

  return suite.exit();

}
