// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/indexset.hh>
#include <dune/istl/remoteindices.hh>
#include <algorithm>
#include "mpi.h"

enum GridFlags {
  owner, overlap, border
};


void testIndices()
{
  using namespace Dune;

  // The global grid size
  const int Nx = 20;
  const int Ny = 2;

  // Process configuration
  int procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // The local grid
  int nx = Nx/procs;
  // distributed indexset
  //  typedef ParallelLocalIndex<GridFlags> LocalIndexType;

  IndexSet<int,ParallelLocalIndex<GridFlags> > distIndexSet;
  // global indexset
  IndexSet<int,ParallelLocalIndex<GridFlags> > globalIndexSet;

  // Set up the indexsets.
  int start = std::max(rank*nx-1,0);
  int end = std::min((rank + 1) * nx+1, Nx);

  distIndexSet.beginResize();

  int localIndex=0;

  for(int j=0; j<Ny; j++)
    for(int i=start; i<end; i++) {
      bool isPublic = (i<=start+1)||(i>=end-2);
      GridFlags flag = owner;

      if((i==start && i!=0)||(i==end-1 && i!=Nx-1))
        flag = overlap;

      distIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
    }

  distIndexSet.endResize();

  // Build remote indices
  /*  RemoteIndices<int,GridFlags> distRemote(distIndexSet,
                                          distIndexSet, MPI_COMM_WORLD);
     distRemote.rebuild<false>();

     std::cout<<rank<<": "<<distRemote<<std::endl;
     std::cout<< rank<<": Finished!"<<std::endl;
   */
  if(rank==0) {
    // build global indexset on first process
    globalIndexSet.beginResize();
    for(int j=0; j<Ny; j++)
      for(int i=0; i<Nx; i++)
        globalIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (i+j*Nx,owner,false));
    globalIndexSet.endResize();
    std::cout<<std::flush;
  }

  std::cout<< rank<<": distributed and global index set!"<<std::endl<<std::flush;
  RemoteIndices<int,GridFlags> crossRemote(distIndexSet,
                                           globalIndexSet, MPI_COMM_WORLD);
  crossRemote.rebuild<true>();
  std::cout << crossRemote<<std::endl<<std::flush;
}

void testRedistributeIndices()
{
  using namespace Dune;

  // The global grid size
  const int Nx = 20;
  const int Ny = 2;

  // Process configuration
  int procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // The local grid
  int nx = Nx/procs;
  // distributed indexset

  IndexSet<int,ParallelLocalIndex<GridFlags> > sendIndexSet;
  // global indexset
  IndexSet<int,ParallelLocalIndex<GridFlags> > receiveIndexSet;

  // Set up the indexsets.
  {

    int start = std::max(rank*nx-1,0);
    int end = std::min((rank + 1) * nx+1, Nx);

    sendIndexSet.beginResize();

    int localIndex=0;

    for(int j=0; j<Ny; j++)
      for(int i=start; i<end; i++) {
        bool isPublic = (i<=start+1)||(i>=end-2);
        GridFlags flag = owner;

        if((i==start && i!=0)||(i==end-1 && i!=Nx-1))
          flag = overlap;

        sendIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
      }

    sendIndexSet.endResize();
  }
  {
    int newrank = (rank + 1) % procs;

    int start = std::max(newrank*nx-1,0);
    int end = std::min((newrank + 1) * nx+1, Nx);

    std::cout<<rank<<": "<<newrank<<" start="<<start<<" end"<<end<<std::endl;

    receiveIndexSet.beginResize();

    int localIndex=0;

    for(int j=0; j<Ny; j++)
      for(int i=start; i<end; i++) {
        bool isPublic = (i<=start+1)||(i>=end-2);
        GridFlags flag = owner;

        if((i==start && i!=0)||(i==end-1 && i!=Nx-1))
          flag = overlap;

        receiveIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
      }

    receiveIndexSet.endResize();
  }


  std::cout<< rank<<": distributed and global index set!"<<std::endl<<std::flush;
  RemoteIndices<int,GridFlags> crossRemote(sendIndexSet,
                                           receiveIndexSet, MPI_COMM_WORLD);
  crossRemote.rebuild<true>();
  std::cout << crossRemote<<std::endl<<std::flush;
}
int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
#ifdef DEBUG
  bool wait=1;
  while(wait) ;
#endif
  //testIndices();
  std::cout<<"Redistributing!"<<std::endl;
  testRedistributeIndices();
  MPI_Finalize();
}
