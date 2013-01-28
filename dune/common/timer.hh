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

     \warning In a multi-threading situation, this class does NOT return wall-time!
     Instead, the run time for all threads will be added up.
     For example, if you have four threads running in parallel taking one second each,
     then the Timer class will return an elapsed time of four seconds.

   */
  class Timer
  {
  public:

    /** \brief A new timer, create and reset
     *
     * \param startImmediately If true (default) the timer starts counting immediately
     */
    Timer (bool startImmediately=true) throw(TimerError)
    {
      isRunning_ = startImmediately;
      reset();
    }

    //! Reset timer while keeping the running/stopped state
    void reset() throw (TimerError)
    {
      sumElapsed_ = 0.0;
      storedLastElapsed_ = 0.0;
      rawReset();
    }


    //! Start the timer and continue measurement if it is not running. Otherwise do nothing.
    void start() throw (TimerError)
    {
      if (not (isRunning_))
      {
        rawReset();
        isRunning_ = true;
      }
    }


    //! Get elapsed user-time from last reset until now/last stop in seconds.
    double elapsed () const throw (TimerError)
    {
      // if timer is running add the time elapsed since last start to sum
      if (isRunning_)
        return sumElapsed_ + lastElapsed();

      return sumElapsed_;
    }


    //! Get elapsed user-time from last start until now/last stop in seconds.
    double lastElapsed () const throw (TimerError)
    {
      // if timer is running return the current value
      if (isRunning_)
        return rawElapsed();

      // if timer is not running return stored value from last run
      return storedLastElapsed_;
    }


    //! Stop the timer and return elapsed().
    double stop() throw (TimerError)
    {
      if (isRunning_)
      {
        // update storedLastElapsed_ and  sumElapsed_ and stop timer
        storedLastElapsed_ = lastElapsed();
        sumElapsed_ += storedLastElapsed_;
        isRunning_ = false;
      }
      return elapsed();
    }


  private:

    bool isRunning_;
    double sumElapsed_;
    double storedLastElapsed_;


#ifdef TIMER_USE_STD_CLOCK
    void rawReset() throw (TimerError)
    {
      cstart = std::clock();
    }

    double rawElapsed () const throw (TimerError)
    {
      return (std::clock()-cstart) / static_cast<double>(CLOCKS_PER_SEC);
    }

    std::clock_t cstart;
#else
    void rawReset() throw (TimerError)
    {
      rusage ru;
      if (getrusage(RUSAGE_SELF, &ru))
        DUNE_THROW(TimerError, strerror(errno));
      cstart = ru.ru_utime;
    }

    double rawElapsed () const throw (TimerError)
    {
      rusage ru;
      if (getrusage(RUSAGE_SELF, &ru))
        DUNE_THROW(TimerError, strerror(errno));
      return 1.0 * (ru.ru_utime.tv_sec - cstart.tv_sec) + (ru.ru_utime.tv_usec - cstart.tv_usec) / (1000.0 * 1000.0);
    }

    struct timeval cstart;
#endif
  }; // end class Timer

  /** @} end documentation */

} // end namespace

#endif
