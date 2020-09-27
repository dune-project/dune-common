// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_POWER_HH
#define DUNE_COMMON_POWER_HH

/** \file
    \brief Various implementations of the power function for run-time and static arguments
 */

#include <dune/common/math.hh>

namespace Dune {

  /** @addtogroup Common

       @{
   */

  /** \brief Calculates m^p at compile time
   * \deprecated Please use the method `power` from `math.hh` instead!
   */
  template <int m, int p>
  struct StaticPower
  {
    /** \brief power stores m^p */
    static constexpr int power = Dune::power(m,p);
  };


  /** \brief Compute power for a run-time mantissa and a compile-time integer exponent
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
