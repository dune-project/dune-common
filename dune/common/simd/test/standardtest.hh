#ifndef DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH
#define DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH

#include <complex>

#include <dune/common/simd/test.hh>

namespace Dune {
  namespace Simd {
    namespace Test {

      extern template bool checkVector<char              >();
      extern template bool checkVector<unsigned char     >();
      extern template bool checkVector<signed char       >();

      extern template bool checkVector<short             >();
      extern template bool checkVector<unsigned short    >();
      extern template bool checkVector<int               >();
      extern template bool checkVector<unsigned          >();
      extern template bool checkVector<long              >();
      extern template bool checkVector<unsigned long     >();
      extern template bool checkVector<long long         >();
      extern template bool checkVector<unsigned long long>();

      extern template bool checkVector<wchar_t           >();
      extern template bool checkVector<char16_t          >();
      extern template bool checkVector<char32_t          >();

      extern template bool checkVector<bool              >();

      extern template bool checkVector<float             >();
      extern template bool checkVector<double            >();
      extern template bool checkVector<long double       >();

      extern template bool checkVector<std::complex<float      >>();
      extern template bool checkVector<std::complex<double     >>();
      extern template bool checkVector<std::complex<long double>>();

    } // namespace Test
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_TEST_STANDARDTEST_HH
