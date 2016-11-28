#ifndef DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH
#define DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH

#include <complex>

#include <dune/common/simd/test.hh>

namespace Dune {
  namespace Simd {

    extern template void UnitTest::checkVector<char              >();
    extern template void UnitTest::checkVector<unsigned char     >();
    extern template void UnitTest::checkVector<signed char       >();

    extern template void UnitTest::checkVector<short             >();
    extern template void UnitTest::checkVector<unsigned short    >();
    extern template void UnitTest::checkVector<int               >();
    extern template void UnitTest::checkVector<unsigned          >();
    extern template void UnitTest::checkVector<long              >();
    extern template void UnitTest::checkVector<unsigned long     >();
    extern template void UnitTest::checkVector<long long         >();
    extern template void UnitTest::checkVector<unsigned long long>();

    extern template void UnitTest::checkVector<wchar_t           >();
    extern template void UnitTest::checkVector<char16_t          >();
    extern template void UnitTest::checkVector<char32_t          >();

    extern template void UnitTest::checkVector<bool              >();

    extern template void UnitTest::checkVector<float             >();
    extern template void UnitTest::checkVector<double            >();
    extern template void UnitTest::checkVector<long double       >();

    extern template void UnitTest::checkVector<std::complex<float      >>();
    extern template void UnitTest::checkVector<std::complex<double     >>();
    extern template void UnitTest::checkVector<std::complex<long double>>();

    extern template void UnitTest::checkMask<bool              >();

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH
