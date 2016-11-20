#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkSimdType<wchar_t           >();
    template void UnitTest::checkSimdType<char16_t          >();
    template void UnitTest::checkSimdType<char32_t          >();

  } // namespace Simd
} // namespace Dune
