// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_PRECISION_HH__
#define __DUNE_PRECISION_HH__

#include <stdlib.h>

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  class ISTLPrecision { // uses standard malloc and free
  public:
    //! return threshold to do pivoting
    static double pivoting_limit ()
    {
      return _pivoting;
    }

    //! set pivoting threshold
    static void set_pivoting_limit (double pivthres)
    {
      _pivoting = pivthres;
    }

    //! return threshold to declare matrix singular
    static double singular_limit ()
    {
      return _singular;
    }

    //! set singular threshold
    static void set_singular_limit (double singthres)
    {
      _singular = singthres;
    }

    //! return threshold to declare matrix singular
    static double absolute_limit ()
    {
      return _absolute;
    }

    //! set singular threshold
    static void set_absolute_limit (double absthres)
    {
      _absolute = absthres;
    }

  private:
    // just to demonstrate some state information
    static double _pivoting;
    static double _singular;
    static double _absolute;
  };

  double ISTLPrecision::_pivoting = 1E-8;
  double ISTLPrecision::_singular = 1E-14;
  double ISTLPrecision::_absolute = 1E-80;


  /** @} end documentation */

} // end namespace

#endif
