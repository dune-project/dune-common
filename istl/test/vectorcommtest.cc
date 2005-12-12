// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/istl/indexset.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/remoteindices.hh>
#include <dune/istl/bvector.hh>
#include <dune/common/enumset.hh>
#include <dune/common/fvector.hh>
#include <algorithm>
#include <iostream>
#include "mpi.h"

enum GridFlags {
  owner, overlap, border
};

void testIndices(MPI_Comm comm)
{
  //using namespace Dune;

  // The global grid size
  const int Nx = 20;
  const int Ny = 2;

  // Process configuration
  int procs, rank, master=0;
  MPI_Comm_size(comm, &procs);
  MPI_Comm_rank(comm, &rank);

  // shift the ranks
  //rank = (rank + 1) % procs;
  //master= (master+1) %procs;

  // The local grid
  int nx = Nx/procs;
  // distributed indexset
  //  typedef ParallelLocalIndex<GridFlags> LocalIndexType;

  typedef Dune::ParallelIndexSet<int,Dune::ParallelLocalIndex<GridFlags>,45> ParallelIndexSet;

  ParallelIndexSet distIndexSet;
  // global indexset
  ParallelIndexSet globalIndexSet;

  // Set up the indexsets.
  int start = std::max(rank*nx-1,0);
  int end = std::min((rank + 1) * nx+1, Nx);

  distIndexSet.beginResize();

  int localIndex=0;
  int size = Ny*(end-start);

  typedef Dune::FieldVector<int,5> Vector;
  typedef Dune::BlockVector<Vector> Array;

  Array distArray(size);
  Array* globalArray;
  int index=0;

  for(int j=0; j<Ny; j++)
    for(int i=start; i<end; i++) {
      bool isPublic = (i<=start+1)||(i>=end-2);
      GridFlags flag = owner;
      if((i==start && i!=0)||(i==end-1 && i!=Nx-1)) {
        distArray[index++]=-(i+j*Nx+rank*Nx*Ny);
        flag = overlap;
      }else
        distArray[index++]=i+j*Nx+rank*Nx*Ny;

      distIndexSet.add(i+j*Nx, Dune::ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
    }

  distIndexSet.endResize();

  if(rank==master) {
    // build global indexset on first process
    globalIndexSet.beginResize();
    globalArray=new Array(Nx*Ny);
    int k=0;
    for(int j=0; j<Ny; j++)
      for(int i=0; i<Nx; i++) {
        globalIndexSet.add(i+j*Nx, Dune::ParallelLocalIndex<GridFlags> (i+j*Nx,owner,false));
        globalArray->operator[](i+j*Nx)=-(i+j*Nx);
        k++;

      }

    globalIndexSet.endResize();
  }else
    globalArray=new Array(0);

  typedef Dune::RemoteIndices<ParallelIndexSet> RemoteIndices;

  RemoteIndices accuIndices(distIndexSet, globalIndexSet,  comm);
  RemoteIndices overlapIndices(distIndexSet, distIndexSet, comm);
  accuIndices.rebuild<true>();
  overlapIndices.rebuild<false>();

  Dune::DatatypeCommunicator<ParallelIndexSet> accumulator, overlapExchanger;

  Dune::EnumItem<GridFlags,owner> sourceFlags;
  Dune::Combine<Dune::EnumItem<GridFlags,overlap>,Dune::EnumItem<GridFlags,owner>,GridFlags> destFlags;

  accumulator.build(accuIndices, sourceFlags, distArray, destFlags, *globalArray);

  overlapExchanger.build(overlapIndices, Dune::EnumItem<GridFlags,owner>(), distArray, Dune::EnumItem<GridFlags,overlap>(), distArray);

  std::cout<< rank<<": before forward distArray="<< distArray<<std::endl;

  // Exchange the overlap
  overlapExchanger.forward();

  std::cout<<rank<<": overlap exchanged distArray"<< distArray<<std::endl;

  if(rank==master)
    std::cout<<": before forward globalArray="<< *globalArray<<std::endl;

  accumulator.forward();


  if(rank==master) {
    std::cout<<"after forward global: "<<*globalArray<<std::endl;
    *globalArray*=2;
    std::cout<<" added one: globalArray="<<*globalArray<<std::endl;
  }

  accumulator.backward();
  std::cout<< rank<<": after backward distArray"<< distArray<<std::endl;


  // Exchange the overlap
  overlapExchanger.forward();

  std::cout<<rank<<": overlap exchanged distArray"<< distArray<<std::endl;

  //std::cout << rank<<": source and dest are the same:"<<std::endl;
  //std::cout << remote<<std::endl<<std::flush;
  if(rank==master)
    delete globalArray;
}

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  testIndices(MPI_COMM_WORLD);
  MPI_Finalize();

}
