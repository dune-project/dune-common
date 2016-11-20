#ifndef DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH
#define DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH

#include <complex>

#include <dune/common/simd/test.hh>

namespace Dune {
  namespace Simd {

    extern template void UnitTest::checkSimdType<char              >();
    extern template void UnitTest::checkSimdType<unsigned char     >();
    extern template void UnitTest::checkSimdType<signed char       >();

    extern template void UnitTest::checkSimdType<short             >();
    extern template void UnitTest::checkSimdType<unsigned short    >();
    extern template void UnitTest::checkSimdType<int               >();
    extern template void UnitTest::checkSimdType<unsigned          >();
    extern template void UnitTest::checkSimdType<long              >();
    extern template void UnitTest::checkSimdType<unsigned long     >();
    extern template void UnitTest::checkSimdType<long long         >();
    extern template void UnitTest::checkSimdType<unsigned long long>();

    extern template void UnitTest::checkSimdType<wchar_t           >();
    extern template void UnitTest::checkSimdType<char16_t          >();
    extern template void UnitTest::checkSimdType<char32_t          >();

    extern template void UnitTest::checkSimdType<bool              >();

    extern template void UnitTest::checkSimdType<float             >();
    extern template void UnitTest::checkSimdType<double            >();
    extern template void UnitTest::checkSimdType<long double       >();

    extern template void UnitTest::checkSimdType<std::complex<float      >>();
    extern template void UnitTest::checkSimdType<std::complex<double     >>();
    extern template void UnitTest::checkSimdType<std::complex<long double>>();

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH
