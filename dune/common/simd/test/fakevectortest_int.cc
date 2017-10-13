#include <config.h>

#include <dune/common/simd/test/fakevectortest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkVector<simdfakevector<int,             5>>();

//    template void UnitTest::checkMask<simdfakevector<int,             5>>();

  } //namespace Dune
} // namespace Dune
