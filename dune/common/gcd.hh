// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GCD_HH
#define DUNE_GCD_HH

namespace Dune
{
  /**
   * @addtogroup Common
   * @{
   */
  /**
   * @file
   * \brief Statically compute the greatest common divisor of two integers
   */

#ifndef DOXYGEN
  /**
   * @brief Helper for calculating the gcd.
   */
  template<long a, long b, bool bo>
  struct GcdHelper
  {};


  template<long a, long b>
  struct GcdHelper<a,b,true>
  {
    /**
     * @brief Check that a>b.
     */
    static void conceptCheck()
    {
      static_assert(b<a, "b<a must hold!");
      static_assert(0<b, "b must be positive");
    }


    /**
     * @brief The greatest common divisor of the numbers a and b.
     */
    const static long gcd = GcdHelper<b,a%b,true>::gcd;
  };

  template<long a, long b>
  struct GcdHelper<a,b,false>
  {
    /**
     * @brief The greatest common divisor of the numbers a and b.
     */
    const static long gcd = GcdHelper<b,a,true>::gcd;
  };
  template<long a>
  struct GcdHelper<a,0,true>
  {
    const static long gcd=a;
  };

#endif

  /**
   * @brief Calculator of the greatest common divisor.
   */
  template<long a, long b>
  struct Gcd
  {
    /**
     * @brief The greatest common divisior of a and b. */
    const static long value = GcdHelper<a,b,(a>b)>::gcd;
  };

  /**
   * @}
   */
}

#endif
