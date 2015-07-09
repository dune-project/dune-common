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
   * \note This method is Dune-specific and not part of any C++-standard.
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
    return std::move(r);
  }

  /** @} */

} // end namespace Dune

#endif
