// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
   */
  template<std::size_t n, class T>
  constexpr std::array<T, n> filledArray(const T& t)
  {
    std::array<T, n> arr{};
    // this is constexpr in c++17, `arr.fill(t)` is not
    for(auto &el : arr)
      el = t;
    return arr;
  }

  /** @} */

} // end namespace Dune

#endif // DUNE_COMMON_FILLED_ARRAY_HH
