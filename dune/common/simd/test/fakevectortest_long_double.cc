#include <config.h>

#include <dune/common/simd/test/fakevectortest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkVector<simdfakevector<long double,             5>>();

//    template void UnitTest::checkMask<simdfakevector<long double,             5>>();

  } //namespace Dune
} // namespace Dune
