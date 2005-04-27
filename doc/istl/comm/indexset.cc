// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#include <dune/istl/indexset.hh>
#include <dune/istl/plocalindex.hh>
#include <iostream>
#include "mpi.h"
#include "buildindexset.hh"
#include "reverse.hh"

int main(int argc, char **argv)
{
  // This is a parallel programm so we need to
  // initialize mpi first.
  MPI_Init(&argc, &argv);

  // The number of processes
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // The rank of our process
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // The type used as the local index
  typedef Dune::ParallelLocalIndex<Flag> LocalIndex;

  // The type used as the global index
  typedef int GlobalIndex;

  // The index set we use to identify the local indices with the globally
  // unique ones
  typedef Dune::IndexSet<GlobalIndex,LocalIndex,100> IndexSet;

  // The index set
  IndexSet indexSet;

  build(indexSet);

  // Print the index set
  std::cout<<indexSet<<std::endl;


  reverseLocalIndex(indexSet);

  // Print the index set
  if(rank==0)
    std::cout<<"Reordered lcoal indices:"<<std::endl;

  MPI_Barrier(MPI_COMM_WORLD);

  std::cout<<indexSet<<std::endl;
  // Assign new local indices

  // Let MPI do a cleanup
  MPI_Finalize();

  return 0;
}
