#include "config.h"

#include <type_traits>

#include <dune/common/function.hh>
#include <dune/common/test/testsuite.hh>

int main()
{
  Dune::TestSuite t;

  {
    auto f = Dune::makeVirtualFunction<int, long>(
      [](int x) -> long { return x*x; });

    static_assert(
      std::is_base_of< Dune::VirtualFunction<int, long>, decltype(f) >::value,
      "makeVirtualFunction() must return type derived from VirtualFunction");

    long y;
    f.evaluate(2, y);
    t.check(y == 4);
  }

  {
    auto f1 = [](int x) -> long { return x*x; };
    auto f = Dune::makeVirtualFunction<int, long>(f1);

    static_assert(
      std::is_base_of< Dune::VirtualFunction<int, long>, decltype(f) >::value,
      "makeVirtualFunction() must return type derived from VirtualFunction");

    long y;
    f.evaluate(2, y);
    t.check(y == 4);
  }

  return t.exit();
}
