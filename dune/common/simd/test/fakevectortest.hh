#ifndef DUNE_COMMON_SIMD_TEST_FAKEVECTORTEST_HH
#define DUNE_COMMON_SIMD_TEST_FAKEVECTORTEST_HH

//#include <complex>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/simdfakevector.hh>

namespace Dune { namespace Simd {

  static int size = 5;
/**
  extern template void UnitTest::checkVector<simdfakevector<char              ,size>>();
  extern template void UnitTest::checkVector<simdfakevector<unsigned char     ,size>>();
  extern template void UnitTest::checkVector<simdfakevector<signed char       ,size>>();
*/

//  extern template void UnitTest::checkVector<simdfakevector<short             ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<unsigned short    ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<int               ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<unsigned          ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<long              ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<unsigned long     ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<long long         ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<unsigned long long,size>>();

/**
  extern template void UnitTest::checkVector<simdfakevector<wchar_t           ,size>>();
  extern template void UnitTest::checkVector<simdfakevector<char16_t          ,size>>();
  extern template void UnitTest::checkVector<simdfakevector<char32_t          ,size>>();
*/

  extern template void UnitTest::checkVector<simdfakevector<bool              ,5>>();

//  extern template void UnitTest::checkVector<simdfakevector<float             ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<double            ,size>>();
//  extern template void UnitTest::checkVector<simdfakevector<long double       ,size>>();

/**
  extern template void UnitTest::checkVector<simdfakevector<std::complex<float      >,size>>();
  extern template void UnitTest::checkVector<simdfakevector<std::complex<double     >,size>>();
  extern template void UnitTest::checkVector<simdfakevector<std::complex<long double>,size>>();

  extern template void UnitTest::checkMask<bool              ,size>>();
*/
  }
} // namespace Dune

#endif
