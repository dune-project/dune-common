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
#include <utility>

#include <dune/common/indices.hh>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  //! Return an array filled with the provided value.
  /**
   * \tparam n     Size of the returned array.
   * \tparam T     Value type of the returned array.  This is usually deduced
   *               from `t`.
   */
  template<std::size_t n, class T>
  constexpr std::array<T, n> filledArray(const T& t)
  {
    return unpackIntegerSequence([&](auto... i) {
      return std::array<T, n>{((void)(i),t)...};
    }, std::make_index_sequence<n>{});
  }

  /** @} */

} // end namespace Dune

#endif // DUNE_COMMON_FILLED_ARRAY_HH
