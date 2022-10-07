// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_INDICESSYNCER_HH
#define DUNE_INDICESSYNCER_HH

#include "indexset.hh"
#include "remoteindices.hh"
#include <dune/common/stdstreams.hh>
#include <dune/common/sllist.hh>
#include <cassert>
#include <cmath>
#include <limits>
#include <algorithm>
#include <functional>
#include <map>
#include <tuple>

#if HAVE_MPI
namespace Dune
{
  /** @addtogroup Common_Parallel
   *
   * @{
   */
  /**
   * @file
   * @brief Class for adding missing indices of a distributed index set in a local
   * communication.
   * @author Markus Blatt
   */

  /**
   * @brief Class for recomputing missing indices of a distributed index set.
   *
   * Missing local and remote indices will be added.
   */
  template<typename T>
  class IndicesSyncer
  {
  public:

    /** @brief The type of the index set. */
    typedef T ParallelIndexSet;

    /** @brief The type of the index pair */
    typedef typename ParallelIndexSet::IndexPair IndexPair;

    /** @brief Type of the global index used in the index set. */
    typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

    /** @brief Type of the attribute used in the index set. */
    typedef typename ParallelIndexSet::LocalIndex::Attribute Attribute;

    /**
     * @brief Type of the remote indices.
     */
    typedef Dune::RemoteIndices<ParallelIndexSet> RemoteIndices;

    /**
     * @brief Constructor.
     *
     * The source as well as the target index set of the remote
     * indices have to be the same as the provided index set.
     * @param indexSet The index set with the information
     * of the locally present indices.
     * @param remoteIndices The remoteIndices.
     */
    IndicesSyncer(ParallelIndexSet& indexSet,
                  RemoteIndices& remoteIndices);

    /**
     * @brief Sync the index set.
     *
     * Computes the missing indices in the local and the remote index list and adds them.
     * No global communication is necessary!
     * All indices added to the index will become the local index
     * std::numeric_limits<size_t>::max()
     *
     */
    void sync();

    /**
     * @brief Synce the index set and assign local numbers to new indices
     *
     * Computes the missing indices in the local and the remote index list and adds them.
     * No global communication is necessary!
     * @param numberer Functor providing the local indices for the added global indices.
     * has to provide a function size_t operator()(const TG& global) that provides the
     * local index to a global one. It will be called for ascending global indices.
     *
     */
    template<typename T1>
    void sync(T1& numberer);

  private:

    /** @brief The set of locally present indices.*/
    ParallelIndexSet& indexSet_;

    /** @brief The remote indices. */
    RemoteIndices& remoteIndices_;

    /** @brief The send buffers for the neighbour processes. */
    char** sendBuffers_;

    /** @brief The receive buffer. */
    char* receiveBuffer_;

    /** @brief The size of the send buffers. */
    std::size_t* sendBufferSizes_;

    /** @brief The size of the receive buffer in bytes. */
    int receiveBufferSize_; // int because of MPI

    /**
     * @brief Information about the messages to send to a neighbouring process.
     */
    struct MessageInformation
    {
      MessageInformation()
        : publish(), pairs()
      {}
      /** @brief The number of indices we publish for the other process. */
      int publish;
      /**
       * @brief The number of pairs (attribute and process number)
       * we publish to the neighbour process.
       */
      int pairs;
    };

    /**
     * @brief Default numberer for sync().
     */
    class DefaultNumberer
    {
    public:
      /**
       * @brief Provide the local index, always
       * std::numeric_limits<size_t>::max()
       * @param global The global index (ignored).
       */
      std::size_t operator()([[maybe_unused]] const GlobalIndex& global)
      {
        return std::numeric_limits<size_t>::max();
      }
    };

    /** @brief The mpi datatype for the MessageInformation */
    MPI_Datatype datatype_;

    /** @brief Our rank. */
    int rank_;

    /**
     * @brief List type for temporarily storing the global indices of the
     * remote indices.
     */
    typedef SLList<std::pair<GlobalIndex,Attribute>, typename RemoteIndices::Allocator> GlobalIndexList;

    /** @brief The modifying iterator for the global index list. */
    typedef typename GlobalIndexList::ModifyIterator GlobalIndexModifier;

    /**
     * @brief The type of the iterator of GlobalIndexList
     */
    typedef typename SLList<GlobalIndex, typename RemoteIndices::Allocator>::iterator
    GlobalIndexIterator;

    /** @brief Type of the map of ranks onto GlobalIndexLists. */
    typedef std::map<int, GlobalIndexList> GlobalIndicesMap;

    /**
     * @brief Map of global index lists onto process ranks.
     *
     * As the pointers in the remote index lists become invalid due to
     * resorting the index set entries one has store the corresponding
     * global index for each remote index. Thus the pointers can be adjusted
     * properly as a last step.
     */
    GlobalIndicesMap globalMap_;

    /**
     * @brief The type of the singly linked list of bools.
     */
    typedef SLList<bool, typename RemoteIndices::Allocator> BoolList;

    /**
     * @brief The mutable iterator of the singly linked bool list.
     */
    typedef typename BoolList::iterator BoolIterator;

    /** @brief The type of the modifying iterator for the list of bools. */
    typedef typename BoolList::ModifyIterator BoolListModifier;

    /** @brief The type of the map of bool lists. */
    typedef std::map<int,BoolList> BoolMap;

    /**
     * @brief Map of lists of bool indicating whether the remote index was present before
     * call of sync.
     */
    BoolMap oldMap_;

    /** @brief Information about the messages we send. */
    std::map<int,MessageInformation> infoSend_;

    /** @brief The type of the remote index list. */
    typedef typename RemoteIndices::RemoteIndexList RemoteIndexList;

    /** @brief The tyoe of the modifying iterator of the remote index list. */
    typedef typename RemoteIndexList::ModifyIterator RemoteIndexModifier;

    /** @brief The type of the remote inde. */
    typedef Dune::RemoteIndex<GlobalIndex,Attribute> RemoteIndex;

    /** @brief The iterator of the remote index list. */
    typedef typename RemoteIndexList::iterator RemoteIndexIterator;

    /** @brief The const iterator of the remote index list. */
    typedef typename RemoteIndexList::const_iterator ConstRemoteIndexIterator;

    /** @brief Type of the tuple of iterators needed for the adding of indices. */
    typedef std::tuple<RemoteIndexModifier,GlobalIndexModifier,BoolListModifier,
        const ConstRemoteIndexIterator> IteratorTuple;

    /**
     * @brief A tuple of iterators.
     *
     * Insertion into a singly linked list is only possible at the position after the one of the iterator.
     * Therefore for each linked list two iterators are needed: One position before the actual entry
     * (for insertion) and one positioned at the actual position (for searching).
     */
    class Iterators
    {
      friend class IndicesSyncer<T>;
    public:
      /**
       * @brief Constructor.
       *
       * Initializes all iterator to first entry and the one before the first entry, respectively.
       * @param remoteIndices The list of the remote indices.
       * @param globalIndices The list of the corresponding global indices. This is needed because the
       * the pointers to the local index will become invalid due to the merging of the index sets.
       * @param booleans Whether the remote index was there before the sync process started.
       */
      Iterators(RemoteIndexList& remoteIndices, GlobalIndexList& globalIndices,
                BoolList& booleans);

      /**
       * @brief Default constructor.
       */
      Iterators();

      /**
       * @brief Increment all iteraors.
       */
      Iterators& operator++();

      /**
       * @brief Insert a new remote index to the underlying remote index list.
       * @param index The remote index.
       * @param global The global index corresponding to the remote index.
       */
      void insert(const RemoteIndex& index,
                  const std::pair<GlobalIndex,Attribute>& global);

      /**
       * @brief Get the remote index at current position.
       * @return The current remote index.
       */
      RemoteIndex& remoteIndex() const;

      /**
       * @brief Get the global index of the remote index at current position.
       * @return The current global index.
       */
      std::pair<GlobalIndex,Attribute>& globalIndexPair() const;

      Attribute& attribute() const;

      /**
       * @brief Was this entry already in the remote index list before the sync process?
       * @return True if the current index wasalready in the remote index list
       * before the sync process.
       */
      bool isOld() const;

      /**
       * @brief Reset all the underlying iterators.
       *
       * Position them to first list entry and the entry before the first entry respectively.
       * @param remoteIndices The list of the remote indices.
       * @param globalIndices The list of the corresponding global indices. This is needed because the
       * the pointers to the local index will become invalid due to the merging of the index sets.
       * @param booleans Whether the remote index was there before the sync process started.
       */
      void reset(RemoteIndexList& remoteIndices, GlobalIndexList& globalIndices,
                 BoolList& booleans);

      /**
       * @brief Are we not at the end of the list?
       * @return True if the iterators are not positioned at the end of the list
       * and the tail of the list respectively.
       */
      bool isNotAtEnd() const;

      /**
       * @brief Are we at the end of the list?
       * @return True if the iterators are positioned at the end of the list
       * and the tail of the list respectively.
       */
      bool isAtEnd() const;

    private:
      /**
       * @brief The iterator tuple.
       *
       * The tuple consists of one iterator over a singly linked list of remote indices
       * initially positioned before the first entry, one over a singly linked list of global indices
       * , one over a singly linked list of bool values both positioned at the same entry. The another three
       * iterators of the same type positioned at the first entry. Last an iterator over the
       * singly linked list of remote indices positioned at the end.
       */
      IteratorTuple iterators_;
    };

    /** @brief Type of the map from ranks to iterator tuples. */
    typedef std::map<int,Iterators> IteratorsMap;

    /**
     * @brief The iterator tuples mapped on the neighbours.
     *
     * The key of the map is the rank of the neighbour.
     * The first entry in the tuple is an iterator over the remote indices
     * initially positioned before the first entry. The second entry is an
     * iterator over the corresponding global indices also initially positioned
     * before the first entry. The third entry an iterator over remote indices
     * initially positioned at the beginning. The last entry is the iterator over
     * the remote indices positioned at the end.
     */
    IteratorsMap iteratorsMap_;

    /** @brief Calculates the message sizes to send. */
    void calculateMessageSizes();

    /**
     * @brief Pack and send the message for another process.
     * @param destination The rank of the process we send to.
     * @param buffer The allocated buffer to use.
     * @param bufferSize The size of the buffer.
     * @param req The MPI_Request to setup the nonblocking send.
     */
    void packAndSend(int destination, char* buffer, std::size_t bufferSize, MPI_Request& req);

    /**
     * @brief Recv and unpack the message from another process and add the indices.
     * @param numberer Functor providing local indices for added global indices.
     */
    template<typename T1>
    void recvAndUnpack(T1& numberer);

    /**
     * @brief Register the MPI datatype for the MessageInformation.
     */
    void registerMessageDatatype();

    /**
     * @brief Insert an entry into the  remote index list if not yet present.
     */
    void insertIntoRemoteIndexList(int process,
                                   const std::pair<GlobalIndex,Attribute>& global,
                                   char attribute);

    /**
     * @brief Reset the iterator tuples of all neighbouring processes.
     */
    void resetIteratorsMap();

    /**
     * @brief Check whether the iterator tuples of all neighbouring processes
     * are reset.
     */
    bool checkReset();

    /**
     * @brief Check whether the iterator tuple is reset.
     *
     * @param iterators The iterator tuple to check.
     * @param rlist The SLList of the remote indices.
     * @param gList The SLList of the global indices.
     * @param bList The SLList of the bool values.
     */
    bool checkReset(const Iterators& iterators, RemoteIndexList& rlist, GlobalIndexList& gList,
                    BoolList& bList);
  };

  template<typename TG, typename TA>
  bool operator<(const IndexPair<TG,ParallelLocalIndex<TA> >& i1,
                 const std::pair<TG,TA>& i2)
  {
    return i1.global() < i2.first ||
           (i1.global() == i2.first && i1.local().attribute()<i2.second);
  }

  template<typename TG, typename TA>
  bool operator<(const std::pair<TG,TA>& i1,
                 const IndexPair<TG,ParallelLocalIndex<TA> >& i2)
  {
    return i1.first < i2.global() ||
           (i1.first == i2.global() && i1.second<i2.local().attribute());
  }

  template<typename TG, typename TA>
  bool operator==(const IndexPair<TG,ParallelLocalIndex<TA> >& i1,
                  const std::pair<TG,TA>& i2)
  {
    return (i1.global() == i2.first && i1.local().attribute()==i2.second);
  }

  template<typename TG, typename TA>
  bool operator!=(const IndexPair<TG,ParallelLocalIndex<TA> >& i1,
                  const std::pair<TG,TA>& i2)
  {
    return (i1.global() != i2.first || i1.local().attribute()!=i2.second);
  }

  template<typename TG, typename TA>
  bool operator==(const std::pair<TG,TA>& i2,
                  const IndexPair<TG,ParallelLocalIndex<TA> >& i1)
  {
    return (i1.global() == i2.first && i1.local().attribute()==i2.second);
  }

  template<typename TG, typename TA>
  bool operator!=(const std::pair<TG,TA>& i2,
                  const IndexPair<TG,ParallelLocalIndex<TA> >& i1)
  {
    return (i1.global() != i2.first || i1.local().attribute()!=i2.second);
  }

  /**
   * @brief Stores the corresponding global indices of the remote index information.
   *
   * Whenever a ParallelIndexSet is resized all RemoteIndices that use it will be invalided
   * as the pointers to the index set are invalid after calling ParallelIndexSet::Resize()
   * One can rebuild them by storing the global indices in a map with this function and later
   * repairing the pointers by calling repairLocalIndexPointers.
   *
   * @warning The RemoteIndices class has to be build with the same index set for both the
   * sending and receiving side
   * @param globalMap Map to store the corresponding global indices in.
   * @param remoteIndices The remote index information we need to store the corresponding global
   * indices of.
   * @param indexSet The index set that is for both the sending and receiving side of the remote
   * index information.
   */
  template<typename T, typename A, typename A1>
  void storeGlobalIndicesOfRemoteIndices(std::map<int,SLList<std::pair<typename T::GlobalIndex, typename T::LocalIndex::Attribute>,A> >& globalMap,
                                         const RemoteIndices<T,A1>& remoteIndices)
  {
    for(auto remote = remoteIndices.begin(), end =remoteIndices.end(); remote != end; ++remote) {
      typedef typename RemoteIndices<T,A1>::RemoteIndexList RemoteIndexList;
      typedef SLList<std::pair<typename T::GlobalIndex, typename T::LocalIndex::Attribute>,A> GlobalIndexList;
      GlobalIndexList& global = globalMap[remote->first];
      RemoteIndexList& rList = *(remote->second.first);

      for(auto index = rList.begin(), riEnd = rList.end();
          index != riEnd; ++index) {
        global.push_back(std::make_pair(index->localIndexPair().global(),
                                        index->localIndexPair().local().attribute()));
      }
    }
  }

  /**
   * @brief Repair the pointers to the local indices in the remote indices.
   *
   * @param globalMap The map of the process number to the list of global indices
   * corresponding to the remote index list of the process.
   * @param remoteIndices The known remote indices.
   * @param indexSet The set of local indices of the current process.
   */
  template<typename T, typename A, typename A1>
  inline void repairLocalIndexPointers(std::map<int,
                                           SLList<std::pair<typename T::GlobalIndex,
                                                   typename T::LocalIndex::Attribute>,A> >& globalMap,
                                       RemoteIndices<T,A1>& remoteIndices,
                                       const T& indexSet)
  {
    assert(globalMap.size()==static_cast<std::size_t>(remoteIndices.neighbours()));
    // Repair pointers to index set in remote indices.
    auto global = globalMap.begin();
    auto end = remoteIndices.remoteIndices_.end();

    for(auto remote = remoteIndices.remoteIndices_.begin(); remote != end; ++remote, ++global) {
      assert(remote->first==global->first);
      assert(remote->second.first->size() == global->second.size());

      auto riEnd  = remote->second.first->end();
      auto rIndex = remote->second.first->begin();
      auto gIndex = global->second.begin();
      auto index  = indexSet.begin();

      assert(rIndex==riEnd || gIndex != global->second.end());
      while(rIndex != riEnd) {
        // Search for the index in the set.
        assert(gIndex != global->second.end());

        while(!(index->global() == gIndex->first
                && index->local().attribute() == gIndex->second)) {
          ++index;
          // this is only needed for ALU, where there may exist
          // more entries with the same global index in the remote index set
          // than in the index set
          if (index->global() > gIndex->first) {
            index=indexSet.begin();
          }
        }

        assert(index != indexSet.end() && *index == *gIndex);

        rIndex->localIndex_ = &(*index);
        ++index;
        ++rIndex;
        ++gIndex;
      }
    }
    remoteIndices.sourceSeqNo_ = remoteIndices.source_->seqNo();
    remoteIndices.destSeqNo_ = remoteIndices.target_->seqNo();
  }

  template<typename T>
  IndicesSyncer<T>::IndicesSyncer(ParallelIndexSet& indexSet,
                                  RemoteIndices& remoteIndices)
    : indexSet_(indexSet), remoteIndices_(remoteIndices)
  {
    // index sets must match.
    assert(remoteIndices.source_ == remoteIndices.target_);
    assert(remoteIndices.source_ == &indexSet);
    MPI_Comm_rank(remoteIndices_.communicator(), &rank_);
  }

  template<typename T>
  IndicesSyncer<T>::Iterators::Iterators(RemoteIndexList& remoteIndices,
                                         GlobalIndexList& globalIndices,
                                         BoolList& booleans)
    : iterators_(remoteIndices.beginModify(), globalIndices.beginModify(),
                 booleans.beginModify(), remoteIndices.end())
  { }

  template<typename T>
  IndicesSyncer<T>::Iterators::Iterators()
    : iterators_()
  {}

  template<typename T>
  inline typename IndicesSyncer<T>::Iterators& IndicesSyncer<T>::Iterators::operator++()
  {
    ++(std::get<0>(iterators_));
    ++(std::get<1>(iterators_));
    ++(std::get<2>(iterators_));
    return *this;
  }

  template<typename T>
  inline void IndicesSyncer<T>::Iterators::insert(const RemoteIndex & index,
                                                  const std::pair<GlobalIndex,Attribute>& global)
  {
    std::get<0>(iterators_).insert(index);
    std::get<1>(iterators_).insert(global);
    std::get<2>(iterators_).insert(false);
  }

  template<typename T>
  inline typename IndicesSyncer<T>::RemoteIndex&
  IndicesSyncer<T>::Iterators::remoteIndex() const
  {
    return *(std::get<0>(iterators_));
  }

  template<typename T>
  inline std::pair<typename IndicesSyncer<T>::GlobalIndex,typename IndicesSyncer<T>::Attribute>&
  IndicesSyncer<T>::Iterators::globalIndexPair() const
  {
    return *(std::get<1>(iterators_));
  }

  template<typename T>
  inline bool IndicesSyncer<T>::Iterators::isOld() const
  {
    return *(std::get<2>(iterators_));
  }

  template<typename T>
  inline void IndicesSyncer<T>::Iterators::reset(RemoteIndexList& remoteIndices,
                                                 GlobalIndexList& globalIndices,
                                                 BoolList& booleans)
  {
    std::get<0>(iterators_) = remoteIndices.beginModify();
    std::get<1>(iterators_) = globalIndices.beginModify();
    std::get<2>(iterators_) = booleans.beginModify();
  }

  template<typename T>
  inline bool IndicesSyncer<T>::Iterators::isNotAtEnd() const
  {
    return std::get<0>(iterators_) != std::get<3>(iterators_);
  }

  template<typename T>
  inline bool IndicesSyncer<T>::Iterators::isAtEnd() const
  {
    return std::get<0>(iterators_) == std::get<3>(iterators_);
  }

  template<typename T>
  void IndicesSyncer<T>::registerMessageDatatype()
  {
    MPI_Datatype type[2] = {MPI_INT, MPI_INT};
    int blocklength[2] = {1,1};
    MPI_Aint displacement[2];
    MPI_Aint base;

    // Compute displacement
    MessageInformation message;

    MPI_Get_address( &(message.publish), displacement);
    MPI_Get_address( &(message.pairs), displacement+1);

    // Make the displacement relative
    MPI_Get_address(&message, &base);
    displacement[0] -= base;
    displacement[1] -= base;

    MPI_Type_create_struct( 2, blocklength, displacement, type, &datatype_);
    MPI_Type_commit(&datatype_);
  }

  template<typename T>
  void IndicesSyncer<T>::calculateMessageSizes()
  {
    auto iEnd = indexSet_.end();
    auto collIter = remoteIndices_.template iterator<true>();

    for(auto index = indexSet_.begin(); index != iEnd; ++index) {
      collIter.advance(index->global(), index->local().attribute());
      if(collIter.empty())
        break;
      int knownRemote=0;
      auto end = collIter.end();

      // Count the remote indices we know.
      for(auto valid = collIter.begin(); valid != end; ++valid) {
        ++knownRemote;
      }

      if(knownRemote>0) {
        Dune::dverb<<rank_<<": publishing "<<knownRemote<<" for index "<<index->global()<< " for processes ";

        // Update MessageInformation
        for(auto valid = collIter.begin(); valid != end; ++valid) {
          ++(infoSend_[valid.process()].publish);
          (infoSend_[valid.process()].pairs) += knownRemote;
          Dune::dverb<<valid.process()<<" ";
          Dune::dverb<<"(publish="<<infoSend_[valid.process()].publish<<", pairs="<<infoSend_[valid.process()].pairs
                     <<") ";
        }
        Dune::dverb<<std::endl;
      }
    }

    const auto end = infoSend_.end();

    // Now determine the buffersizes needed for each neighbour using MPI_Pack_size
    MessageInformation dummy;

    auto messageIter= infoSend_.begin();
    const auto rend = remoteIndices_.end();
    int neighbour=0;

    for(auto remote = remoteIndices_.begin(); remote != rend; ++remote, ++neighbour) {
      MessageInformation* message;
      MessageInformation recv;

      if(messageIter != end && messageIter->first==remote->first) {
        // We want to send message information to that process
        message = const_cast<MessageInformation*>(&(messageIter->second));
        ++messageIter;
      }else
        // We do not want to send information but the other process might.
        message = &dummy;

      sendBufferSizes_[neighbour]=0;
      int tsize;
      // The number of indices published
      MPI_Pack_size(1, MPI_INT,remoteIndices_.communicator(), &tsize);
      sendBufferSizes_[neighbour] += tsize;

      for(int i=0; i < message->publish; ++i) {
        // The global index
        MPI_Pack_size(1, MPITraits<GlobalIndex>::getType(), remoteIndices_.communicator(), &tsize);
        sendBufferSizes_[neighbour] += tsize;
        // The attribute in the local index
        MPI_Pack_size(1, MPI_CHAR, remoteIndices_.communicator(), &tsize);
        sendBufferSizes_[neighbour] += tsize;
        // The number of corresponding remote indices
        MPI_Pack_size(1, MPI_INT, remoteIndices_.communicator(), &tsize);
        sendBufferSizes_[neighbour] += tsize;
      }
      for(int i=0; i < message->pairs; ++i) {
        // The process of the remote index
        MPI_Pack_size(1, MPI_INT, remoteIndices_.communicator(), &tsize);
        sendBufferSizes_[neighbour] += tsize;
        // The attribute of the remote index
        MPI_Pack_size(1, MPI_CHAR, remoteIndices_.communicator(), &tsize);
        sendBufferSizes_[neighbour] += tsize;
      }

      Dune::dverb<<rank_<<": Buffer (neighbour="<<remote->first<<") size is "<< sendBufferSizes_[neighbour]<<" for publish="<<message->publish<<" pairs="<<message->pairs<<std::endl;
    }

  }

  template<typename T>
  inline void IndicesSyncer<T>::sync()
  {
    DefaultNumberer numberer;
    sync(numberer);
  }

  template<typename T>
  template<typename T1>
  void IndicesSyncer<T>::sync(T1& numberer)
  {
    // The pointers to the local indices in the remote indices
    // will become invalid due to the resorting of the index set.
    // Therefore store the corresponding global indices.
    // Mark all indices as not added
    const auto end = remoteIndices_.end();

    // Number of neighbours might change during the syncing.
    // save the old neighbours
    std::size_t noOldNeighbours = remoteIndices_.neighbours();
    int* oldNeighbours = new int[noOldNeighbours];
    sendBufferSizes_ = new std::size_t[noOldNeighbours];
    std::size_t neighbourI = 0;

    for(auto remote = remoteIndices_.begin(); remote != end; ++remote, ++neighbourI) {
      oldNeighbours[neighbourI] = remote->first;

      // Make sure we only have one remote index list.
      assert(remote->second.first==remote->second.second);

      RemoteIndexList& rList = *(remote->second.first);

      // Store the corresponding global indices.
      GlobalIndexList& global = globalMap_[remote->first];
      BoolList& added = oldMap_[remote->first];
      auto riEnd = rList.end();

      for(auto index = rList.begin();
          index != riEnd; ++index) {
        global.push_back(std::make_pair(index->localIndexPair().global(),
                                        index->localIndexPair().local().attribute()));
        added.push_back(true);
      }

      Iterators iterators(rList, global, added);
      iteratorsMap_.insert(std::make_pair(remote->first, iterators));
      assert(checkReset(iteratorsMap_[remote->first], rList,global,added));
    }

    // Exchange indices with each neighbour
    calculateMessageSizes();

    // Allocate the buffers
    receiveBufferSize_=1;
    sendBuffers_ = new char*[noOldNeighbours];

    for(std::size_t i=0; i<noOldNeighbours; ++i) {
      sendBuffers_[i] = new char[sendBufferSizes_[i]];
      receiveBufferSize_ = std::max(receiveBufferSize_, static_cast<int>(sendBufferSizes_[i]));
    }

    receiveBuffer_=new char[receiveBufferSize_];

    indexSet_.beginResize();

    Dune::dverb<<rank_<<": Neighbours: ";

    for(std::size_t i = 0; i<noOldNeighbours; ++i)
      Dune::dverb<<oldNeighbours[i]<<" ";

    Dune::dverb<<std::endl;

    MPI_Request* requests = new MPI_Request[noOldNeighbours];
    MPI_Status* statuses = new MPI_Status[noOldNeighbours];

    // Pack Message data and start the sends
    for(std::size_t i = 0; i<noOldNeighbours; ++i)
      packAndSend(oldNeighbours[i], sendBuffers_[i], sendBufferSizes_[i], requests[i]);

    // Probe for incoming messages, receive and unpack them
    for(std::size_t i = 0; i<noOldNeighbours; ++i)
      recvAndUnpack(numberer);
    //       }else{
    //  recvAndUnpack(oldNeighbours[i], numberer);
    //  packAndSend(oldNeighbours[i]);
    //       }
    //     }

    delete[] receiveBuffer_;

    // Wait for the completion of the sends
    // Wait for completion of sends
    if(MPI_SUCCESS!=MPI_Waitall(noOldNeighbours, requests, statuses)) {
      std::cerr<<": MPI_Error occurred while sending message"<<std::endl;
      for(std::size_t i=0; i< noOldNeighbours; i++)
        if(MPI_SUCCESS!=statuses[i].MPI_ERROR)
          std::cerr<<"Destination "<<statuses[i].MPI_SOURCE<<" error code: "<<statuses[i].MPI_ERROR<<std::endl;
    }

    delete[] statuses;
    delete[] requests;

    for(std::size_t i=0; i<noOldNeighbours; ++i)
      delete[] sendBuffers_[i];

    delete[] sendBuffers_;
    delete[] sendBufferSizes_;

    // No need for the iterator tuples any more
    iteratorsMap_.clear();

    indexSet_.endResize();

    delete[] oldNeighbours;

    repairLocalIndexPointers(globalMap_, remoteIndices_, indexSet_);

    oldMap_.clear();
    globalMap_.clear();

    // update the sequence number
    remoteIndices_.sourceSeqNo_ = remoteIndices_.destSeqNo_ = indexSet_.seqNo();
  }

  template<typename T>
  void IndicesSyncer<T>::packAndSend(int destination, char* buffer, std::size_t bufferSize, MPI_Request& request)
  {
    auto iEnd       = indexSet_.end();
    int bpos       = 0;
    int published  = 0;
    int pairs      = 0;

    assert(checkReset());

    // Pack the number of indices we publish
    MPI_Pack(&(infoSend_[destination].publish), 1, MPI_INT, buffer, bufferSize, &bpos,
             remoteIndices_.communicator());

    for(auto index = indexSet_.begin(); index != iEnd; ++index) {
      // Search for corresponding remote indices in all iterator tuples
      auto iteratorsEnd = iteratorsMap_.end();

      // advance all iterators to a position with global index >= index->global()
      for(auto iterators = iteratorsMap_.begin(); iteratorsEnd != iterators; ++iterators) {
        while(iterators->second.isNotAtEnd() &&
              iterators->second.globalIndexPair().first < index->global())
          ++(iterators->second);
        assert(!iterators->second.isNotAtEnd() || iterators->second.globalIndexPair().first >= index->global());
      }

      // Add all remote indices positioned at global which were already present before calling sync
      // to the message.
      // Count how many remote indices we will send
      int indices = 0;
      bool knownRemote = false; // Is the remote process supposed to know this index?

      for(auto iterators = iteratorsMap_.begin(); iteratorsEnd != iterators; ++iterators)
      {
        std::pair<GlobalIndex,Attribute> p;
        if (iterators->second.isNotAtEnd())
        {
          p = iterators->second.globalIndexPair();
        }

        if(iterators->second.isNotAtEnd() && iterators->second.isOld()
           && iterators->second.globalIndexPair().first == index->global()) {
          indices++;
          if(destination == iterators->first)
            knownRemote = true;
        }
      }

      if(!knownRemote)
        // We do not need to send any indices
        continue;

      Dune::dverb<<rank_<<": sending "<<indices<<" for index "<<index->global()<<" to "<<destination<<std::endl;


      // Pack the global index, the attribute and the number
      MPI_Pack(const_cast<GlobalIndex*>(&(index->global())), 1, MPITraits<GlobalIndex>::getType(), buffer, bufferSize, &bpos,
               remoteIndices_.communicator());

      char attr = index->local().attribute();
      MPI_Pack(&attr, 1, MPI_CHAR, buffer, bufferSize, &bpos,
               remoteIndices_.communicator());

      // Pack the number of remote indices we send.
      MPI_Pack(&indices, 1, MPI_INT, buffer, bufferSize, &bpos,
               remoteIndices_.communicator());

      // Pack the information about the remote indices
      for(auto iterators = iteratorsMap_.begin(); iteratorsEnd != iterators; ++iterators)
        if(iterators->second.isNotAtEnd() && iterators->second.isOld()
           && iterators->second.globalIndexPair().first == index->global()) {
          int process = iterators->first;

          ++pairs;
          assert(pairs <= infoSend_[destination].pairs);
          MPI_Pack(&process, 1, MPI_INT, buffer, bufferSize, &bpos,
                   remoteIndices_.communicator());
          char attr2 = iterators->second.remoteIndex().attribute();

          MPI_Pack(&attr2, 1, MPI_CHAR, buffer, bufferSize, &bpos,
                   remoteIndices_.communicator());
          --indices;
        }
      assert(indices==0);
      ++published;
      Dune::dvverb<<" (publish="<<published<<", pairs="<<pairs<<")"<<std::endl;
      assert(published <= infoSend_[destination].publish);
    }

    // Make sure we send all expected entries
    assert(published == infoSend_[destination].publish);
    assert(pairs == infoSend_[destination].pairs);
    resetIteratorsMap();

    Dune::dverb << rank_<<": Sending message of "<<bpos<<" bytes to "<<destination<<std::endl;

    MPI_Issend(buffer, bpos, MPI_PACKED, destination, 345, remoteIndices_.communicator(),&request);
  }

  template<typename T>
  inline void IndicesSyncer<T>::insertIntoRemoteIndexList(int process,
                                                          const std::pair<GlobalIndex,Attribute>& globalPair,
                                                          char attribute)
  {
    Dune::dverb<<"Inserting from "<<process<<" "<<globalPair.first<<", "<<
    globalPair.second<<" "<<attribute<<std::endl;

    resetIteratorsMap();

    // There might be cases where there no remote indices for that process yet
    typename IteratorsMap::iterator found = iteratorsMap_.find(process);

    if( found == iteratorsMap_.end() ) {
      Dune::dverb<<"Discovered new neighbour "<<process<<std::endl;
      RemoteIndexList* rlist = new RemoteIndexList();
      remoteIndices_.remoteIndices_.insert(std::make_pair(process,std::make_pair(rlist,rlist)));
      Iterators iterators = Iterators(*rlist, globalMap_[process], oldMap_[process]);
      found = iteratorsMap_.insert(std::make_pair(process, iterators)).first;
    }

    Iterators& iterators = found->second;

    // Search for the remote index
    while(iterators.isNotAtEnd() && iterators.globalIndexPair() < globalPair) {
      // Increment all iterators
      ++iterators;

    }

    if(iterators.isAtEnd() || iterators.globalIndexPair() != globalPair) {
      // The entry is not yet known
      // Insert in the list and do not change the first iterator.
      iterators.insert(RemoteIndex(Attribute(attribute)),globalPair);
      return;
    }

    // Global indices match
    bool indexIsThere=false;
    for(Iterators tmpIterators = iterators;
        !tmpIterators.isAtEnd() && tmpIterators.globalIndexPair() == globalPair;
        ++tmpIterators)
      //entry already exists with the same attribute
      if(tmpIterators.globalIndexPair().second == attribute) {
        indexIsThere=true;
        break;
      }

    if(!indexIsThere)
      // The entry is not yet known
      // Insert in the list and do not change the first iterator.
      iterators.insert(RemoteIndex(Attribute(attribute)),globalPair);
  }

  template<typename T>
  template<typename T1>
  void IndicesSyncer<T>::recvAndUnpack(T1& numberer)
  {
    const ParallelIndexSet& constIndexSet = indexSet_;
    auto iEnd   = constIndexSet.end();
    auto index  = constIndexSet.begin();
    int bpos   = 0;
    int publish;

    assert(checkReset());

    MPI_Status status;

    // We have to determine the message size and source before the receive
    MPI_Probe(MPI_ANY_SOURCE, 345, remoteIndices_.communicator(), &status);

    int source=status.MPI_SOURCE;
    int count;
    MPI_Get_count(&status, MPI_PACKED, &count);

    Dune::dvverb<<rank_<<": Receiving message from "<< source<<" with "<<count<<" bytes"<<std::endl;

    if(count>receiveBufferSize_) {
      receiveBufferSize_=count;
      delete[] receiveBuffer_;
      receiveBuffer_ = new char[receiveBufferSize_];
    }

    MPI_Recv(receiveBuffer_, count, MPI_PACKED, source, 345, remoteIndices_.communicator(), &status);

    // How many global entries were published?
    MPI_Unpack(receiveBuffer_,  count, &bpos, &publish, 1, MPI_INT, remoteIndices_.communicator());

    // Now unpack the remote indices and add them.
    while(publish>0) {

      // Unpack information about the local index on the source process
      GlobalIndex global;           // global index of the current entry
      char sourceAttribute; // Attribute on the source process
      int pairs;

      MPI_Unpack(receiveBuffer_,  count, &bpos, &global, 1, MPITraits<GlobalIndex>::getType(),
                 remoteIndices_.communicator());
      MPI_Unpack(receiveBuffer_,  count, &bpos, &sourceAttribute, 1, MPI_CHAR,
                 remoteIndices_.communicator());
      MPI_Unpack(receiveBuffer_,  count, &bpos, &pairs, 1, MPI_INT,
                 remoteIndices_.communicator());

      // Insert the entry on the remote process to our
      // remote index list
      SLList<std::pair<int,Attribute> > sourceAttributeList;
      sourceAttributeList.push_back(std::make_pair(source,Attribute(sourceAttribute)));
#ifndef NDEBUG
      bool foundSelf = false;
#endif
      Attribute myAttribute=Attribute();

      // Unpack the remote indices
      for(; pairs>0; --pairs) {
        // Unpack the process id that knows the index
        int process;
        char attribute;
        MPI_Unpack(receiveBuffer_,  count, &bpos, &process, 1, MPI_INT,
                   remoteIndices_.communicator());
        // Unpack the attribute
        MPI_Unpack(receiveBuffer_,  count, &bpos, &attribute, 1, MPI_CHAR,
                   remoteIndices_.communicator());

        if(process==rank_) {
#ifndef NDEBUG
          foundSelf=true;
#endif
          myAttribute=Attribute(attribute);
          // Now we know the local attribute of the global index
          //Only add the index if it is unknown.
          // Do we know that global index already?
          auto pos = std::lower_bound(index, iEnd, IndexPair(global));

          if(pos == iEnd || pos->global() != global) {
            // no entry with this global index
            indexSet_.add(global,
                          ParallelLocalIndex<Attribute>(numberer(global),
                                                        myAttribute, true));
            Dune::dvverb << "Adding "<<global<<" "<<myAttribute<<std::endl;
            continue;
          }

          // because of above the global indices match. Add only if the attribute is different
          bool indexIsThere = false;
          index=pos;

          for(; pos->global()==global; ++pos)
            if(pos->local().attribute() == myAttribute) {
              Dune::dvverb<<"found "<<global<<" "<<myAttribute<<std::endl;
              indexIsThere = true;
              break;
            }

          if(!indexIsThere) {
            indexSet_.add(global,
                          ParallelLocalIndex<Attribute>(numberer(global),
                                                        myAttribute, true));
            Dune::dvverb << "Adding "<<global<<" "<<myAttribute<<std::endl;
          }

        }else{
          sourceAttributeList.push_back(std::make_pair(process,Attribute(attribute)));
        }
      }
      assert(foundSelf);
      // Insert remote indices
      typedef typename SLList<std::pair<int,Attribute> >::const_iterator Iter;
      for(Iter i=sourceAttributeList.begin(), end=sourceAttributeList.end();
          i!=end; ++i)
        insertIntoRemoteIndexList(i->first, std::make_pair(global, myAttribute),
                                  i->second);
      --publish;
    }

    resetIteratorsMap();
  }

  template<typename T>
  void IndicesSyncer<T>::resetIteratorsMap(){

    // Reset iterators in all tuples.
    const auto remoteEnd = remoteIndices_.remoteIndices_.end();
    auto iterators = iteratorsMap_.begin();
    auto global = globalMap_.begin();
    auto added = oldMap_.begin();

    for(auto remote = remoteIndices_.remoteIndices_.begin();
        remote != remoteEnd; ++remote, ++global, ++added, ++iterators) {
      iterators->second.reset(*(remote->second.first), global->second, added->second);
    }
  }

  template<typename T>
  bool IndicesSyncer<T>::checkReset(const Iterators& iterators, RemoteIndexList& rList, GlobalIndexList& gList,
                                    BoolList& bList){

    if(std::get<0>(iterators.iterators_) != rList.begin())
      return false;
    if(std::get<1>(iterators.iterators_) != gList.begin())
      return false;
    if(std::get<2>(iterators.iterators_) != bList.begin())
      return false;
    return true;
  }


  template<typename T>
  bool IndicesSyncer<T>::checkReset(){

    // Reset iterators in all tuples.
    const auto remoteEnd = remoteIndices_.remoteIndices_.end();
    auto iterators = iteratorsMap_.begin();
    auto global = globalMap_.begin();
    auto added = oldMap_.begin();
    bool ret = true;

    for(auto remote = remoteIndices_.remoteIndices_.begin();
        remote != remoteEnd; ++remote, ++global, ++added, ++iterators) {
      if(!checkReset(iterators->second, *(remote->second.first), global->second,
                     added->second))
        ret=false;
    }
    return ret;
  }
}

#endif
#endif
