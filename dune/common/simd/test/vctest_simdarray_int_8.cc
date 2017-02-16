#include <config.h>

#include <dune/common/simd/test/vctest.hh>

namespace Dune {
  namespace Simd {

    template
    void UnitTest::checkVector<Vc::SimdArray<int,                8> >();

  } // namespace Simd
} // namespace Dune
