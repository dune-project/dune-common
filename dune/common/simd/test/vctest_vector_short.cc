#include <config.h>

#include <dune/common/simd/test/vctest.hh>

namespace Dune {
  namespace Simd {

    template
    void UnitTest::checkVector<Vc::Vector<short             > >();

  } // namespace Simd
} // namespace Dune
