// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LCM_HH
#define DUNE_LCM_HH

#warning "This header is deprecated and will be removed after release 2.8. Use std::lcm instead."

/** \file
 * \brief Statically compute the least common multiple of two integers
 */

#include <numeric>

namespace Dune
{

  /**
   * @addtogroup Common
   * @{
   */
  /**
   * @file
   * This file provides template constructs for calculation the
   * least common multiple.
   */

  /**
   * @brief Calculate the least common multiple of two numbers
   */
  template<long m, long n>
  struct [[deprecated("Will be removed after Dune 2.8. Use std::lcm instead.")]] Lcm
  {
    static void conceptCheck()
    {
      static_assert(0<m, "m must be positive!");
      static_assert(0<n, "n must be positive!");
    }
    /**
     * @brief The least common multiple of the template parameters
     * m and n.
     */
    constexpr static long value = std::lcm(m,n);
  };
}

#endif
