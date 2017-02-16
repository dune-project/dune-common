#include <config.h>

#include <dune/common/simd/test/vctest.hh>

namespace Dune {
  namespace Simd {

    template
    void UnitTest::checkMask<Vc::SimdMaskArray<int,                4> >();

  } // namespace Simd
} // namespace Dune
