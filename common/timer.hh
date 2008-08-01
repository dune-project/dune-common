// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TIMER_HH
#define DUNE_TIMER_HH

#ifndef TIMER_USE_STD_CLOCK
// headers for getrusage(2)
#include <sys/resource.h>
#endif

#include <ctime>

// headers for stderror(3)
#include <cstring>

// access to errno in C++
#include <cerrno>

#include "exceptions.hh"

namespace Dune {

  /** @addtogroup Common
     @{
   */

  /*! \file
      \brief A simple timing class.
   */

  /** \brief %Exception thrown by the Timer class */
  class TimerError : public SystemError {} ;


  /** \brief A simple stop watch

     This class reports the elapsed user-time, i.e. time spent computing,
     after the last call to Timer::reset(). The results are seconds and
     fractional seconds. Note that the resolution of the timing depends
     on your OS kernel which should be somewhere in the milisecond range.

     The class is basically a wrapper for the libc-function getrusage()

   */
  class Timer
  {
  public:
    //! A new timer, start immediately
    Timer () throw(TimerError)
    {
      reset();
    }

    //! Reset timer
    void reset() throw (TimerError)
    {
#ifdef TIMER_USE_STD_CLOCK
      cstart = std::clock();
#else
      rusage ru;
      if (getrusage(RUSAGE_SELF, &ru))
        DUNE_THROW(TimerError, strerror(errno));
      cstart = ru.ru_utime;
#endif
    }

    //! Get elapsed user-time in seconds
    double elapsed () const throw (TimerError)
    {
#ifdef TIMER_USE_STD_CLOCK
      return (std::clock()-cstart) / static_cast<double>(CLOCKS_PER_SEC);
#else
      rusage ru;
      if (getrusage(RUSAGE_SELF, &ru))
        DUNE_THROW(TimerError, strerror(errno));
      return 1.0 * (ru.ru_utime.tv_sec - cstart.tv_sec) + (ru.ru_utime.tv_usec - cstart.tv_usec) / (1000.0 * 1000.0);
#endif
    }

  private:
#ifdef TIMER_USE_STD_CLOCK
    std::clock_t cstart;
#else
    struct timeval cstart;
#endif
  }; // end class Timer

  /** @} end documentation */

} // end namespace

#endif
