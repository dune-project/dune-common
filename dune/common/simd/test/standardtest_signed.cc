#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      template bool checkVector<short             >();
      template bool checkVector<int               >();
      template bool checkVector<long              >();
      template bool checkVector<long long         >();

    } // namespace Test
  } // namespace Simd
} // namespace Dune
