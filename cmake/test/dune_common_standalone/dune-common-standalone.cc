// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/fvector.hh>
#include <dune/common/parametertree.hh>

int main() {
  Dune::MPIHelper::instance();

  Dune::FieldVector<double,2> lower_left(0.);
  Dune::FieldVector<double,2> upper_right(-1.);

  Dune::ParameterTree ptree;
  ptree["foo"] = "bar";

  return 0;
}
