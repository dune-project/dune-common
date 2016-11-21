#include <config.h>

#include <complex>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkSimdType<std::complex<long double>>();

  } // namespace Simd
} // namespace Dune
