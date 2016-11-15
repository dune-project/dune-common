#include <config.h>

#include <dune/common/simd/test.hh>
#include <dune/common/simd/test/standardtest.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      template bool checkVector<wchar_t           >();
      template bool checkVector<char16_t          >();
      template bool checkVector<char32_t          >();

    } // namespace Test
  } // namespace Simd
} // namespace Dune
