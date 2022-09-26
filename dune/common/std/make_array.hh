// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_MAKE_ARRAY_HH
#define DUNE_COMMON_STD_MAKE_ARRAY_HH

#include <array>
#include <type_traits>

#if DUNE_HAVE_CXX_EXPERIMENTAL_MAKE_ARRAY
#include <experimental/array>
#endif

#warning make_array.hh is deprecated and will be removed after Dune 2.9. \
  Use deduction guide of `std::array` or `std::to_array`.

namespace Dune {
namespace Std {

#if DUNE_HAVE_CXX_EXPERIMENTAL_MAKE_ARRAY

  /// \deprecated Use deduction guide of `std::array` or `std::to_array`.
  using std::experimental::make_array;

#else // DUNE_HAVE_CXX_EXPERIMENTAL_MAKE_ARRAY

  //! Create and initialize an array
  /**
   * \note This method is a somewhat limited dune-specific version of
   *       make_array() as proposed for C++17 (see <a
   *       href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4391.html">N4391</a>,
   *       accepted <a
   *       href="https://botondballo.wordpress.com/2015/06/05/trip-report-c-standards-meeting-in-lenexa-may-2015/">May
   *       2015</a>). The differences are that this version should
   *       never be used with explicitly given template arguments, or
   *       with std::reference_wrapper<...> arguments, and we do not
   *       give a diagnostic when anyone happens to do that.
   *
   * \ingroup CxxUtilities
   * \deprecated Use deduction guide of `std::array` or `std::to_array`.
   */
  template <typename... Args>
  std::array<typename std::common_type<Args...>::type, sizeof...(Args)>
  make_array(const Args&... args) {
    std::array<typename std::common_type<Args...>::type, sizeof...(Args)>
        result = {{args...}};
    return result;
  }

#endif // DUNE_HAVE_CXX_EXPERIMENTAL_MAKE_ARRAY

}
}

#endif
