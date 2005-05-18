// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LCM_HH
#define DUNE_LCM_HH

#include <dune/common/helpertemplates.hh>

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
  struct Lcm
  {
    static void conceptCheck()
    {
      IsTrue<0<m>::yes();
      IsTrue<0<n>::yes();
    }
    /**
     * @brief The least common multiple of the template parameters
     * m and n.
     */
    const static long value = (m/Gcd<m,n>::value)*n;
  };
}

#endif
