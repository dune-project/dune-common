// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/istl/indicessyncer.hh>
#include <dune/common/sllist.hh>
#include <string>
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
  typedef Dune::RemoteIndices<T> RemoteIndices;
  typedef typename RemoteIndices::RemoteIndexList::ModifyIterator RemoteModifier;
  typedef typename RemoteIndices::RemoteIndexList::const_iterator RemoteIterator;
  typedef Dune::SLList<GlobalIndex, typename RemoteIndices::RemoteIndexList::Allocator> GlobalList;
  typedef typename GlobalList::ModifyIterator GlobalModifier;
  typedef Dune::Tuple<RemoteModifier,GlobalModifier,const RemoteIterator,const typename GlobalList::const_iterator,
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
      gList.push_back(index->localIndexPair().global());

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
        while(Dune::Element<0>::get(remote->second) != Dune::Element<2>::get(remote->second)
              && *(Dune::Element<1>::get(remote->second)) < index->global()) {
          // increment all iterators
          ++(Dune::Element<0>::get(remote->second));
          ++(Dune::Element<1>::get(remote->second));
          if(Dune::Element<0>::get(remote->second)!=Dune::Element<2>::get(remote->second))
            assert(Dune::Element<1>::get(remote->second)!=Dune::Element<3>::get(remote->second));
        }

        // Delete the entry if present
        if(Dune::Element<0>::get(remote->second) != Dune::Element<2>::get(remote->second)) {
          assert(Dune::Element<1>::get(remote->second) != Dune::Element<3>::get(remote->second));

          if(*(Dune::Element<1>::get(remote->second)) == index->global()) {

            std::cout<<rank<<": Deleting remote "<<*(Dune::Element<1>::get(remote->second))<<" of process "
                     << remote->first<<std::endl;

            // Delete entries
            Dune::Element<0>::get(remote->second).remove();
            Dune::Element<1>::get(remote->second).remove();
            assert(Dune::Element<4>::get(remote->second)->size()==Dune::Element<5>::get(remote->second)->size());
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

  // Test the index sets
  if(indices.size() != oIndices.size())
    return false;

  for(IndexIterator index = indices.begin(), oIndex = oIndices.begin();
      index != iEnd; ++index, ++oIndex) {
    if( index->global() != oIndex->global()) {
      std::cerr<<"Entry for "<<index->global() <<" is missing!"<<std::endl;
      return false;
    }
    if(index->local().attribute() !=oIndex->local().attribute()) {
      std::cerr<<"Entry for "<<index->global() <<" has wrong attribute: "<<
      index->local().attribute()<< "!= "<<oIndex->local().attribute()<<std::endl;
      return false;
    }
  }

  // Test the remote index lists
  typedef typename RemoteIndices::RemoteIndexMap::const_iterator RemoteMapIterator;

  RemoteMapIterator rmEnd = remoteIndices.end();

  for(RemoteMapIterator remote = remoteIndices.begin(),
      oRemote = oRemoteIndices.begin();
      remote != rmEnd; ++remote, ++oRemote) {
    if(oRemote->second.first->size() != remote->second.first->size())
      std::cerr <<" Size of remote index list for process "<<remote->first
                <<" does not match!"<<std::endl;

    RemoteIterator rEnd = oRemote->second.first->end();
    for(RemoteIterator rIndex= remote->second.first->begin(),
        oRIndex = oRemote->second.first->begin(); oRIndex != rEnd;
        ++rIndex, ++oRIndex) {

      if(rIndex->localIndexPair().global() != oRIndex->localIndexPair().global()) {

        std::cerr<<"Remote Entry for "<< rIndex->localIndexPair().global()
                 <<" is missing for process "<<remote->first<<std::endl;
        return false;
      }

      if(rIndex->attribute() != oRIndex->attribute()) {
        std::cerr<<"Attribute for entry "<<rIndex->localIndexPair().global()
                 <<" for process "<< remote->first<<" is wrong: "
                 <<rIndex->attribute()<<" != "<<oRIndex->attribute()<<std::endl;
        return false;
      }
    }
  }

  return true;
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

int testIndicesSyncer()
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
  // distributed indexset
  //  typedef ParallelLocalIndex<GridFlags> LocalIndexType;

  typedef Dune::IndexSet<int,Dune::ParallelLocalIndex<GridFlags> > IndexSet;
  IndexSet indexSet, changedIndexSet;

  // Set up the indexsets.
  int start = std::max(rank*nx-1,0);
  int end = std::min((rank + 1) * nx+1, Nx);

  indexSet.beginResize();
  changedIndexSet.beginResize();

  int localIndex=0;

  for(int j=0; j<Ny; j++)
    for(int i=start; i<end; i++) {
      bool isPublic = (i<=start+1)||(i>=end-2);
      GridFlags flag = owner;
      if((i==start && (i!=0))||(i==end-1 && (i!=Nx-1))) {
        flag = overlap;
      }

      indexSet.add(i+j*Nx, Dune::ParallelLocalIndex<GridFlags> (localIndex,flag,isPublic));
      changedIndexSet.add(i+j*Nx, Dune::ParallelLocalIndex<GridFlags> (localIndex++,flag,isPublic));
    }

  indexSet.endResize();
  changedIndexSet.endResize();

  Dune::RemoteIndices<IndexSet> remoteIndices(indexSet, indexSet, MPI_COMM_WORLD);
  Dune::RemoteIndices<IndexSet> changedRemoteIndices(changedIndexSet, changedIndexSet, MPI_COMM_WORLD);

  remoteIndices.rebuild<false>();
  changedRemoteIndices.rebuild<false>();


  std::cout<<"Unchanged: "<<indexSet<<std::endl<<remoteIndices<<std::endl;
  assert(areEqual(indexSet, remoteIndices,changedIndexSet, changedRemoteIndices));

  std::cout<<"Deleting entries!"<<std::endl;

  //if(procs==1)
  //addFakeRemoteIndices(indexSet, changedIndexSet, remoteIndices, changedRemoteIndices);

  deleteOverlapEntries(changedIndexSet, changedRemoteIndices);
  std::cout<<"Changed:   "<<changedIndexSet<<std::endl<<changedRemoteIndices<<std::endl;

  Dune::IndicesSyncer<IndexSet> syncer(changedIndexSet, changedRemoteIndices);
  //  return 0;

  std::cout<<"Syncing!"<<std::endl;

  syncer.sync();

  std::cout<<"Synced:   "<<changedIndexSet<<std::endl<<changedRemoteIndices<<std::endl;
  if( areEqual(indexSet, remoteIndices,changedIndexSet, changedRemoteIndices))
    return 0;
  else
    return 1;

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

void MPI_err_handler(MPI_Comm *comm, int *err_code, ...){
  char *err_string=new char[MPI_MAX_ERROR_STRING];
  int err_length;
  MPI_Error_string(*err_code, err_string, &err_length);
  std::string s(err_string, err_length);
  std::cerr << "An MPI Error ocurred:"<<std::endl<<s<<std::endl;
  delete[] err_string;
  throw MPIError(s, *err_code);
}

int main(int argc, char** argv){
  MPI_Init(&argc, &argv);
  MPI_Errhandler handler;
  MPI_Errhandler_create(MPI_err_handler, &handler);
  MPI_Errhandler_set(MPI_COMM_WORLD, handler);
  int ret = testIndicesSyncer();
  MPI_Finalize();
  exit(ret);
}
