// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include "config.h"

#include <dune/common/parallel/indicessyncer.hh>
#include <dune/common/sllist.hh>
#include <string>
#include <tuple>
#include <iostream>

enum GridFlags {
  owner, overlap, border
};

template<typename T>
void deleteOverlapEntries(T& indices,
                          Dune::RemoteIndices<T>& remoteIndices)
{
  typedef typename T::iterator IndexIterator;
  typedef typename T::GlobalIndex GlobalIndex;
  typedef typename T::LocalIndex::Attribute Attribute;
  typedef Dune::RemoteIndices<T> RemoteIndices;
  typedef typename RemoteIndices::RemoteIndexList::ModifyIterator RemoteModifier;
  typedef typename RemoteIndices::RemoteIndexList::const_iterator RemoteIterator;
  typedef Dune::SLList<std::pair<GlobalIndex,Attribute>, typename RemoteIndices::RemoteIndexList::Allocator> GlobalList;
  typedef typename GlobalList::ModifyIterator GlobalModifier;
  typedef std::tuple<RemoteModifier,GlobalModifier,const RemoteIterator,const typename GlobalList::const_iterator,
      const GlobalList*, const typename RemoteIndices::RemoteIndexList*> IteratorTuple;
  typedef std::map<int,IteratorTuple> IteratorMap;
  typedef typename RemoteIndices::const_iterator RemoteMapIterator;

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::map<int,GlobalList> globalLists;

  IteratorMap iterators;
  RemoteMapIterator rmEnd = remoteIndices.end();

  for(RemoteMapIterator remote = remoteIndices.begin();
      remote != rmEnd; ++remote) {
    // Initialize global indices
    GlobalList& gList=globalLists[remote->first];
    const RemoteIterator rend = remote->second.first->end();

    for(RemoteIterator index= remote->second.first->begin();
        index != rend; ++index)
      gList.push_back(std::make_pair(index->localIndexPair().global(),
                                     index->localIndexPair().local().attribute()));

    assert(gList.size()==remote->second.first->size());
    std::cout << "Size of remote indices is "<<gList.size()<<std::endl;

    iterators.insert(std::make_pair(remote->first,
                                    IteratorTuple(remote->second.first->beginModify(),
                                                  gList.beginModify(),
                                                  rend,
                                                  gList.end(),
                                                  &gList,
                                                  remote->second.first)));
  }

  indices.beginResize();

  const IndexIterator endIndex = indices.end();
  for(IndexIterator index = indices.begin(); index != endIndex; ++index) {
    if(index->local().attribute()==overlap) {
      std::cout << rank<<": Deleting "<<*index<<std::endl;

      indices.markAsDeleted(index);

      // Delete corresponding indices in all remote index lists.
      typedef typename IteratorMap::iterator iterator;
      iterator end = iterators.end();

      for(iterator remote = iterators.begin(); remote != end; ++remote) {

        // Search for the index
        while(std::get<0>(remote->second) != std::get<2>(remote->second)
              && *(std::get<1>(remote->second)) < *index) {
          // increment all iterators
          ++(std::get<0>(remote->second));
          ++(std::get<1>(remote->second));
          if(std::get<0>(remote->second)!=std::get<2>(remote->second))
            assert(std::get<1>(remote->second)!=std::get<3>(remote->second));
        }

        // Delete the entry if present
        if(std::get<0>(remote->second) != std::get<2>(remote->second)) {
          assert(std::get<1>(remote->second) != std::get<3>(remote->second));

          if(*(std::get<1>(remote->second)) == *index) {

            std::cout<<rank<<": Deleting remote "<<
            std::get<1>(remote->second)->first<<", "<<
            std::get<1>(remote->second)->second<<" of process "
            << remote->first<<std::endl;

            // Delete entries
            std::get<0>(remote->second).remove();
            std::get<1>(remote->second).remove();
            assert(std::get<4>(remote->second)->size()==std::get<5>(remote->second)->size());
          }
        }
      }
    }
  }

  indices.endResize();

  // Update the pointers to the local index pairs
  Dune::repairLocalIndexPointers(globalLists, remoteIndices, indices);
  globalLists.clear();
}


template<typename T>
bool areEqual(T& indices,
              Dune::RemoteIndices<T>& remoteIndices,
              T& oIndices,
              Dune::RemoteIndices<T>& oRemoteIndices){

  typedef typename T::iterator IndexIterator;
  typedef Dune::RemoteIndices<T> RemoteIndices;
  typedef typename RemoteIndices::RemoteIndexList::iterator RemoteIterator;

  IndexIterator iEnd = indices.end();
  bool ret=true;
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Test the index sets
  if(indices.size() != oIndices.size()) {
    std::cerr<< rank<<": Size of index set is unequal!"<<std::endl;
    ret= false;
  }
  for(IndexIterator index = indices.begin(), oIndex = oIndices.begin();
      index != iEnd; ++index, ++oIndex) {
    if( index->global() != oIndex->global()) {
      std::cerr<<rank<<": Entry for "<<index->global() <<" is missing!"<<std::endl;
      ret = false;
    }
    else if(index->local().attribute() !=oIndex->local().attribute()) {
      std::cerr<<rank<<": Entry for "<<index->global() <<" has wrong attribute: "<<
      index->local().attribute()<< "!= "<<oIndex->local().attribute()<<std::endl;
      ret = false;
    }
  }

  // Test the remote index lists
  typedef typename RemoteIndices::RemoteIndexMap::const_iterator RemoteMapIterator;

  RemoteMapIterator rmEnd = remoteIndices.end();

  for(RemoteMapIterator remote = remoteIndices.begin(),
      oRemote = oRemoteIndices.begin();
      remote != rmEnd; ++remote, ++oRemote) {
    if(oRemote->second.first->size() != remote->second.first->size()) {
      std::cerr <<rank<<": Size of remote index list for process "<<remote->first
                <<" does not match!"<<std::endl;
      ret=false;
    }

    RemoteIterator rEnd = oRemote->second.first->end();
    for(RemoteIterator rIndex= remote->second.first->begin(),
        oRIndex = oRemote->second.first->begin(); oRIndex != rEnd;
        ++rIndex, ++oRIndex) {

      if(rIndex->localIndexPair().global() != oRIndex->localIndexPair().global()) {

        std::cerr<<rank<<": Remote Entry for "<< rIndex->localIndexPair().global()
                 <<" is missing for process "<<remote->first<<std::endl;
        ret = false;
      }

      if(rIndex->attribute() != oRIndex->attribute()) {
        std::cerr<<rank<<": Attribute for entry "<<rIndex->localIndexPair().global()
                 <<" for process "<< remote->first<<" is wrong: "
                 <<rIndex->attribute()<<" != "<<oRIndex->attribute()<<std::endl;
        ret = false;
      }
    }
  }

  return ret;
}

template<typename T>
void addFakeRemoteIndices(T& indices,
                          T& oIndices,
                          Dune::RemoteIndices<T>& remoteIndices,
                          Dune::RemoteIndices<T>& oRemoteIndices){
  typedef typename T::iterator IndexIterator;
  typedef typename T::GlobalIndex GlobalIndex;
  typedef typename T::LocalIndex::Attribute Attribute;
  typedef typename Dune::RemoteIndices<T>::RemoteIndexList RemoteIndexList;
  assert(remoteIndices.neighbours()==0 && oRemoteIndices.neighbours()==0);

  RemoteIndexList* rlist = new RemoteIndexList();
  RemoteIndexList* orlist = new RemoteIndexList();
  int added=0;
  IndexIterator iEnd = indices.end();

  for(IndexIterator index = indices.begin(), oIndex = oIndices.begin();
      index != iEnd; ++index, ++oIndex) {
    assert(*index == *oIndex);
    if(index->local().attribute()==overlap) {
      added++;
      rlist->push_back(Dune::RemoteIndex<GlobalIndex,Attribute>(owner,&(*index)));
      orlist->push_back(Dune::RemoteIndex<GlobalIndex,Attribute>(owner,&(*oIndex)));
    }
  }


  remoteIndices.remoteIndices_.insert(std::make_pair(1,std::make_pair(rlist,rlist)));
  oRemoteIndices.remoteIndices_.insert(std::make_pair(1,std::make_pair(orlist,orlist)));

  std::cout<<"Added "<<added<<" fake remote indices!"<<std::endl;
}

bool testIndicesSyncer()
{
  //using namespace Dune;

  // The global grid size
  const int Nx = 6;
  const int Ny = 1;

  // Process configuration
  int procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // The local grid
  int nx = Nx/procs;
  int first=Nx%procs;
  // distributed indexset
  //  typedef ParallelLocalIndex<GridFlags> LocalIndexType;

  typedef Dune::ParallelIndexSet<int,Dune::ParallelLocalIndex<GridFlags> > ParallelIndexSet;
  ParallelIndexSet indexSet, changedIndexSet;

  // Set up the indexsets.
  int start,end, ostart, oend;
  if(rank<first) {
    start = rank*nx+rank;
    end = rank +rank * nx+nx+1;
  }else{
    start = first+rank*nx;
    end = first +rank*nx +nx;
  }

  if(rank>0 &&start<Nx)
    ostart=start-1;
  else
    ostart=start;

  if(rank<procs-1 &&end<Nx)
    oend=end+1;
  else
    oend=end;

  std::cout<<rank<<": ostart="<<ostart<<" start="<<start<<" end="<<end<<" oend="<<oend<<std::endl;
  //return true;

  indexSet.beginResize();
  changedIndexSet.beginResize();

  int localIndex=0;

  for(int j=0; j<Ny; j++)
    for(int i=ostart; i<oend; i++) {
      bool isPublic = (i<start+1)||(i>=end-1);
      GridFlags flag = owner;
      if((i==ostart && (i!=0))||(i==end && (i!=Nx-1))) {
        flag = overlap;
      }

      indexSet.add(i+j*Nx, Dune::ParallelLocalIndex<GridFlags> (localIndex,flag,isPublic));
      changedIndexSet.add(i+j*Nx, Dune::ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
    }

  indexSet.endResize();
  changedIndexSet.endResize();

  Dune::RemoteIndices<ParallelIndexSet> remoteIndices(indexSet, indexSet, MPI_COMM_WORLD);
  Dune::RemoteIndices<ParallelIndexSet> changedRemoteIndices(changedIndexSet, changedIndexSet, MPI_COMM_WORLD);

  remoteIndices.rebuild<false>();
  changedRemoteIndices.rebuild<false>();


  std::cout<<rank<<": Unchanged: "<<indexSet<<std::endl<<remoteIndices<<std::endl;
  assert(areEqual(indexSet, remoteIndices,changedIndexSet, changedRemoteIndices));

  std::cout<<"Deleting entries!"<<std::endl;

  //if(procs==1)
  //addFakeRemoteIndices(indexSet, changedIndexSet, remoteIndices, changedRemoteIndices);

  deleteOverlapEntries(changedIndexSet, changedRemoteIndices);
  std::cout<<rank<<": Changed:   "<<changedIndexSet<<std::endl<<changedRemoteIndices<<std::endl;

  Dune::IndicesSyncer<ParallelIndexSet> syncer(changedIndexSet, changedRemoteIndices);
  //  return 0;

  std::cout<<"Syncing!"<<std::endl;

  syncer.sync();

  std::cout<<rank<<": Synced:   "<<changedIndexSet<<std::endl<<changedRemoteIndices<<std::endl;
  if( areEqual(indexSet, remoteIndices,changedIndexSet, changedRemoteIndices))
    return true;
  else{
    std::cerr<<"Output not equal!"<<std::endl;
    return false;
  }


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

void MPI_err_handler([[maybe_unused]] MPI_Comm *comm, int *err_code, ...)
{
  char *err_string=new char[MPI_MAX_ERROR_STRING];
  int err_length;
  MPI_Error_string(*err_code, err_string, &err_length);
  std::string s(err_string, err_length);
  std::cerr << "An MPI Error occurred:"<<std::endl<<s<<std::endl;
  delete[] err_string;
  throw MPIError(s, *err_code);
}

int main(int argc, char** argv){
  MPI_Init(&argc, &argv);
  MPI_Errhandler handler;
  MPI_Comm_create_errhandler(MPI_err_handler, &handler);
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, handler);
  int procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  bool ret=testIndicesSyncer();
  MPI_Barrier(MPI_COMM_WORLD);
  std::cout<<rank<<": ENd="<<ret<<std::endl;
  if(!ret)
    MPI_Abort(MPI_COMM_WORLD, 1);
  MPI_Finalize();
  return 0;
}
