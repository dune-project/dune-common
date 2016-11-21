#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkSimdType<char32_t          >();

  } // namespace Simd
} // namespace Dune
