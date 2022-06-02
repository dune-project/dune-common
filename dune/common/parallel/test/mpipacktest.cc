// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

constexpr int TAG = 42;

#include <iostream>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpipack.hh>
#include <dune/common/test/testsuite.hh>

template<typename Comm>
auto testSync(Comm comm)
{
  Dune::TestSuite suite("testSync");

  int rank = comm.rank();
  // we receive from left and right
  int src  = (rank - 1 + comm.size()) % comm.size();
  int dest = (rank + 1) % comm.size();

  // send
  {
    Dune::MPIPack pack(comm);
    pack << 3 << comm.rank();
    pack << std::vector<int>{4711, 42};
    comm.send(pack, dest, TAG);
  }
  // recv
  {
    Dune::MPIPack pack = comm.rrecv(Dune::MPIPack(comm), src, TAG);
    int drei; pack >> drei;
    int rank_src; pack >> rank_src;
    std::vector<int> vec;
    pack >> vec;
    suite.check(drei==3) << "received wrong value";
    suite.check(rank_src==src) << "received wrong value";
    suite.check(vec.size() == 2) << "vector has wrong size!";
    suite.check(vec[0] == 4711 && vec[1] == 42) << "vector contains wrong values!";
  }
  return suite;
}

int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);
  Dune::TestSuite suite;
  auto comm = helper.getCommunication();

  suite.subTest(testSync(comm));

  return suite.exit();
}
