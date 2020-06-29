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

  std::cout << "test static factorial\n{";

  forEach(integralRange(_k), [&](auto _i) {
    auto value = Dune::factorial(_i);

    t.check(decltype(value)::value == Dune::Factorial<decltype(_i)::value>::factorial);

    std::cout<< ' ' << value() << ',';
  });

  std::cout << "};\n\n";

  return t;
}

template<class T, T k>
auto testStaticBinomial (std::integral_constant<T, k> _k = {}) -> TestSuite
{
  TestSuite t;

  std::cout << "test static binomial\n";

  forEach(integralRange(_k), [&](auto _i) {
    std::cout << "{";
    forEach(integralRange(next(_i)), [&](auto _j) {
      const auto value = Dune::binomial(_i,_j);

      auto control = Dune::Factorial<decltype(_i)::value>::factorial
                     / Dune::Factorial<decltype(_j)::value>::factorial
                     / Dune::Factorial<decltype(_i)::value - decltype(_j)::value>::factorial;
      t.check(decltype(value)::value == control);

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
