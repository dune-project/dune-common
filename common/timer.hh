// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_TIMER_HH__
#define __DUNE_TIMER_HH__

//#include<math.h>
//#include<complex>
//#include<iostream>
//#include<iomanip>
//#include<string>
#include <sys/times.h>            // for timing measurements
#include <asm/param.h>

/*! \file
    A simple timing class.
 */

namespace Dune {

  /** @addtogroup ISTL
     @{
   */

  //! a simple stop watch
  //! using the C command 'times' defined in sys/times.h
  class Timer
  {
  public:
    //! a new timer, start immediately
    Timer ()
    {
      reset();
    }

    //! reset timer
    void reset ()
    {
      struct tms buf;
      times(&buf);
      cstart = buf.tms_utime + buf.tms_stime;
    }

    //! get elapsed user+sys time in seconds
    double elapsed ()
    {
      struct tms buf;
      times(&buf);
      clock_t cend = buf.tms_utime + buf.tms_stime;
      return difftime(cend,cstart)/HZ;
    }

  private:
    clock_t cstart;
  }; // end class Timer

  /** @} end documentation */

} // end namespace

#endif
