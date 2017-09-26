#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/fakevectortest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkVector<simdfakevector<bool,             5>>();

    //template void UnitTest::checkMask<simdfakevector<bool              >>();
  }
} // namespace Dune
