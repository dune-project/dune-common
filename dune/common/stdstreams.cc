// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stdstreams.hh"

namespace Dune {

  /*

     The standard debug streams declared in stdstreams.hh exist in this
     file so that they can be compiled into libdune

   */

  /* stream for very verbose output: information on the lowest
     level. This is expected to report insane amounts of
     information. Use of the activation-flag to only generate output
     near the problem is recommended */
  DVVerbType dvverb(std::cout);

  /* stream for verbose output: information that helps to trace in
     more detail what the modules do */
  DVerbType dverb(std::cout);

  /* stream for informative output: summary infos on what a module
     does, runtimes, etc. */
  DInfoType dinfo(std::cout);

  /* stream for warnings: messages which may indicate problems */
  DWarnType dwarn(std::cerr);

  /* stream for strong warnings: when a failure */
  DGraveType dgrave(std::cerr);

  /* stream for error messages: only packages integrating Dune
     completely will redirect it. The output of derr is independent of
     the debug-level, only the activation-flag is checked */
  DErrType derr(std::cerr);

}
