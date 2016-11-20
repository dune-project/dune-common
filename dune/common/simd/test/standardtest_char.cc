#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkSimdType<char              >();
    template void UnitTest::checkSimdType<unsigned char     >();
    template void UnitTest::checkSimdType<signed char       >();

  } // namespace Simd
} // namespace Dune
