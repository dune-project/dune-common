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
      sumElapsed_ = 0.0;
      storedLastElapsed_ = 0.0;
      rawReset();
    }


    //! Start the timer and continue measurement if it is not running. Otherwise do nothing.
    void start() noexcept
    {
      if (not (isRunning_))
      {
        rawReset();
        isRunning_ = true;
      }
    }


    //! Get elapsed user-time from last reset until now/last stop in seconds.
    double elapsed () const noexcept
    {
      // if timer is running add the time elapsed since last start to sum
      if (isRunning_)
        return sumElapsed_ + lastElapsed();

      return sumElapsed_;
    }


    //! Get elapsed user-time from last start until now/last stop in seconds.
    double lastElapsed () const noexcept
    {
      // if timer is running return the current value
      if (isRunning_)
        return rawElapsed();

      // if timer is not running return stored value from last run
      return storedLastElapsed_;
    }


    //! Stop the timer and return elapsed().
    double stop() noexcept
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


    void rawReset() noexcept
    {
      cstart = std::chrono::high_resolution_clock::now();
    }

    double rawElapsed () const noexcept
    {
      std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double> >(now - cstart);
      return time_span.count();
    }

    std::chrono::high_resolution_clock::time_point cstart;
  }; // end class Timer

  /** @} end documentation */

} // end namespace

#endif
