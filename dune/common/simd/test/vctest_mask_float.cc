#include <config.h>

#include <dune/common/simd/test/vctest.hh>

namespace Dune {
  namespace Simd {

    template
    void UnitTest::checkMask<Vc::Mask<float             > >();

  } // namespace Simd
} // namespace Dune
