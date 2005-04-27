// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#include <dune/istl/indexset.hh>
#include <dune/istl/plocalindex.hh>
#include <iostream>
#include "mpi.h"

/**
 * @brief Flag for marking the indices.
 */
enum Flag {owner, overlap};


int main(int argc, char **argv)
{
  // This is a parallel programm so we need to
  // initialize mpi first.
  MPI_Init(&argc, &argv);

  // The number of processes
  int size;

  // The rank of our process
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
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

  // Indicate that we add or remove indices.
  indexSet.beginResize();

  if(rank==0) {
    indexSet.add(0, LocalIndex(0,overlap,true));
    indexSet.add(2, LocalIndex(1,owner,true));
    indexSet.add(6, LocalIndex(2,owner,true));
    indexSet.add(3, LocalIndex(3,owner,true));
    indexSet.add(5, LocalIndex(4,owner,true));
  }
  if(rank==1) {

    indexSet.add(0, LocalIndex(0,owner,true));
    indexSet.add(1, LocalIndex(1,owner,true));
    indexSet.add(7, LocalIndex(2,owner,true));
    indexSet.add(5, LocalIndex(3,overlap,true));
    indexSet.add(4, LocalIndex(4,owner,true));
  }

  // Modification is over
  indexSet.endResize();

  // Print the index set
  std::cout<<indexSet<<std::endl;

  // Let MPI do a cleanup
  MPI_Finalize();

  return 0;
}
