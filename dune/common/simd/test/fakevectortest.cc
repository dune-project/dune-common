#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <complex>
#include <cstdlib>

#include <dune/common/simd/simdfakevector.hh>
#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/fakevectortest.hh>

int main()
{
  Dune::Simd::UnitTest test;

/**
  test.checkVector<simdfakevector<char              ,5>>();
  test.checkVector<simdfakevector<unsigned char     ,5>>();
  test.checkVector<simdfakevector<signed char       ,5>>();
*/

  test.checkVector<Dune::simdfakevector<short             ,5>>();
  test.checkVector<Dune::simdfakevector<unsigned short    ,5>>();
  test.checkVector<Dune::simdfakevector<int               ,5>>();
  test.checkVector<Dune::simdfakevector<unsigned          ,5>>();
  test.checkVector<Dune::simdfakevector<long              ,5>>();
  test.checkVector<Dune::simdfakevector<unsigned long     ,5>>();
  test.checkVector<Dune::simdfakevector<long long         ,5>>();
  test.checkVector<Dune::simdfakevector<unsigned long long,5>>();

/**
  test.checkVector<Dune::simdfakevector<wchar_t           ,5>>();
  test.checkVector<Dune::simdfakevector<char16_t          ,5>>();
  test.checkVector<Dune::simdfakevector<char32_t          ,5>>();
*/

  test.checkVector<Dune::simdfakevector<bool              ,5>>();

  test.checkVector<Dune::simdfakevector<float             ,5>>();
  test.checkVector<Dune::simdfakevector<double            ,5>>();
  test.checkVector<Dune::simdfakevector<long double       ,5>>();

/**
  test.checkVector<Dune::simdfakevector<std::complex<float      >,5>>();
  test.checkVector<Dune::simdfakevector<std::complex<double     >,5>>();
  test.checkVector<Dune::simdfakevector<std::complex<long double>,5>>();
*/

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
