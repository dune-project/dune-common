// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <chrono>
#include <thread>

#include <dune/common/timer.hh>

#include <dune/common/test/testsuite.hh>

int
main()
{
  Dune::TestSuite suite;

  using clock = std::chrono::high_resolution_clock;
  using namespace std::chrono_literals;
  Dune::Timer top_timer(false);
  auto top_chrono_start = clock::now();
  std::this_thread::sleep_for(10ms);
  top_timer.start();

  Dune::Timer loop_timer;
  for (std::size_t i = 0; i != 5; ++i) {
    { // let chrono timer run for longer time
      auto chrono_start = clock::now();
      loop_timer.start();
      std::this_thread::sleep_for(10ms);
      loop_timer.stop();
      double dune_elapsed = loop_timer.lastElapsed();
      std::this_thread::sleep_for(10ms);
      auto chrono_elapsed = clock::now() - chrono_start;

      std::this_thread::sleep_for(10ms);
      suite.check(dune_elapsed == loop_timer.lastElapsed())
        << "Elapsed time is not the same as when timer is stop";
      suite.check(dune_elapsed < (chrono_elapsed / 1.0s))
        << "Dune timer " << dune_elapsed << "s takes more time than chrono timer "
        << (chrono_elapsed / 1.0s) << "s";
    }

    { // let dune timer run for longer time
      loop_timer.start();
      auto chrono_start = clock::now();
      std::this_thread::sleep_for(10ms);
      auto chrono_elapsed = clock::now() - chrono_start;
      std::this_thread::sleep_for(10ms);
      loop_timer.stop();
      double dune_elapsed = loop_timer.lastElapsed();

      std::this_thread::sleep_for(10ms);
      suite.check(dune_elapsed == loop_timer.lastElapsed())
        << "Elapsed time is not the same as when timer is stop";
      suite.check(dune_elapsed > (chrono_elapsed / 1.0s))
        << "Dune timer " << dune_elapsed << "s takes less time than chrono timer "
        << (chrono_elapsed / 1.0s) << "s";
    }
  }

  double top_dune_elapsed = top_timer.stop();
  std::this_thread::sleep_for(10ms);
  auto top_chrono_elapsed = clock::now() - top_chrono_start;

  suite.check(top_dune_elapsed < (top_chrono_elapsed / 1.0s))
    << "Top Dune timer " << top_dune_elapsed << "s takes less time than Top chrono timer "
    << (top_chrono_elapsed / 1.0s) << "s";

  suite.check(top_dune_elapsed > loop_timer.elapsed())
    << "Top Dune timer " << top_dune_elapsed << "s takes less time than Loop Dune timer "
    << loop_timer.elapsed() << "s";
}
