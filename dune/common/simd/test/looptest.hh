#ifndef DUNE_COMMON_SIMD_TEST_LOOPTEST_HH
#define DUNE_COMMON_SIMD_TEST_LOOPTEST_HH

#include <dune/common/simd/test.hh>
#include <dune/common/simd/loop.hh>

namespace Dune {
  namespace Simd {

    extern template void UnitTest::checkVector<LoopSIMD<char              ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<unsigned char     ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<signed char       ,5>>();

    extern template void UnitTest::checkVector<LoopSIMD<short             ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<unsigned short    ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<int               ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<unsigned          ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<long              ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<unsigned long     ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<long long         ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<unsigned long long,5>>();

    extern template void UnitTest::checkVector<LoopSIMD<bool              ,5>>();

    extern template void UnitTest::checkVector<LoopSIMD<float             ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<double            ,5>>();
    extern template void UnitTest::checkVector<LoopSIMD<long double       ,5>>();

    extern template void UnitTest::checkMask<LoopSIMD<bool              ,5>>();

  } //namespace Simd
} // namespace Dune

#endif
