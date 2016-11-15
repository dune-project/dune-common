#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      template bool checkVector<char              >();
      template bool checkVector<unsigned char     >();
      template bool checkVector<signed char       >();

    } // namespace Test
  } // namespace Simd
} // namespace Dune
