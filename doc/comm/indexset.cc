// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <config.h>

#include <dune/common/parallel/indexset.hh>
#include <dune/common/parallel/plocalindex.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <iostream>
#include "buildindexset.hh"
#include "reverse.hh"

int main(int argc, char **argv)
{
  // This is a parallel program so we need to
  // initialize mpi first.
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);

  // The rank of our process
  int rank = helper.rank();

  // The type used as the global index
  typedef int GlobalIndex;

  // The index set we use to identify the local indices with the globally
  // unique ones
  typedef Dune::ParallelIndexSet<GlobalIndex,LocalIndex,100> ParallelIndexSet;

  // The index set
  ParallelIndexSet indexSet;

  build(helper, indexSet);

  // Print the index set
  std::cout<<indexSet<<std::endl;


  reverseLocalIndex(indexSet);

  // Print the index set
  if(rank==0)
    std::cout<<"Reordered local indices:"<<std::endl;

  // Wait for all processes
  helper.getCommunication().barrier();

  std::cout<<indexSet<<std::endl;
  // Assign new local indices

  return 0;
}
