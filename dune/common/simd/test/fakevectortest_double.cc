#include <config.h>

#include <dune/common/simd/test/fakevectortest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkVector<simdfakevector<double,             5>>();

//    template void UnitTest::checkMask<simdfakevector<double,             5>>();

  } //namespace Dune
} // namespace Dune
