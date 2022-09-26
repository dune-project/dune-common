// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_POWER_HH
#define DUNE_COMMON_POWER_HH

#warning The header power.hh is deprecated. Use power from math.hh instead.

/** \file
    \brief Various implementations of the power function for run-time and static arguments
 */

#include <dune/common/math.hh>

namespace Dune {

  /** @addtogroup Common

       @{
   */

  /** \brief Calculates b^p at compile time
   * \deprecated Please use the method `power` from `math.hh` instead!
   */
  template <int b, int p>
  struct StaticPower
  {
    /** \brief power stores b^p */
    static constexpr int power = Dune::power(b,p);
  };


  /** \brief Compute power for a run-time base and a compile-time integer exponent
   *
   * \deprecated Please use the method `power` from `math.hh` instead!
   *
   * \tparam p The exponent
   */
  template <int p>
  struct Power
  {
    template <typename T>
    static constexpr auto eval(const T & a)
    {
      return power(a,p);
    }
  };

}

#endif
