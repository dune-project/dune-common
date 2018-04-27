#include <config.h>

#include <dune/common/simd/test/looptest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkIndex<LoopSIMD<std::size_t, 5>>();

  } //namespace Simd
} // namespace Dune
