// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_TIMER_HH__
#define __DUNE_TIMER_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>
#include <sys/times.h>            // for timing measurements


/*! \file

    A simple timing class.
 */

namespace Dune {

  /** @addtogroup ISTL
          @{
   */


  //! a simple stop watch
  class Timer
  {
  public:
    //! a new timer, start immediately
    Timer ()
    {
      struct tms buf;
      cstart = times(&buf);
    }

    //! reset timer
    void reset ()
    {
      struct tms buf;
      cstart = times(&buf);
    }

    //! get elapsed time in seconds
    double elapsed ()
    {
      struct tms buf;
      cend = times(&buf);
      return ((double)(cend-cstart))/100.0;
    }

  private:
    clock_t cstart,cend;
  };


  /** @} end documentation */

} // end namespace

#endif
