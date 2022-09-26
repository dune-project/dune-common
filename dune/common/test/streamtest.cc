// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

/*

   Test to check if the standard streams in libdune can be properly
   linked with this program and if they work

 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <fstream>

#include <dune/common/stdstreams.hh>

// enums are a nice special case (was a bug)
enum check { VALUE = 5 };

int main () {
  try {
    // let output happen but vanish
    std::ofstream dummy("/dev/null");
    Dune::derr.attach(dummy);

    Dune::derr.push(true);
    Dune::derr << "Teststring" << std::endl;

    Dune::derr << VALUE << std::endl;
    Dune::dverb << VALUE << std::endl;
    Dune::dvverb << VALUE << std::endl;
    Dune::dinfo << VALUE << std::endl;
    Dune::dwarn << VALUE << std::endl;
    Dune::dgrave << VALUE << std::endl;

    // instantiate private stream and connect global stream
    {
      Dune::DebugStream<> mystream(dummy);
      Dune::derr.tie(mystream);
      Dune::derr << "Blah" << std::endl;
      // untie before mystream gets destructed
      Dune::derr.untie();
    }

    Dune::derr << "Still working" << std::endl;
  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 2;
  } catch (...) {
    return 1;
  };

  return 0;
}
