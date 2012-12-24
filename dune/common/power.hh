// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_POWER_HH
#define DUNE_COMMON_POWER_HH

/** \file
    \brief Various implementations of the power function for run-time and static arguments
 */

namespace Dune {

  /** @addtogroup Common

       @{
   */

  //! Calculates m^p at compile time
  template <int m, int p>
  struct StaticPower
  {
    /** \brief power stores m^p */
    enum { power = (m * StaticPower<m,p-1>::power ) };
  };

  //! end of recursion via specialization
  template <int m>
  struct StaticPower< m , 0>
  {
    /** \brief m^0 = 1 */
    enum { power = 1 };
  };


   #ifndef DOXYGEN
  template <int p, bool odd = p%2>
  struct PowerImp {};
#endif

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
    static T eval(const T & a)
    {
      return PowerImp<p>::eval(a);
    }
  };

#ifndef DOXYGEN
  template <int p>
  struct PowerImp<p,false>
  {
    template <typename T>
    static T eval(const T & a)
    {
      T t = Power<p/2>::eval(a);
      return t*t;
    }
  };

  template <int p>
  struct PowerImp<p,true>
  {
    template <typename T>
    static T eval(const T & a)
    {
      return a*Power<p-1>::eval(a);;
    }
  };

  template <>
  struct PowerImp<1,true>
  {
    template <typename T>
    static T eval(const T & a)
    {
      return a;
    }
  };
#endif

}

#endif
