#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <complex>
#include <cstdlib>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

int main()
{
  Dune::Simd::UnitTest test;

  test.checkSimdType<char              >();
  test.checkSimdType<unsigned char     >();
  test.checkSimdType<signed char       >();

  test.checkSimdType<short             >();
  test.checkSimdType<unsigned short    >();
  test.checkSimdType<int               >();
  test.checkSimdType<unsigned          >();
  test.checkSimdType<long              >();
  test.checkSimdType<unsigned long     >();
  test.checkSimdType<long long         >();
  test.checkSimdType<unsigned long long>();

  test.checkSimdType<wchar_t           >();
  test.checkSimdType<char16_t          >();
  test.checkSimdType<char32_t          >();

  test.checkSimdType<bool              >();

  test.checkSimdType<float             >();
  test.checkSimdType<double            >();
  test.checkSimdType<long double       >();

  test.checkSimdType<std::complex<float      >>();
  test.checkSimdType<std::complex<double     >>();
  test.checkSimdType<std::complex<long double>>();

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
