// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/indexset.hh>
#include <dune/istl/remoteindices.hh>
#include "mpi.h"

enum GridFlags {
  owner, overlap, border
};


void testIndices()
{
  using namespace Dune;

  // The global grid size
  const int N = 10;
  // Process configuration
  int procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // The local grid
  int n = N/procs;
  // distributed indexset
  //  typedef ParallelLocalIndex<GridFlags> LocalIndexType;

  IndexSet<int,ParallelLocalIndex<GridFlags> > distIndexSet;
  // global indexset
  IndexSet<int,ParallelLocalIndex<GridFlags> >* globalIndexSet;

  // Set up the indexsets.
  int start = rank*n;
  int end = (rank + 1) * n;

  distIndexSet.beginResize();

  for(int i=start; i<end; i++)
    for(int j=start; j<end; j++) {
      bool isPublic = true|| (i==start)||(i==end)||(j==start)||(j==end);
      GridFlags flag = owner;

      if((i==start && i!=0)||(i==end && i!=N-1)
         ||(j==start && j==0)||(j==end && j!=N-1))
        flag = overlap;



      distIndexSet.add(i*N+j, ParallelLocalIndex<GridFlags> (flag,isPublic));
    }

  distIndexSet.endResize();

  // build global indexset on first process
  if(rank==0) {
    globalIndexSet = new IndexSet<int,ParallelLocalIndex<GridFlags> >();
    globalIndexSet->beginResize();
    for(int i=0; i<N; i++)
      for(int j=0; j<N; j++)
        globalIndexSet->add(i*N+j, ParallelLocalIndex<GridFlags> (owner,false));
    globalIndexSet->endResize();
  }

  // Build remote indices
  RemoteIndices<int,GridFlags> distRemote(distIndexSet,
                                          distIndexSet, MPI_COMM_WORLD);
  if(rank==0) {
    RemoteIndices<int,GridFlags> crossRemote(distIndexSet,
                                             *globalIndexSet, MPI_COMM_WORLD);
    delete globalIndexSet;
  }else
    RemoteIndices<int,GridFlags> distRemote(distIndexSet,
                                            distIndexSet, MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  testIndices();
  MPI_Finalize();
}
