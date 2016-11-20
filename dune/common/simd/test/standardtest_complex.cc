#include <config.h>

#include <complex>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {

    template void UnitTest::checkSimdType<std::complex<float      >>();
    template void UnitTest::checkSimdType<std::complex<double     >>();
    template void UnitTest::checkSimdType<std::complex<long double>>();

  } // namespace Simd
} // namespace Dune
