// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ARRAY_HH
#define DUNE_ARRAY_HH

/** \file
    \brief Fallback implementation of the std::array class (a static array)
 */

#include <array>
#include <iostream>
#include <type_traits>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  // pull in default implementation
  using std::array;

  //! Output operator for array
  template < class T, size_t N >
  inline std::ostream& operator<< (std::ostream& s, const std::array<T,N>& e)
  {
    if (N == 0)
    {
      s << "[]";
      return s;
    }

    s << "[";
    for (size_t i=0; i<N-1; i++) s << e[i] << ",";
    s << e[N-1] << "]";
    return s;
  }

  //! Create and initialize an array
  /**
   * \note This method is a somewhat limited dune-specific version of
   *       make_array() as proposed for C++17 (see <a
   *       href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4391.html">N4391</a>,
   *       accepted <a
   *       href="https://botondballo.wordpress.com/2015/06/05/trip-report-c-standards-meeting-in-lenexa-may-2015/">May
   *       2015</a>).  The differeces are that this version should never be
   *       used with expliclitly given template arguments, or with
   *       std::reference_wrapper<...> arguments, and we do not give a
   *       diagnostic when anyone happens to do that.
   */
  template < typename... Args >
  std::array<typename std::common_type<Args...>::type, sizeof...(Args)> make_array(const Args&... args)
  {
    std::array<typename std::common_type<Args...>::type, sizeof...(Args)> result = { {args...} };
    return result;
  }

  //! Create an array and fill it with copies of the provided value.
  /**
   * \note This method is Dune-specific and not part of any C++ standard.
   */
  template<typename T, std::size_t n>
  std::array<T,n> fill_array(const T& t)
  {
    std::array<T,n> r;
    r.fill(t);
    return r;
  }

  /** @} */

} // end namespace Dune

#endif
