// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/istl/indexset.hh>
#include <dune/istl/communicator.hh>
#include <dune/istl/remoteindices.hh>
#include <dune/common/enumset.hh>
#include <algorithm>
#include <iostream>
#include "mpi.h"

enum GridFlags {
  owner, overlap, border
};

class Array;

std::ostream& operator<<(std::ostream& os, const Array& a);

class Array
{
  friend std::ostream& operator<<(std::ostream& os, const Array& a);
public:
  typedef double IndexedType;
  Array() : size_(-1)
  {}

  Array(int size) : size_(size)
  {
    vals_ = new double[size];
  }

  void build(int size)
  {
    vals_ = new double[size];
    size_ = size;
  }

  Array& operator+=(double d)
  {
    for(int i=0; i < size_; i++)
      vals_[i]+=d;
    return *this;
  }

  ~Array()
  {
    delete[] vals_;
  }

  const double& operator[](int i) const
  {
    return vals_[i];
  }

  double& operator[](int i)
  {
    return vals_[i];
  }
private:
  double *vals_;
  int size_;
};

std::ostream& operator<<(std::ostream& os, const Array& a)
{
  if(a.size_>0)
    os<< "{ "<<a.vals_[0];

  for(int i=1; i<a.size_; i++)
    os <<", "<< a.vals_[i];

  os << " }";
  return os;
}

void testIndices()
{
  //using namespace Dune;

  // The global grid size
  const int Nx = 20;
  const int Ny = 2;

  // Process configuration
  int procs, rank, master=0;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // shift the ranks
  //rank = (rank + 1) % procs;
  //master= (master+1) %procs;

  // The local grid
  int nx = Nx/procs;
  // distributed indexset
  //  typedef ParallelLocalIndex<GridFlags> LocalIndexType;

  Dune::IndexSet<int,Dune::ParallelLocalIndex<GridFlags> > distIndexSet;
  // global indexset
  Dune::IndexSet<int,Dune::ParallelLocalIndex<GridFlags> > globalIndexSet;

  // Set up the indexsets.
  int start = std::max(rank*nx-1,0);
  int end = std::min((rank + 1) * nx+1, Nx);

  distIndexSet.beginResize();

  int localIndex=0;
  int size = Ny*(end-start);
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

  Dune::RemoteIndices<int,GridFlags> accuIndices(distIndexSet, globalIndexSet, MPI_COMM_WORLD);

  accuIndices.rebuild<true>();
  //    std::cout << accuIndices<<std::endl<<std::flush;

  //    Dune::RemoteIndices<int,GridFlags> remote(distIndexSet, distIndexSet, MPI_COMM_WORLD);
  //remote.rebuild<false>();


  Dune::Communicator<int,GridFlags> accumulator(accuIndices);
  Dune::Communicator<int,GridFlags>  overlapExchanger(distIndexSet, distIndexSet, MPI_COMM_WORLD);

  Dune::EnumItem<GridFlags,owner> sourceFlags;
  Dune::Combine<Dune::EnumItem<GridFlags,overlap>,Dune::EnumItem<GridFlags,owner>,GridFlags> destFlags;
  Dune::Bool2Type<true> flag;

  accumulator.build(sourceFlags, distArray, destFlags, *globalArray, flag);

  overlapExchanger.build(Dune::EnumItem<GridFlags,owner>(), distArray, Dune::EnumItem<GridFlags,overlap>(), distArray);

  std::cout<< rank<<": before forward distArray="<< distArray<<std::endl;

  // Exchange the overlap
  overlapExchanger.forward();

  std::cout<<rank<<": overlap exchanged distArray"<< distArray<<std::endl;

  if(rank==master)
    std::cout<<": before forward globalArray="<< *globalArray<<std::endl;

  accumulator.forward();


  if(rank==master) {
    std::cout<<"after forward global: "<<*globalArray<<std::endl;
    *globalArray+=1;
    std::cout<<" added one: globalArray="<<*globalArray<<std::endl;
  }

  accumulator.backward();
  std::cout<< rank<<": after backward distArray"<< distArray<<std::endl;


  // Exchange the overlap
  overlapExchanger.forward();

  std::cout<<rank<<": overlap exchanged distArray"<< distArray<<std::endl;

  //std::cout << rank<<": source and dest are the same:"<<std::endl;
  //std::cout << remote<<std::endl<<std::flush;
  //    delete globalArray;
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

  Array array, redistributedArray;

  // Set up the indexsets.
  {

    int start = std::max(rank*nx-1,0);
    int end = std::min((rank + 1) * nx+1, Nx);

    sendIndexSet.beginResize();


    array.build(Ny*(end-start));

    for(int j=0, localIndex=0; j<Ny; j++)
      for(int i=start; i<end; i++, localIndex++) {
        bool isPublic = (i<=start+1)||(i>=end-2);
        GridFlags flag = owner;

        if((i==start && i!=0)||(i==end-1 && i!=Nx-1))
          flag = overlap;

        sendIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (localIndex,flag,isPublic));
        array[localIndex]=i+j*Nx+rank*Nx*Ny;
      }

    sendIndexSet.endResize();
  }
  {
    int newrank = (rank + 1) % procs;

    int start = std::max(newrank*nx-1,0);
    int end = std::min((newrank + 1) * nx+1, Nx);

    std::cout<<rank<<": "<<newrank<<" start="<<start<<" end"<<end<<std::endl;

    redistributedArray.build(Ny*(end-start));

    receiveIndexSet.beginResize();

    for(int j=0, localIndex=0; j<Ny; j++)
      for(int i=start; i<end; i++, localIndex++) {
        bool isPublic = (i<=start+1)||(i>=end-2);
        GridFlags flag = owner;

        if((i==start && i!=0)||(i==end-1 && i!=Nx-1))
          flag = overlap;

        receiveIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (localIndex,flag,isPublic));
        redistributedArray[localIndex]=-1;
      }

    receiveIndexSet.endResize();
  }


  std::cout<< rank<<": distributed and global index set!"<<std::endl<<std::flush;
  Communicator<int,GridFlags> redistribute(sendIndexSet,
                                           receiveIndexSet, MPI_COMM_WORLD);
  Communicator<int, GridFlags> overlapComm(receiveIndexSet, receiveIndexSet, MPI_COMM_WORLD);
  EnumItem<GridFlags,owner> fowner;
  EnumItem<GridFlags,overlap> foverlap;

  redistribute.build(fowner, array, fowner, redistributedArray, Bool2Type<true>());

  overlapComm.build(fowner, redistributedArray, foverlap, redistributedArray);
  std::cout<<rank<<": initial array: "<<array<<std::endl;

  redistribute.forward();

  std::cout<<rank<<": redistributed array: "<<redistributedArray<<std::endl;

  overlapComm.forward();

  std::cout<<rank<<": redistributed array with overlap communicated: "<<redistributedArray<<std::endl;
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  int rank;
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef DEBUG
  bool wait=1;
  while(size>1 && rank==0 && wait) ;
#endif
  testIndices();
  if(rank==0)
    std::cout<<std::endl<<"Redistributing!"<<std::endl<<std::endl;
  testRedistributeIndices();
  MPI_Finalize();
}
