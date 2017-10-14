#ifndef DUNE_COMMON_SIMD_TEST_FAKEVECTORTEST_HH
#define DUNE_COMMON_SIMD_TEST_FAKEVECTORTEST_HH

//#include <complex>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/simdfakevector.hh>

namespace Dune {
  namespace Simd {

/**
    extern template void UnitTest::checkVector<simdfakevector<char              ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<unsigned char     ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<signed char       ,5>>();
*/

    extern template void UnitTest::checkVector<simdfakevector<short             ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<unsigned short    ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<int               ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<unsigned          ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<long              ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<unsigned long     ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<long long         ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<unsigned long long,5>>();

/**
    extern template void UnitTest::checkVector<simdfakevector<wchar_t           ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<char16_t          ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<char32_t          ,5>>();
*/

    extern template void UnitTest::checkVector<simdfakevector<bool              ,5>>();

    extern template void UnitTest::checkVector<simdfakevector<float             ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<double            ,5>>();
    extern template void UnitTest::checkVector<simdfakevector<long double       ,5>>();

/**
    extern template void UnitTest::checkVector<simdfakevector<std::complex<float      >,5>>();
    extern template void UnitTest::checkVector<simdfakevector<std::complex<double     >,5>>();
    extern template void UnitTest::checkVector<simdfakevector<std::complex<long double>,5>>();
*/
//    extern template void UnitTest::checkMask<bool              ,5>>();

  } //namespace Simd
} // namespace Dune

#endif
