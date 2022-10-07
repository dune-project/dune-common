// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/ios_state.hh>

namespace Dune {

  //////////////////////////////////////////////////////////////////////
  //
  //  class ios_base_all_saver
  //

  ios_base_all_saver::ios_base_all_saver(state_type& ios_)
    : ios(ios_), oldflags(ios.flags()), oldprec(ios.precision()),
      oldwidth(ios.width())
  {}

  ios_base_all_saver::~ios_base_all_saver()
  {
    restore();
  }

  void ios_base_all_saver::restore()
  {
    ios.flags(oldflags);
    ios.precision(oldprec);
    ios.width(oldwidth);
  }

}
