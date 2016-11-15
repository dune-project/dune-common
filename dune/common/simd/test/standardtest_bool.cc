#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      template bool checkVector<bool              >();

    } // namespace Test
  } // namespace Simd
} // namespace Dune
