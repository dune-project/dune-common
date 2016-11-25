#include "config.h"

#include <array>

#include <dune/common/rangeutilities.hh>
#include <dune/common/iteratorrange.hh>
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

  return suite.exit();

}
