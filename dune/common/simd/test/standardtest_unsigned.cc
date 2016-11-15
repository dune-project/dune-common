#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      template bool checkVector<unsigned short    >();
      template bool checkVector<unsigned          >();
      template bool checkVector<unsigned long     >();
      template bool checkVector<unsigned long long>();

    } // namespace Test
  } // namespace Simd
} // namespace Dune
