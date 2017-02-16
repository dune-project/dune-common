#include <config.h>

#include <dune/common/simd/test/vctest.hh>

namespace Dune {
  namespace Simd {

    template
    void UnitTest::checkVector<Vc::Vector<int               > >();

  } // namespace Simd
} // namespace Dune
