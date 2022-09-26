// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include <dune/common/streamoperators.hh>

using Dune::operator>>;
using Dune::operator<<;

int main()
{
  typedef std::tuple<int, int, int> Tuple;

  {
    const Tuple t{1, 2, 3};
    const std::string expected = "[1,2,3]";

    std::ostringstream out;
    out << t;

    if( out.str() != expected )
      return 1;
  }

  {
    const std::string data = "1 2 3";
    const Tuple expected{1, 2, 3};

    std::istringstream in(data);
    Tuple t;
    in >> t;

    if( t != expected )
      return 1;
  }

  return 0;
}
