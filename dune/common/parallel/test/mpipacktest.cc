// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpipack.hh>
#include <dune/common/test/testsuite.hh>

constexpr int TAG = 42;
int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);
  Dune::TestSuite suite;
  suite.require(helper.size() == 2) << "This test must be executed on two processes";
  auto comm = helper.getCommunication();
  Dune::MPIPack pack(comm);

  if(helper.rank() == 0){
    pack << 3 << helper.rank();
    pack << std::vector<int>{4711, 42};
    comm.send(pack, 1, TAG);
  }
  if(helper.rank() == 1){
    Dune::MPIPack pack = comm.rrecv(Dune::MPIPack(comm), 0, TAG);
    int drei; pack >> drei;
    int rank_0; pack >> rank_0;
    std::vector<int> vec;
    pack >> vec;
    suite.check(drei==3) << "received wrong value";
    suite.check(rank_0==0) << "received wrong value";
    suite.check(vec.size() == 2) << "vector has wrong size!";
    suite.check(vec[0] == 4711 && vec[1] == 42) << "vector contains wrong values!";
  }

  return 0;
}
