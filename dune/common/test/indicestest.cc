// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tuple>

#include <dune/common/indices.hh>
#include <dune/common/tuplevector.hh>

using namespace Dune;



int main()
{
  using namespace Dune::Indices;

  // Test whether indices can be used to index a data structure
  Dune::TupleVector<int,double,float> v;
  v[_0] = 42;
  v[_1] = 3.14;
  v[_2] = 2.7;

  // Test whether the indices can be used as numbers
  std::get<_0>(v) = 43;
  std::get<_1>(v) = 4.14;
  std::get<_2>(v) = 3.7;

  return 0;
}
