// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TIMER_HH
#define DUNE_TIMER_HH

// headers for getrusage(2)
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

// headers for stderror(3)
#include <string.h>

// access to errno in C++
#include <cerrno>

#include "exceptions.hh"


/*! \file
    A simple timing class.
 */

namespace Dune {

  class TimerError : public SystemError {} ;

  /** @addtogroup common
     @{
   */

  //! a simple stop watch
  //! using the C command getrusage
  class Timer
  {
  public:
    //! a new timer, start immediately
    Timer () throw(TimerError)
    {
      reset();
    }

    //! reset timer
    void reset() throw (TimerError)
    {
      rusage ru;
      if (getrusage(RUSAGE_SELF, &ru))
        DUNE_THROW(TimerError, strerror(errno));
      cstart = ru.ru_utime;
    }

    //! get elapsed user+sys time in seconds
    double elapsed () const throw (TimerError)
    {
      rusage ru;
      if (getrusage(RUSAGE_SELF, &ru))
        DUNE_THROW(TimerError, strerror(errno));
      return 1.0 * (ru.ru_utime.tv_sec - cstart.tv_sec) + (ru.ru_utime.tv_usec - cstart.tv_usec) / (1000.0 * 1000.0);
    }

  private:
    struct timeval cstart;
  }; // end class Timer

  /** @} end documentation */

} // end namespace

#endif
