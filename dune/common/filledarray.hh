// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_FILLED_ARRAY_HH
#define DUNE_COMMON_FILLED_ARRAY_HH

/** \file
    \brief Utility to generate an array with a certain value
 */

#include <array>
#include <cstddef>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  //! Return an array filled with the provided value.
  /**
   * \note This function is `constexpr` only in C++17, or, more precisely,
   *       when `std::array::begin()` and `std::array::end()` are `constexpr`.
   *
   * \tparam n     Size of the returned array.
   * \tparam T     Value type of the returned array.  This is usually deduced
   *               from `t`.
   * \tparam Array Helper template parameter to make specifying `constexpr`
   *               well-defined under all circumstances.  Do not provide an
   *               argument for this parameter or otherwise rely on it, it may
   *               vanish without notice.
   */
  template<std::size_t n, class T, class Array = std::array<T, n> >
  constexpr auto filledArray(const T& t)
  {
    Array arr{};
    // this is constexpr in c++17, `arr.fill(t)` is not
    for(auto &el : arr)
      el = t;
    return arr;
  }

  /** @} */

} // end namespace Dune

#endif // DUNE_COMMON_FILLED_ARRAY_HH
