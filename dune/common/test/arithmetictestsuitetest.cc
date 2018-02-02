#include <config.h>

#include <tuple>

#include <dune/common/hybridutilities.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/test/arithmetictestsuite.hh>
#if HAVE_QUADMATH
#include <dune/common/quadmath.hh>
#endif

int main(int argc, char **argv)
{
  Dune::MPIHelper::instance(argc, argv);

  Dune::ArithmeticTestSuite test;

  using ArithmeticTypes = std::tuple<
    bool,
    char, signed char, unsigned char,
    short, unsigned short,
    int, unsigned,
    long, long unsigned,
    long long, long long unsigned,
    wchar_t, char16_t, char32_t,
    float, double, long double
#if HAVE_QUADMATH
    , __float128
#endif
    >;

  Dune::Hybrid::forEach(ArithmeticTypes(), [&](auto val) {
      using T = decltype(val);
      test.checkArithmetic<T, T>();
    });

  return test.exit();
}
