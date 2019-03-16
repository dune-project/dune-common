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

  //! Calculates m^p at compile time
  template <int m, int p>
  struct StaticPower
  {
    /** \brief power stores m^p */
    enum { power = Dune::power(m,p) };
  };


  /** \brief Compute power for a run-time mantissa and a compile-time integer exponent
   *
   * Does some magic to create efficient code.  Not benchmarked AFAIK.
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
