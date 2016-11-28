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

  test.checkVector<char              >();
  test.checkVector<unsigned char     >();
  test.checkVector<signed char       >();

  test.checkVector<short             >();
  test.checkVector<unsigned short    >();
  test.checkVector<int               >();
  test.checkVector<unsigned          >();
  test.checkVector<long              >();
  test.checkVector<unsigned long     >();
  test.checkVector<long long         >();
  test.checkVector<unsigned long long>();

  test.checkVector<wchar_t           >();
  test.checkVector<char16_t          >();
  test.checkVector<char32_t          >();

  test.checkVector<bool              >();

  test.checkVector<float             >();
  test.checkVector<double            >();
  test.checkVector<long double       >();

  test.checkVector<std::complex<float      >>();
  test.checkVector<std::complex<double     >>();
  test.checkVector<std::complex<long double>>();

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
