#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <complex>
#include <cstdlib>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

int main()
{
  using Dune::Simd::Test::checkVector;

  bool good = true;

  good &= checkVector<char              >();
  good &= checkVector<unsigned char     >();
  good &= checkVector<signed char       >();

  good &= checkVector<short             >();
  good &= checkVector<unsigned short    >();
  good &= checkVector<int               >();
  good &= checkVector<unsigned          >();
  good &= checkVector<long              >();
  good &= checkVector<unsigned long     >();
  good &= checkVector<long long         >();
  good &= checkVector<unsigned long long>();

  good &= checkVector<wchar_t           >();
  good &= checkVector<char16_t          >();
  good &= checkVector<char32_t          >();

  good &= checkVector<bool              >();

  good &= checkVector<float             >();
  good &= checkVector<double            >();
  good &= checkVector<long double       >();

  good &= checkVector<std::complex<float      >>();
  good &= checkVector<std::complex<double     >>();
  good &= checkVector<std::complex<long double>>();

  return good ? EXIT_SUCCESS : EXIT_FAILURE;
}
