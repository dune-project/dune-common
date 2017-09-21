#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <complex>
#include <cstdlib>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/fakevectortest.hh>

int main()
{
  Dune::Simd::UnitTest test;
  static int size = 5;

/**
  test.checkVector<simdfakevector<char              ,size>>();
  test.checkVector<simdfakevector<unsigned char     ,size>>();
  test.checkVector<simdfakevector<signed char       ,size>>();
*/

//  test.checkVector<simdfakevector<short             ,size>>();
//  test.checkVector<simdfakevector<unsigned short    ,size>>();
  test.checkVector<simdfakevector<int               ,size>>();
  test.checkVector<simdfakevector<unsigned          ,size>>();
  test.checkVector<simdfakevector<long              ,size>>();
  test.checkVector<simdfakevector<unsigned long     ,size>>();
//  test.checkVector<simdfakevector<long long         ,size>>();
//  test.checkVector<simdfakevector<unsigned long long,size>>();

/**
  test.checkVector<simdfakevector<wchar_t           ,size>>();
  test.checkVector<simdfakevector<char16_t          ,size>>();
  test.checkVector<simdfakevector<char32_t          ,size>>();
*/

  test.checkVector<simdfakevector<bool              ,size>>();

//  test.checkVector<simdfakevector<float             ,size>>();
//  test.checkVector<simdfakevector<double            ,size>>();
//  test.checkVector<simdfakevector<long double       ,size>>();

/**
  test.checkVector<simdfakevector<std::complex<float      >,size>>();
  test.checkVector<simdfakevector<std::complex<double     >,size>>();
  test.checkVector<simdfakevector<std::complex<long double>,size>>();
*/

  return test.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
