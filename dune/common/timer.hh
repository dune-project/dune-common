// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_TIMER_HH
#define DUNE_TIMER_HH

#include <chrono>

namespace Dune {

  /** @addtogroup Common
     @{
   */

  /*! \file
      \brief A simple timing class.
   */


  /** \brief A simple stop watch

     This class reports the elapsed real time, i.e. time elapsed
     after Timer::reset(). It does not measure the time spent computing,
     i.e. time spend in concurrent threads is not added up while
     time measurements include the time elapsed while sleeping.

     The class is basically a wrapper around std::chrono::high_resolution_clock::now().
   */
  class Timer
  {
    using Clock = std::chrono::high_resolution_clock;
    using Units = std::chrono::duration<double>; // seconds stored as double
  public:

    /** \brief A new timer, create and reset
     *
     * \param startImmediately If true (default) the timer starts counting immediately
     */
    Timer (bool startImmediately=true) noexcept
    {
      isRunning_ = startImmediately;
      reset();
    }

    //! Reset timer while keeping the running/stopped state
    void reset() noexcept
    {
      sumElapsed_ = std::chrono::seconds{0};
      storedLastElapsed_ = std::chrono::seconds{0};
      cstart = Clock::now();
    }


    //! Start the timer and continue measurement if it is not running. Otherwise do nothing.
    void start() noexcept
    {
      if (not (isRunning_))
      {
        cstart = Clock::now();
        isRunning_ = true;
      }
    }


    //! Get elapsed user-time from last reset until now/last stop in seconds.
    double elapsed () const noexcept
    {
      return durationCast(rawElapsed());
    }

    //! Get elapsed user-time from last start until now/last stop in seconds.
    double lastElapsed () const noexcept
    {
      return durationCast(rawLastElapsed());
    }

    //! Stop the timer and return elapsed().
    double stop() noexcept
    {
      if (isRunning_)
      {
        // update storedLastElapsed_ and  sumElapsed_ and stop timer
        storedLastElapsed_ = rawLastElapsed();
        sumElapsed_ += storedLastElapsed_;
        isRunning_ = false;
      }
      return elapsed();
    }


  private:

    bool isRunning_;
    Clock::duration sumElapsed_;
    Clock::duration storedLastElapsed_;

    Clock::duration rawElapsed () const noexcept
    {
      // if timer is running add the time elapsed since last start to sum
      if (isRunning_)
        return sumElapsed_ + rawLastElapsed();

      return sumElapsed_;
    }

    //! Get elapsed user-time from last start until now/last stop in seconds.
    Clock::duration rawLastElapsed () const noexcept
    {
      // if timer is running return the current value
      if (isRunning_)
        return Clock::now() - cstart;

      // if timer is not running return stored value from last run
      return storedLastElapsed_;
    }

    double durationCast(Clock::duration duration) const noexcept {
      return std::chrono::duration_cast<Units>(duration).count();
    }

    Clock::time_point cstart;
  }; // end class Timer

  /** @} end documentation */

} // end namespace

#endif
