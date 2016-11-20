#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkSimdType<unsigned short    >();
    template void UnitTest::checkSimdType<unsigned          >();
    template void UnitTest::checkSimdType<unsigned long     >();
    template void UnitTest::checkSimdType<unsigned long long>();

  } // namespace Simd
} // namespace Dune
