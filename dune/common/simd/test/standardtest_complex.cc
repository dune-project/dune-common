#include <config.h>

#include <complex>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      template bool checkVector<std::complex<float      >>();
      template bool checkVector<std::complex<double     >>();
      template bool checkVector<std::complex<long double>>();

    } // namespace Test
  } // namespace Simd
} // namespace Dune
