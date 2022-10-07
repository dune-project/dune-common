// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <config.h>

#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include <mpi.h>

#include <dune/common/enumset.hh>
#include <dune/common/parallel/communicator.hh>
#include <dune/common/parallel/indexset.hh>
#include <dune/common/parallel/interface.hh>
#include <dune/common/parallel/plocalindex.hh>
#include <dune/common/parallel/remoteindices.hh>

enum GridFlags {
  owner, overlap, border
};

class Array;

std::ostream& operator<<(std::ostream& os, const Array& a);

class Array
{
  friend std::ostream& operator<<(std::ostream& os, const Array& a);
public:
  typedef double value_type;
  Array() : vals_(0), size_(0)
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
    if(vals_!=0)
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
  Array(const Array&)
  {}
  double *vals_;
  int size_;
};

struct ArrayGatherScatter
{
  static double gather(const Array& a, int i);

  static void scatter(Array& a, double v, int i);

};


inline double ArrayGatherScatter::gather(const Array& a, int i)
{
  return a[i];
}

inline void ArrayGatherScatter::scatter(Array& a, double v, int i)
{
  a[i]=v;

}

std::ostream& operator<<(std::ostream& os, const Array& a)
{
  if(a.size_>0)
    os<< "{ "<<a.vals_[0];

  for(int i=1; i<a.size_; i++)
    os <<", "<< a.vals_[i];

  os << " }";
  return os;
}

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
  Array distArray(size);
  Array* globalArray;
  int index=0;

  std::cout<<rank<<": Nx="<<Nx<<" Ny="<<Ny<<" size="<<size<<std::endl;

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
  if(rank==master)
    delete globalArray;
}


template<int NX, int NY, typename TG, typename TA>
void setupDistributed(Array& distArray, Dune::ParallelIndexSet<TG,Dune::ParallelLocalIndex<TA> >& distIndexSet,
                      int rank, int procs)
{
  // The local grid
  int nx = NX/procs;
  int mod = NX%procs;

  // Set up the indexsets.
  int start, end;
  int ostart, oend;

  if(rank<mod) {
    start = rank * (nx + 1);
    end   = start + (nx + 1);
  }else{
    start = mod + rank * nx;
    end   = start + nx;
  }

  if(rank>0)
    ostart = start - 1;
  else
    ostart = start;

  if(rank<procs-1)
    oend = end+1;
  else
    oend = end;

  distIndexSet.beginResize();

  int localIndex=0;
  int size = NY*(oend-ostart);

  distArray.build(size);

  for(int j=0; j<NY; j++)
    for(int i=ostart; i<oend; i++) {
      bool isPublic = (i<=start+1)||(i>=end-1);
      GridFlags flag = owner;
      if((i<start || i>=end)) {
        distArray[localIndex]=-(i+j*NX+rank*NX*NY);
        flag = overlap;
      }else
        distArray[localIndex]=i+j*NX+rank*NX*NY;

      distIndexSet.add(i+j*NX, Dune::ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
    }

  distIndexSet.endResize();


}

template<int NX,int NY, typename TG, typename TA>
void setupGlobal(Array& globalArray, Dune::ParallelIndexSet<TG,Dune::ParallelLocalIndex<TA> >& globalIndexSet)
{
  // build global indexset on first process
  globalIndexSet.beginResize();
  globalArray.build(NX*NY);
  int k=0;
  for(int j=0; j<NY; j++)
    for(int i=0; i<NX; i++) {
      globalIndexSet.add(i+j*NX, Dune::ParallelLocalIndex<GridFlags> (i+j*NX,owner,false));
      globalArray[i+j*NX]=-(i+j*NX);
      k++;

    }

  globalIndexSet.endResize();
}

void testIndicesBuffered(MPI_Comm comm)
{
  //using namespace Dune;

  // The global grid size
  const int Nx = 8;
  const int Ny = 1;

  // Process configuration
  int procs, rank, master=0;
  MPI_Comm_size(comm, &procs);
  MPI_Comm_rank(comm, &rank);

  typedef Dune::ParallelIndexSet<int,Dune::ParallelLocalIndex<GridFlags> > ParallelIndexSet;

  ParallelIndexSet distIndexSet;
  // global indexset
  ParallelIndexSet globalIndexSet;

  Array distArray;
  Array globalArray;

  setupDistributed<Nx,Ny>(distArray, distIndexSet, rank, procs);


  if(rank==master) {
    setupGlobal<Nx,Ny>(globalArray, globalIndexSet);
  }

  typedef Dune::RemoteIndices<ParallelIndexSet> RemoteIndices;

  RemoteIndices accuIndices(distIndexSet, globalIndexSet, comm);

  accuIndices.rebuild<true>();
  std::cout<<"dist "<<rank<<": "<<distIndexSet<<std::endl;
  std::cout<<"global "<<rank<<": "<<globalIndexSet<<std::endl;
  std::cout << accuIndices<<std::endl;
  std::cout <<" end remote indices"<<std::endl;

  RemoteIndices overlapIndices(distIndexSet, distIndexSet, comm);
  overlapIndices.rebuild<false>();

  Dune::Interface accuInterface;
  Dune::Interface overlapInterface;
  Dune::EnumItem<GridFlags,owner> sourceFlags;
  Dune::Combine<Dune::EnumItem<GridFlags,overlap>,Dune::EnumItem<GridFlags,owner>,GridFlags> destFlags;
  //    Dune::Bool2Type<true> flag;

  accuInterface.build(accuIndices, sourceFlags, destFlags);
  overlapInterface.build(overlapIndices, Dune::EnumItem<GridFlags,owner>(),
                         Dune::EnumItem<GridFlags,overlap>());
  overlapInterface.print();
  accuInterface.print();

  //accuInterface.print();

  Dune::BufferedCommunicator accumulator, overlapExchanger;

  accumulator.build<Array>(accuInterface);

  overlapExchanger.build<Array>(overlapInterface);

  std::cout<< rank<<": before forward distArray="<< distArray<<std::endl;

  // Exchange the overlap
  overlapExchanger.forward<ArrayGatherScatter>(distArray, distArray);

  std::cout<<rank<<": overlap exchanged distArray"<< distArray<<std::endl;

  if(rank==master)
    std::cout<<": before forward globalArray="<< globalArray<<std::endl;

  accumulator.forward<ArrayGatherScatter>(distArray, globalArray);


  if(rank==master) {
    std::cout<<"after forward global: "<<globalArray<<std::endl;
    globalArray+=1;
    std::cout<<" added one: globalArray="<<globalArray<<std::endl;
  }

  accumulator.backward<ArrayGatherScatter>(distArray, globalArray);
  std::cout<< rank<<": after backward distArray"<< distArray<<std::endl;


  // Exchange the overlap
  overlapExchanger.forward<ArrayGatherScatter>(distArray);

  std::cout<<rank<<": overlap exchanged distArray"<< distArray<<std::endl;

  //std::cout << rank<<": source and dest are the same:"<<std::endl;
  //std::cout << remote<<std::endl<<std::flush;
}


void testRedistributeIndices(MPI_Comm comm)
{
  using namespace Dune;

  // The global grid size
  const int Nx = 20;
  const int Ny = 2;

  // Process configuration
  int procs, rank;
  MPI_Comm_size(comm, &procs);
  MPI_Comm_rank(comm, &rank);

  // The local grid
  int nx = Nx/procs;
  // distributed indexset

  typedef ParallelIndexSet<int,ParallelLocalIndex<GridFlags> > ParallelIndexSet;
  ParallelIndexSet sendIndexSet;
  // global indexset
  ParallelIndexSet receiveIndexSet;

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
  typedef RemoteIndices<ParallelIndexSet> RemoteIndices;

  RemoteIndices redistributeIndices(sendIndexSet,
                                    receiveIndexSet, comm);
  RemoteIndices overlapIndices(receiveIndexSet, receiveIndexSet, comm);

  redistributeIndices.rebuild<true>();
  overlapIndices.rebuild<false>();

  DatatypeCommunicator<ParallelIndexSet> redistribute, overlapComm;
  EnumItem<GridFlags,owner> fowner;
  EnumItem<GridFlags,overlap> foverlap;

  redistribute.build(redistributeIndices, fowner, array, fowner, redistributedArray);

  overlapComm.build(overlapIndices, fowner, redistributedArray, foverlap, redistributedArray);
  std::cout<<rank<<": initial array: "<<array<<std::endl;

  redistribute.forward();

  std::cout<<rank<<": redistributed array: "<<redistributedArray<<std::endl;

  overlapComm.forward();

  std::cout<<rank<<": redistributed array with overlap communicated: "<<redistributedArray<<std::endl;
}

void testRedistributeIndicesBuffered(MPI_Comm comm)
{
  using namespace Dune;

  // The global grid size
  const int Nx = 20;
  const int Ny = 2;

  // Process configuration
  int procs, rank;
  MPI_Comm_size(comm, &procs);
  MPI_Comm_rank(comm, &rank);

  // The local grid
  int nx = Nx/procs;
  // distributed indexset

  typedef ParallelIndexSet<int,ParallelLocalIndex<GridFlags> > ParallelIndexSet;
  ParallelIndexSet sendIndexSet;
  // global indexset
  ParallelIndexSet receiveIndexSet;

  Array array, redistributedArray;

  std::vector<int> neighbours;

  // Set up the indexsets.
  {

    int start = std::max(rank*nx-1,0);
    int end = std::min((rank + 1) * nx+1, Nx);

    neighbours.reserve(2);

    if(rank>0) neighbours.push_back(rank-1);
    if(rank<procs-1) neighbours.push_back(rank+1);

    sendIndexSet.beginResize();


    array.build(Ny*(end-start));

    for(int j=0, localIndex=0; j<Ny; j++)
      for(int i=start; i<end; i++, localIndex++) {
        bool isPublic = (i<=start+1)||(i>=end-2);
        GridFlags flag = owner;

        if((i==start && i!=0)||(i==end-1 && i!=Nx-1))
          flag = overlap;

        sendIndexSet.add(i+j*Nx, ParallelLocalIndex<GridFlags> (localIndex,flag,isPublic));
        array[localIndex]=i+j*Nx; //+rank*Nx*Ny;
        if(flag==overlap)
          array[localIndex]=-array[localIndex];
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

  typedef RemoteIndices<ParallelIndexSet> RemoteIndices;
  RemoteIndices redistributeIndices(sendIndexSet,
                                    receiveIndexSet, comm);
  RemoteIndices overlapIndices(receiveIndexSet, receiveIndexSet, comm);
  RemoteIndices sendIndices(sendIndexSet,
                            sendIndexSet, comm, neighbours);
  RemoteIndices sendIndices1(sendIndexSet,
                             sendIndexSet, comm);
  overlapIndices.rebuild<false>();
  redistributeIndices.rebuild<true>();
  sendIndices.rebuild<true>();
  sendIndices1.rebuild<true>();

  if(rank==0)
    std::cout<<sendIndices<<std::endl<<sendIndices1<<std::endl;

  assert(sendIndices==sendIndices1);

  std::cout<<redistributeIndices<<std::endl;

  Interface redistributeInterface, overlapInterface;
  EnumItem<GridFlags,owner> fowner;
  EnumItem<GridFlags,overlap> foverlap;

  redistributeInterface.build(redistributeIndices, fowner, fowner);
  overlapInterface.build(overlapIndices, fowner, foverlap);

  BufferedCommunicator redistribute;
  BufferedCommunicator overlapComm;

  redistribute.build(array, redistributedArray, redistributeInterface);
  overlapComm.build<Array>(overlapInterface);

  std::cout<<rank<<": initial array: "<<array<<std::endl;

  redistribute.forward<ArrayGatherScatter>(array, redistributedArray);

  std::cout<<rank<<": redistributed array: "<<redistributedArray<<std::endl;

  redistributedArray +=1;

  std::cout<<rank<<": redistributed array (added one): "<<redistributedArray<<std::endl;

  overlapComm.forward<ArrayGatherScatter>(redistributedArray);

  std::cout<<rank<<": redistributed array with overlap communicated: "<<redistributedArray<<std::endl;

  redistribute.backward<ArrayGatherScatter>(array, redistributedArray);

  std::cout<<rank<<": final array: "<<array<<std::endl;

  redistribute.forward<ArrayGatherScatter>(array, redistributedArray);

  std::cout<<rank<<": final array with overlap communicated: "<<array<<std::endl;
}


/**
 * @brief MPI Error.
 * Thrown when an mpi error occurs.
 */
class MPIError {
public:
  /** @brief Constructor. */
  MPIError(std::string s, int e) : errorstring(s), errorcode(e){}
  /** @brief The error string. */
  std::string errorstring;
  /** @brief The mpi error code. */
  int errorcode;
};

void MPI_err_handler([[maybe_unused]] MPI_Comm *comm, int *err_code, ...){
  char *err_string=new char[MPI_MAX_ERROR_STRING];
  int err_length;
  MPI_Error_string(*err_code, err_string, &err_length);
  std::string s(err_string, err_length);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cerr << rank<<": An MPI Error occurred:"<<std::endl<<s<<std::endl;
  delete[] err_string;
  throw MPIError(s, *err_code);
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  MPI_Errhandler handler;
  MPI_Comm_create_errhandler(MPI_err_handler, &handler);
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, handler);
  int rank;
  int size;
  const int firstRank=2;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Comm comm;
  int key = rank;

  if(size>firstRank) {
    if(rank==0)
      key = firstRank;
    if(rank==firstRank)
      key=0;
  }

  MPI_Comm_split(MPI_COMM_WORLD, 0, key, &comm);

#ifdef DEBUG
  bool wait=1;
  while(size>1 && wait) ;
#endif

  //  testIndices(comm);
  testIndicesBuffered(comm);

  if(rank==0)
    std::cout<<std::endl<<"Redistributing bla!"<<std::endl<<std::endl;
  MPI_Barrier(comm);


  //  testRedistributeIndices(comm);
  testRedistributeIndicesBuffered(comm);
  MPI_Comm_free(&comm);
  MPI_Finalize();

  return 0;
}
