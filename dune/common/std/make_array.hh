#ifndef DUNE_COMMON_STD_MAKE_ARRAY_HH
#define DUNE_COMMON_STD_MAKE_ARRAY_HH

#include <array>
#include <type_traits>

#if __cpp_lib_experimental_make_array >= 201505
#include <experimental/array>
#endif

namespace Dune {
namespace Std {

#if __cpp_lib_experimental_make_array >= 201505

  using std::experimental::make_array;

#else // __cpp_lib_experimental_make_array >= 201505

  //! Create and initialize an array
  /**
   * \note This method is a somewhat limited dune-specific version of
   *       make_array() as proposed for C++17 (see <a
   *       href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4391.html">N4391</a>,
   *       accepted <a
   *       href="https://botondballo.wordpress.com/2015/06/05/trip-report-c-standards-meeting-in-lenexa-may-2015/">May
   *       2015</a>). The differences are that this version should
   *       never be used with expliclitly given template arguments, or
   *       with std::reference_wrapper<...> arguments, and we do not
   *       give a diagnostic when anyone happens to do that.
   */
  template <typename... Args>
  std::array<typename std::common_type<Args...>::type, sizeof...(Args)>
  make_array(const Args&... args) {
    std::array<typename std::common_type<Args...>::type, sizeof...(Args)>
        result = {{args...}};
    return result;
  }

#endif // __cpp_lib_experimental_make_array >= 201505

}
}

#endif
