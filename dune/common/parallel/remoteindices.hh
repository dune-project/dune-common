// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_REMOTEINDICES_HH
#define DUNE_REMOTEINDICES_HH

#if HAVE_MPI

#include <cassert>
#include <iostream>
#include <ostream>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <mpi.h>

#include <dune/common/exceptions.hh>
#include <dune/common/parallel/indexset.hh>
#include <dune/common/parallel/mpitraits.hh>
#include <dune/common/parallel/plocalindex.hh>
#include <dune/common/sllist.hh>
#include <dune/common/stdstreams.hh>

namespace Dune {
  /** @addtogroup Common_Parallel
   *
   * @{
   */
  /**
   * @file
   * @brief Classes describing a distributed indexset
   * @author Markus Blatt
   */

  //! \todo Please doc me!
  template<typename TG, typename TA>
  class MPITraits<IndexPair<TG,ParallelLocalIndex<TA> > >
  {
  public:
    inline static MPI_Datatype getType();
  private:
    static MPI_Datatype type;
  };


  template<typename T, typename A>
  class RemoteIndices;

  template<typename T1, typename T2>
  class RemoteIndex;

  // forward declaration needed for friend declaration.
  template<typename T>
  class IndicesSyncer;

  template<typename T1, typename T2>
  std::ostream& operator<<(std::ostream& os, const RemoteIndex<T1,T2>& index);


  template<typename T, typename A, bool mode>
  class RemoteIndexListModifier;


  /**
   * @brief Information about an index residing on another processor.
   */
  template<typename T1, typename T2>
  class RemoteIndex
  {
    template<typename T>
    friend class IndicesSyncer;

    template<typename T, typename A, typename A1>
    friend void repairLocalIndexPointers(std::map<int,SLList<std::pair<typename T::GlobalIndex, typename T::LocalIndex::Attribute>,A> >&,
                                         RemoteIndices<T,A1>&,
                                         const T&);

    template<typename T, typename A, bool mode>
    friend class RemoteIndexListModifier;

  public:
    /**
     * @brief the type of the global index.
     * This type has to provide at least a operator&lt; for sorting.
     */
    typedef T1 GlobalIndex;
    /**
     * @brief The type of the attributes.
     * Normally this will be an enumeration like
     * \code
     * enum Attributes{owner, border, overlap}
     * \endcode
     * e.g. OwnerOverlapCopyAttributes.
     */
    typedef T2 Attribute;

    /**
     * @brief The type of the index pair.
     */
    typedef IndexPair<GlobalIndex,ParallelLocalIndex<Attribute> >
    PairType;

    /**
     * @brief Get the attribute of the index on the remote process.
     * @return The remote attribute.
     */
    const Attribute attribute() const;

    /**
     * @brief Get the corresponding local index pair.
     * @return The corresponding local index pair.
     */

    const PairType& localIndexPair() const;

    /**
     * @brief Parameterless Constructor.
     */
    RemoteIndex();


    /**
     * @brief Constructor.
     * @param attribute The attribute of the index on the remote processor.
     * @param local The corresponding local index.
     */
    RemoteIndex(const T2& attribute,
                const PairType* local);


    /**
     * @brief Constructor.
     * Private as it should only be called from within Indexset.
     * @param attribute The attribute of the index on the remote processor.
     */
    RemoteIndex(const T2& attribute);

    bool operator==(const RemoteIndex& ri) const;

    bool operator!=(const RemoteIndex& ri) const;
  private:
    /** @brief The corresponding local index for this process. */
    const PairType* localIndex_;

    /** @brief The attribute of the index on the other process. */
    char attribute_;
  };

  template<class T, class A>
  std::ostream& operator<<(std::ostream& os, const RemoteIndices<T,A>& indices);

  class InterfaceBuilder;

  template<class T, class A>
  class CollectiveIterator;

  // forward declaration needed for friend declaration.
  template<class T>
  class IndicesSyncer;

  // forward declaration needed for friend declaration.
  template<typename T1, typename T2>
  class OwnerOverlapCopyCommunication;


  /**
   * @brief The indices present on remote processes.
   *
   * To set up communication between the set of processes active in
   * the communication every process needs to know which
   * indices are also known to other processes and which attributes
   * are attached to them on the remote side.
   *
   * This information is managed by this class. The information can either
   * be computed automatically calling rebuild (which requires information
   * to be sent in a ring) or set up by hand using the
   * RemoteIndexListModifiers returned by function getModifier(int).
   *
   * @tparam T The type of the underlying index set.
   * @tparam A The type of the allocator to use.
   */
  template<class T, class A=std::allocator<RemoteIndex<typename T::GlobalIndex,
              typename T::LocalIndex::Attribute> > >
  class RemoteIndices
  {
    friend class InterfaceBuilder;
    friend class IndicesSyncer<T>;
    template<typename T1, typename A2, typename A1>
    friend void repairLocalIndexPointers(std::map<int,SLList<std::pair<typename T1::GlobalIndex, typename T1::LocalIndex::Attribute>,A2> >&,
                                         RemoteIndices<T1,A1>&,
                                         const T1&);

    template<class G, class T1, class T2>
    friend void fillIndexSetHoles(const G& graph, Dune::OwnerOverlapCopyCommunication<T1,T2>& oocomm);
    friend std::ostream& operator<<<>(std::ostream&, const RemoteIndices<T>&);

  public:

    /**
     * @brief Type of the index set we use, e.g. ParallelLocalIndexSet.
     */
    typedef T ParallelIndexSet;

    /**
     * @brief The type of the collective iterator over all remote indices. */
    typedef CollectiveIterator<T,A> CollectiveIteratorT;

    /**
     * @brief The type of the global index.
     */
    typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;


    /**
     * @brief The type of the local index.
     */
    typedef typename ParallelIndexSet::LocalIndex LocalIndex;

    /**
     * @brief The type of the attribute.
     */
    typedef typename LocalIndex::Attribute Attribute;

    /**
     * @brief Type of the remote indices we manage.
     */
    typedef Dune::RemoteIndex<GlobalIndex,Attribute> RemoteIndex;


    /**
     * @brief The type of the allocator for the remote index list.
     */
    using Allocator = typename std::allocator_traits<A>::template rebind_alloc<RemoteIndex>;

    /** @brief The type of the remote index list. */
    typedef Dune::SLList<RemoteIndex,Allocator>
    RemoteIndexList;

    /** @brief The type of the map from rank to remote index list. */
    typedef std::map<int, std::pair<RemoteIndexList*,RemoteIndexList*> >
    RemoteIndexMap;

    typedef typename RemoteIndexMap::const_iterator const_iterator;

    /**
     * @brief Constructor.
     * @param comm The communicator to use.
     * @param source The indexset which represents the global to
     * local mapping at the source of the communication
     * @param destination The indexset to which the communication
     * which represents the global to
     * local mapping at the destination of the communication.
     * May be the same as the source indexset.
     * @param neighbours Optional: The neighbours the process shares indices with.
     * If this parameter is omitted a ring communication with all indices will take
     * place to calculate this information which is O(P).
     * @param includeSelf If true, sending from indices of the processor to other
     * indices on the same processor is enabled even if the same indexset is used
     * on both the
     * sending and receiving side.
     */
    inline RemoteIndices(const ParallelIndexSet& source, const ParallelIndexSet& destination,
                         const MPI_Comm& comm, const std::vector<int>& neighbours=std::vector<int>(), bool includeSelf=false);

    RemoteIndices();

    /**
     * @brief Tell whether sending from indices of the processor to other
     * indices on the same processor is enabled even if the same indexset is
     * used on both the sending and receiving side.
     *
     * @param includeSelf If true it is enabled.
     */
    void setIncludeSelf(bool includeSelf);

    /**
     * @brief Set the index sets and communicator we work with.
     *
     * @warning All remote indices already setup will be deleted!
     *
     * @param comm The communicator to use.
     * @param source The indexset which represents the global to
     * local mapping at the source of the communication
     * @param destination The indexset to which the communication
     * which represents the global to
     * local mapping at the destination of the communication.
     * May be the same as the source indexset.
     * @param neighbours Optional: The neighbours the process shares indices with.
     * If this parameter is omitted a ring communication with all indices will take
     * place to calculate this information which is O(P).
     */
    void setIndexSets(const ParallelIndexSet& source, const ParallelIndexSet& destination,
                      const MPI_Comm& comm, const std::vector<int>& neighbours=std::vector<int>());

    template<typename C>
    void setNeighbours(const C& neighbours)
    {
      neighbourIds.clear();
      neighbourIds.insert(neighbours.begin(), neighbours.end());

    }

    const std::set<int>& getNeighbours() const
    {
      return neighbourIds;
    }

    /**
     * @brief Destructor.
     */
    ~RemoteIndices();

    /**
     * @brief Rebuilds the set of remote indices.
     *
     * This has to be called whenever the underlying index sets
     * change.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     */
    template<bool ignorePublic>
    void rebuild();

    bool operator==(const RemoteIndices& ri) const;

    /**
     * @brief Checks whether the remote indices are synced with
     * the indexsets.
     *
     * If they are not synced the remote indices need to be rebuild.
     * @return True if they are synced.
     */
    inline bool isSynced() const;

    /**
     * @brief Get the mpi communicator used.
     */
    inline MPI_Comm communicator() const;

    /**
     * @brief Get a modifier for a remote index list.
     *
     * Sometimes the user knows in advance which indices will be present
     * on other processors, too. Then he can set them up using this modifier.
     *
     * @warning Use with care. If the remote index list is inconsistent
     * after the modification the communication might result in a dead lock!
     *
     * @tparam mode If true the index set corresponding to the remote indices might get modified.
     * Therefore the internal pointers to the indices need to be repaired.
     * @tparam send If true the remote index information at the sending side will
     * be modified, if false the receiving side.
     */
    template<bool mode, bool send>
    inline RemoteIndexListModifier<T,A,mode> getModifier(int process);

    /**
     * @brief Find an iterator over the remote index lists of a specific process.
     * @param proc The identifier of the process.
     * @return The iterator the remote index lists positioned at the process.
     * If there's is no list for this process, the end iterator is returned.
     */
    inline const_iterator find(int proc) const;

    /**
     * @brief Get an iterator over all remote index lists.
     * @return The iterator over all remote index lists positioned at the first process.
     */
    inline const_iterator begin() const;

    /**
     * @brief Get an iterator over all remote index lists.
     * @return The iterator over all remote index lists positioned at the end.
     */
    inline const_iterator end() const;

    /**
     * @brief Get an iterator for colletively iterating over the remote indices of all remote processes.
     */
    template<bool send>
    inline CollectiveIteratorT iterator() const;

    /**
     * @brief Free the index lists.
     */
    inline void free();

    /**
     * @brief Get the number of processors we share indices with.
     * @return The number of neighbours.
     */
    inline int neighbours() const;

    /** @brief Get the index set at the source. */
    inline const ParallelIndexSet& sourceIndexSet() const;

    /** @brief Get the index set at destination. */
    inline const ParallelIndexSet& destinationIndexSet() const;

  private:
    /** copying is forbidden. */
    RemoteIndices(const RemoteIndices&) = delete;

    /** @brief Index set used at the source of the communication. */
    const ParallelIndexSet* source_;

    /** @brief Index set used at the destination of the communication. */
    const ParallelIndexSet* target_;

    /** @brief The communicator to use.*/
    MPI_Comm comm_;

    /** @brief The neighbours we share indices with.
     * If not empty this will speedup rebuild. */
    std::set<int> neighbourIds;

    /** @brief The communicator tag to use. */
    const static int commTag_=333;

    /**
     * @brief The sequence number of the source index set when the remote indices
     * where build.
     */
    int sourceSeqNo_;

    /**
     * @brief The sequence number of the destination index set when the remote indices
     * where build.
     */
    int destSeqNo_;

    /**
     * @brief Whether the public flag was ignored during the build.
     */
    bool publicIgnored;

    /**
     * @brief Whether the next build will be the first build ever.
     */
    bool firstBuild;

    /*
     * @brief If true, sending from indices of the processor to other
     * indices on the same processor is enabled even if the same indexset is used
     * on both the
     * sending and receiving side.
     */
    bool includeSelf;

    /** @brief The index pair type. */
    typedef IndexPair<GlobalIndex, LocalIndex>
    PairType;

    /**
     * @brief The remote indices.
     *
     * The key is the process id and the values are the pair of remote
     * index lists, the first for receiving, the second for sending.
     */
    RemoteIndexMap remoteIndices_;

    /**
     * @brief Build the remote mapping.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     * @param includeSelf If true, sending from indices of the processor to other
     * indices on the same processor is enabled even if the same indexset is used
     * on both the
     * sending and receiving side.
     */
    template<bool ignorePublic>
    inline void buildRemote(bool includeSelf);

    /**
     * @brief Count the number of public indices in an index set.
     * @param indexSet The index set whose indices we count.
     * @return the number of indices marked as public.
     */
    inline int noPublic(const ParallelIndexSet& indexSet);

    /**
     * @brief Pack the indices to send if source_ and target_ are the same.
     *
     * If the template parameter ignorePublic is true all indices will be treated
     * as public.
     * @param myPairs Array to store references to the public indices in.
     * @param p_out The output buffer to pack the entries to.
     * @param type The mpi datatype for the pairs.
     * @param bufferSize The size of the output buffer p_out.
     * @param position The position to start packing.
     */
    template<bool ignorePublic>
    inline void packEntries(PairType** myPairs, const ParallelIndexSet& indexSet,
                            char* p_out, MPI_Datatype type, int bufferSize,
                            int* position, int n);

    /**
     * @brief unpacks the received indices and builds the remote index list.
     *
     * @param remote The list to add the indices to.
     * @param remoteEntries The number of remote entries to unpack.
     * @param local The local indices to check whether we know the remote
     *              indices.
     * @param localEntries The number of local indices.
     * @param type The mpi data type for unpacking.
     * @param p_in The input buffer to unpack from.
     * @param position The position in the buffer to start unpacking from.
     * @param bufferSize The size of the input buffer.
     */
    inline void unpackIndices(RemoteIndexList& remote, int remoteEntries,
                              PairType** local, int localEntries, char* p_in,
                              MPI_Datatype type, int* position, int bufferSize,
                              bool fromOurself);

    inline void unpackIndices(RemoteIndexList& send, RemoteIndexList& receive,
                              int remoteEntries, PairType** localSource,
                              int localSourceEntries, PairType** localDest,
                              int localDestEntries, char* p_in,
                              MPI_Datatype type, int* position, int bufferSize);

    void unpackCreateRemote(char* p_in, PairType** sourcePairs, PairType** DestPairs,
                            int remoteProc,  int sourcePublish, int destPublish,
                            int bufferSize, bool sendTwo, bool fromOurSelf=false);
  };

  /** @} */

  /**
   * @brief Modifier for adding and/or deleting remote indices from
   * the remote index list.
   *
   * In some cases all the information about the indices also present
   * on remote process might already be known. In this case this
   * information can be provided to the RemoteIndices via this modifier.
   * This prevents the global communication needed by a call to
   * RemoteIndices::rebuild.
   *
   * In some cases it might advisable to run IndicesSyncer::sync afterwards.
   *
   * @warning Use with care. If the indices are not consistent afterwards
   * communication attempts might deadlock!
   */
  template<class T, class A, bool mode>
  class RemoteIndexListModifier
  {

    template<typename T1, typename A1>
    friend class RemoteIndices;

  public:
    class InvalidPosition : public RangeError
    {};

    /**
     * @brief If true the index set corresponding to the
     * remote indices might get modified.
     *
     * If for example new indices are added to an index set
     * all pointers of the remote indices to the local indices
     * become invalid after ParallelIndexSet::endResize() was called.
     */
    constexpr static bool MODIFYINDEXSET = mode;

    /**
     * @brief Type of the index set we use.
     */
    typedef T ParallelIndexSet;

    /**
     * @brief The type of the global index.
     */
    typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

    /**
     * @brief The type of the local index.
     */
    typedef typename ParallelIndexSet::LocalIndex LocalIndex;

    /**
     * @brief The type of the attribute.
     */
    typedef typename LocalIndex::Attribute Attribute;

    /**
     * @brief Type of the remote indices we manage.
     */
    typedef Dune::RemoteIndex<GlobalIndex,Attribute> RemoteIndex;

    /**
     * @brief The type of the allocator for the remote index list.
     */
    typedef A Allocator;

    /** @brief The type of the remote index list. */
    typedef Dune::SLList<RemoteIndex,Allocator>
    RemoteIndexList;

    /**
     * @brief The type of the modifying iterator of the remote index list.
     */
    typedef SLListModifyIterator<RemoteIndex,Allocator> ModifyIterator;

    /**
     * @brief The type of the remote index list iterator.
     */
    typedef typename RemoteIndexList::const_iterator ConstIterator;

    /**
     * @brief Insert an index to the list.
     *
     * Moves to the position where the index fits and inserts it.
     * After the insertion only indices with an bigger global index
     * than the inserted can be inserted.
     *
     * This method is only available if MODIFYINDEXSET is false.
     *
     * @param index The index to insert.
     * @exception InvalidPosition Thrown if the index at the current position or
     * the one before has bigger global index than the one to be inserted.
     */
    void insert(const RemoteIndex& index);


    /**
     * @brief Insert an index to the list.
     *
     * Moves to the position where the index fits and inserts it.
     * After the insertion only indices with an bigger global index
     * than the inserted can be inserted.
     *
     * This method is only available if MODIFYINDEXSET is true.
     *
     * @param index The index to insert.
     * @param global The global index of the remote index.
     * @exception InvalidPosition Thrown if the index at the current position or
     * the one before has bigger global index than the one to be inserted.
     */
    void insert(const RemoteIndex& index, const GlobalIndex& global);

    /**
     * @brief Remove a remote index.
     * @param global The global index corresponding to the remote index.
     * @return True If there was a corresponding remote index.
     * @exception InvalidPostion If there was an insertion or deletion of
     * a remote index corresponding to a bigger global index before.
     */
    bool remove(const GlobalIndex& global);

    /**
     * @brief Repair the pointers to the local index pairs.
     *
     * Due to adding new indices or/and deleting indices in the
     * index set all pointers to the local index pair might become
     * invalid during ParallelIndexSet::endResize().
     * This method repairs them.
     *
     * @exception InvalidIndexSetState Thrown if the underlying
     * index set is not in ParallelIndexSetState::GROUND mode (only when
     * compiled with DUNE_ISTL_WITH_CHECKING!).
     */
    void repairLocalIndexPointers();


    RemoteIndexListModifier(const RemoteIndexListModifier&);

    /**
     * @brief Default constructor.
     * @warning Object is not usable!
     */
    RemoteIndexListModifier()
      : glist_()
    {}

  private:

    /**
     * @brief Create a modifier for a remote index list.
     * @param indexSet The set of indices the process knows.
     * @param rList The list of remote indices to modify.
     */
    RemoteIndexListModifier(const ParallelIndexSet& indexSet,
                            RemoteIndexList& rList);

    typedef SLList<GlobalIndex,Allocator> GlobalList;
    typedef typename GlobalList::ModifyIterator GlobalModifyIterator;
    RemoteIndexList* rList_;
    const ParallelIndexSet* indexSet_;
    GlobalList glist_;
    ModifyIterator iter_;
    GlobalModifyIterator giter_;
    ConstIterator end_;
    bool first_;
    GlobalIndex last_;
  };

  /**
   * @brief A collective iterator for moving over the remote indices for
   * all processes collectively.
   */
  template<class T, class A>
  class CollectiveIterator
  {

    /**
     * @brief Type of the index set we use.
     */
    typedef T ParallelIndexSet;

    /**
     * @brief The type of the global index.
     */
    typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

    /**
     * @brief The type of the local index.
     */
    typedef typename ParallelIndexSet::LocalIndex LocalIndex;

    /**
     * @brief The type of the attribute.
     */
    typedef typename LocalIndex::Attribute Attribute;

    /** @brief The remote index type */
    typedef Dune::RemoteIndex<GlobalIndex,Attribute> RemoteIndex;

    /** @brief The allocator of the remote indices. */
    using Allocator = typename std::allocator_traits<A>::template rebind_alloc<RemoteIndex>;

    /** @brief The type of the remote index list. */
    typedef Dune::SLList<RemoteIndex,Allocator> RemoteIndexList;

    /** @brief The of map for storing the iterators. */
    typedef std::map<int,std::pair<typename RemoteIndexList::const_iterator,
            const typename RemoteIndexList::const_iterator> >
    Map;

  public:

    /** @brief The type of the map from rank to remote index list. */
    typedef std::map<int, std::pair<RemoteIndexList*,RemoteIndexList*> >
    RemoteIndexMap;

    /**
     * @brief Constructor.
     * @param map_ The map of the remote indices.
     * @param send True if we want iterate over the remote indices used for sending.
     */
    inline CollectiveIterator(const RemoteIndexMap& map_, bool send);

    /**
     * @brief Advances all underlying iterators.
     *
     * All iterators are advanced until they point to a remote index whose
     * global id is bigger or equal to global.
     * Iterators pointing to their end are removed.
     * @param global The index we search for.
     */
    inline void advance(const GlobalIndex& global);

    /**
     * @brief Advances all underlying iterators.
     *
     * All iterators are advanced until they point to a remote index whose
     * global id is bigger or equal to global.
     * Iterators pointing to their end are removed.
     * @param global The index we search for.
     * @param attribute The attribute we search for.
     */
    inline void advance(const GlobalIndex& global, const Attribute& attribute);

    CollectiveIterator& operator++();

    /**
     * @brief Checks whether there are still iterators in the map.
     */
    inline bool empty() const;

    /**
     * @brief Iterator over the valid underlying iterators.
     *
     * An iterator is valid if it points to a remote index whose
     * global id is equal to the one currently examined.
     */
    class iterator
    {
    public:
      typedef typename Map::iterator RealIterator;
      typedef typename Map::iterator ConstRealIterator;


      //! \todo Please doc me!
      iterator(const RealIterator& iter, const ConstRealIterator& end, GlobalIndex& index)
        : iter_(iter), end_(end), index_(index), hasAttribute(false)
      {
        // Move to the first valid entry
        while(iter_!=end_ && iter_->second.first->localIndexPair().global()!=index_)
          ++iter_;
      }

      iterator(const RealIterator& iter, const ConstRealIterator& end, GlobalIndex index,
               Attribute attribute)
        : iter_(iter), end_(end), index_(index), attribute_(attribute), hasAttribute(true)
      {
        // Move to the first valid entry or the end
        while(iter_!=end_ && (iter_->second.first->localIndexPair().global()!=index_
                              || iter_->second.first->localIndexPair().local().attribute()!=attribute))
          ++iter_;
      }
      //! \todo Please doc me!
      iterator(const iterator& other)
        : iter_(other.iter_), end_(other.end_), index_(other.index_)
      { }

      //! \todo Please doc me!
      iterator& operator++()
      {
        ++iter_;
        // If entry is not valid move on
        while(iter_!=end_ && (iter_->second.first->localIndexPair().global()!=index_ ||
                              (hasAttribute &&
                               iter_->second.first->localIndexPair().local().attribute()!=attribute_)))
          ++iter_;
        assert(iter_==end_ ||
               (iter_->second.first->localIndexPair().global()==index_));
        assert(iter_==end_ || !hasAttribute ||
               (iter_->second.first->localIndexPair().local().attribute()==attribute_));
        return *this;
      }

      //! \todo Please doc me!
      const RemoteIndex& operator*() const
      {
        return *(iter_->second.first);
      }

      //! \todo Please doc me!
      int process() const
      {
        return iter_->first;
      }

      //! \todo Please doc me!
      const RemoteIndex* operator->() const
      {
        return iter_->second.first.operator->();
      }

      //! \todo Please doc me!
      bool operator==(const iterator& other) const
      {
        return other.iter_==iter_;
      }

      //! \todo Please doc me!
      bool operator!=(const iterator& other) const
      {
        return other.iter_!=iter_;
      }

    private:
      iterator();

      RealIterator iter_;
      RealIterator end_;
      GlobalIndex index_;
      Attribute attribute_;
      bool hasAttribute;
    };

    iterator begin();

    iterator end();

  private:

    Map map_;
    GlobalIndex index_;
    Attribute attribute_;
    bool noattribute;
  };

  template<typename TG, typename TA>
  MPI_Datatype MPITraits<IndexPair<TG,ParallelLocalIndex<TA> > >::getType()
  {
    if(type==MPI_DATATYPE_NULL) {
      int length[2] = {1, 1};
      MPI_Aint base;
      MPI_Aint disp[2];
      MPI_Datatype types[2] = {MPITraits<TG>::getType(),
                               MPITraits<ParallelLocalIndex<TA> >::getType()};
      IndexPair<TG,ParallelLocalIndex<TA> > rep;
      MPI_Get_address(&rep, &base); // lower bound of the datatype
      MPI_Get_address(&(rep.global_), &disp[0]);
      MPI_Get_address(&(rep.local_), &disp[1]);
      for (MPI_Aint& d : disp)
        d -= base;

      MPI_Datatype tmp;
      MPI_Type_create_struct(2, length, disp, types, &tmp);

      MPI_Type_create_resized(tmp, 0, sizeof(IndexPair<TG,ParallelLocalIndex<TA> >), &type);
      MPI_Type_commit(&type);

      MPI_Type_free(&tmp);
    }
    return type;
  }

  template<typename TG, typename TA>
  MPI_Datatype MPITraits<IndexPair<TG,ParallelLocalIndex<TA> > >::type=MPI_DATATYPE_NULL;

  template<typename T1, typename T2>
  RemoteIndex<T1,T2>::RemoteIndex(const T2& attribute, const PairType* local)
    : localIndex_(local), attribute_(static_cast<std::underlying_type_t<T2>>(attribute))
  {}

  template<typename T1, typename T2>
  RemoteIndex<T1,T2>::RemoteIndex(const T2& attribute)
    : localIndex_(0), attribute_(static_cast<std::underlying_type_t<T2>>(attribute))
  {}

  template<typename T1, typename T2>
  RemoteIndex<T1,T2>::RemoteIndex()
    : localIndex_(0), attribute_()
  {}
  template<typename T1, typename T2>
  inline bool RemoteIndex<T1,T2>::operator==(const RemoteIndex& ri) const
  {
    return localIndex_==ri.localIndex_ && attribute_==ri.attribute;
  }

  template<typename T1, typename T2>
  inline bool RemoteIndex<T1,T2>::operator!=(const RemoteIndex& ri) const
  {
    return localIndex_!=ri.localIndex_ || attribute_!=ri.attribute_;
  }

  template<typename T1, typename T2>
  inline const T2 RemoteIndex<T1,T2>::attribute() const
  {
    return T2(attribute_);
  }

  template<typename T1, typename T2>
  inline const IndexPair<T1,ParallelLocalIndex<T2> >& RemoteIndex<T1,T2>::localIndexPair() const
  {
    return *localIndex_;
  }

  template<typename T, typename A>
  inline RemoteIndices<T,A>::RemoteIndices(const ParallelIndexSet& source,
                                           const ParallelIndexSet& destination,
                                           const MPI_Comm& comm,
                                           const std::vector<int>& neighbours,
                                           bool includeSelf_)
    : source_(&source), target_(&destination), comm_(comm),
      sourceSeqNo_(-1), destSeqNo_(-1), publicIgnored(false), firstBuild(true),
      includeSelf(includeSelf_)
  {
    setNeighbours(neighbours);
  }

  template<typename T, typename A>
  void RemoteIndices<T,A>::setIncludeSelf(bool b)
  {
    includeSelf=b;
  }

  template<typename T, typename A>
  RemoteIndices<T,A>::RemoteIndices()
    : source_(0), target_(0), sourceSeqNo_(-1),
      destSeqNo_(-1), publicIgnored(false), firstBuild(true),
      includeSelf(false)
  {}

  template<class T, typename A>
  void RemoteIndices<T,A>::setIndexSets(const ParallelIndexSet& source,
                                        const ParallelIndexSet& destination,
                                        const MPI_Comm& comm,
                                        const std::vector<int>& neighbours)
  {
    free();
    source_ = &source;
    target_ = &destination;
    comm_ = comm;
    firstBuild = true;
    setNeighbours(neighbours);
  }

  template<typename T, typename A>
  const typename RemoteIndices<T,A>::ParallelIndexSet&
  RemoteIndices<T,A>::sourceIndexSet() const
  {
    return *source_;
  }


  template<typename T, typename A>
  const typename RemoteIndices<T,A>::ParallelIndexSet&
  RemoteIndices<T,A>::destinationIndexSet() const
  {
    return *target_;
  }


  template<typename T, typename A>
  RemoteIndices<T,A>::~RemoteIndices()
  {
    free();
  }

  template<typename T, typename A>
  template<bool ignorePublic>
  inline void RemoteIndices<T,A>::packEntries(IndexPair<GlobalIndex,LocalIndex>** pairs,
                                              const ParallelIndexSet& indexSet,
                                              char* p_out, MPI_Datatype type,
                                              int bufferSize,
                                              int *position,
                                              [[maybe_unused]] int n)
  {
    // fill with own indices
    const auto end = indexSet.end();

    //Now pack the source indices
    int i=0;
    for(auto index = indexSet.begin(); index != end; ++index)
      if(ignorePublic || index->local().isPublic()) {

        MPI_Pack(const_cast<PairType*>(&(*index)), 1,
                 type,
                 p_out, bufferSize, position, comm_);
        pairs[i++] = const_cast<PairType*>(&(*index));

      }
    assert(i==n);
  }

  template<typename T, typename A>
  inline int RemoteIndices<T,A>::noPublic(const ParallelIndexSet& indexSet)
  {

    int noPublic=0;

    const auto end=indexSet.end();
    for(auto index=indexSet.begin(); index!=end; ++index)
      if(index->local().isPublic())
        noPublic++;

    return noPublic;

  }


  template<typename T, typename A>
  inline void RemoteIndices<T,A>::unpackCreateRemote(char* p_in, PairType** sourcePairs,
                                                     PairType** destPairs, int remoteProc,
                                                     int sourcePublish, int destPublish,
                                                     int bufferSize, bool sendTwo,
                                                     bool fromOurSelf)
  {

    // unpack the number of indices we received
    int noRemoteSource=-1, noRemoteDest=-1;
    char twoIndexSets=0;
    int position=0;
    // Did we receive two index sets?
    MPI_Unpack(p_in, bufferSize, &position, &twoIndexSets, 1, MPI_CHAR, comm_);
    // The number of source indices received
    MPI_Unpack(p_in, bufferSize, &position, &noRemoteSource, 1, MPI_INT, comm_);
    // The number of destination indices received
    MPI_Unpack(p_in, bufferSize, &position, &noRemoteDest, 1, MPI_INT, comm_);


    // Indices for which we receive
    RemoteIndexList* receive= new RemoteIndexList();
    // Indices for which we send
    RemoteIndexList* send=0;

    MPI_Datatype type= MPITraits<PairType>::getType();

    if(!twoIndexSets) {
      if(sendTwo) {
        send = new RemoteIndexList();
        // Create both remote index sets simultaneously
        unpackIndices(*send, *receive, noRemoteSource, sourcePairs, sourcePublish,
                      destPairs, destPublish, p_in, type, &position, bufferSize);
      }else{
        // we only need one list
        unpackIndices(*receive, noRemoteSource, sourcePairs, sourcePublish,
                      p_in, type, &position, bufferSize, fromOurSelf);
        send=receive;
      }
    }else{

      int oldPos=position;
      // Two index sets received
      unpackIndices(*receive, noRemoteSource, destPairs, destPublish,
                    p_in, type, &position, bufferSize, fromOurSelf);
      if(!sendTwo)
        //unpack source entries again as destination entries
        position=oldPos;

      send = new RemoteIndexList();
      unpackIndices(*send, noRemoteDest, sourcePairs, sourcePublish,
                    p_in, type, &position, bufferSize, fromOurSelf);
    }

    if(receive->empty() && send->empty()) {
      if(send==receive) {
        delete send;
      }else{
        delete send;
        delete receive;
      }
    }else{
      remoteIndices_.insert(std::make_pair(remoteProc,
                                           std::make_pair(send,receive)));
    }
  }


  template<typename T, typename A>
  template<bool ignorePublic>
  inline void RemoteIndices<T,A>::buildRemote(bool includeSelf_)
  {
    // Processor configuration
    int rank, procs;
    MPI_Comm_rank(comm_, &rank);
    MPI_Comm_size(comm_, &procs);

    // number of local indices to publish
    // The indices of the destination will be send.
    int sourcePublish, destPublish;

    // Do we need to send two index sets?
    char sendTwo = (source_ != target_);

    if(procs==1 && !(sendTwo || includeSelf_))
      // Nothing to communicate
      return;

    sourcePublish = (ignorePublic) ? source_->size() : noPublic(*source_);

    if(sendTwo)
      destPublish = (ignorePublic) ? target_->size() : noPublic(*target_);
    else
      // we only need to send one set of indices
      destPublish = 0;

    int maxPublish, publish=sourcePublish+destPublish;

    // Calculate maximum number of indices send
    MPI_Allreduce(&publish, &maxPublish, 1, MPI_INT, MPI_MAX, comm_);

    // allocate buffers
    PairType** destPairs;
    PairType** sourcePairs = new PairType*[sourcePublish>0 ? sourcePublish : 1];

    if(sendTwo)
      destPairs = new PairType*[destPublish>0 ? destPublish : 1];
    else
      destPairs=sourcePairs;

    char** buffer = new char*[2];
    int bufferSize;
    int position=0;
    int intSize;
    int charSize;

    // calculate buffer size
    MPI_Datatype type = MPITraits<PairType>::getType();

    MPI_Pack_size(maxPublish, type, comm_,
                  &bufferSize);
    MPI_Pack_size(1, MPI_INT, comm_,
                  &intSize);
    MPI_Pack_size(1, MPI_CHAR, comm_,
                  &charSize);
    // Our message will contain the following:
    // a bool whether two index sets where sent
    // the size of the source and the dest indexset,
    // then the source and destination indices
    bufferSize += 2 * intSize + charSize;

    if(bufferSize<=0) bufferSize=1;

    buffer[0] = new char[bufferSize];
    buffer[1] = new char[bufferSize];


    // pack entries into buffer[0], p_out below!
    MPI_Pack(&sendTwo, 1, MPI_CHAR, buffer[0], bufferSize, &position,
             comm_);

    // The number of indices we send for each index set
    MPI_Pack(&sourcePublish, 1, MPI_INT, buffer[0], bufferSize, &position,
             comm_);
    MPI_Pack(&destPublish, 1, MPI_INT, buffer[0], bufferSize, &position,
             comm_);

    // Now pack the source indices and setup the destination pairs
    packEntries<ignorePublic>(sourcePairs, *source_, buffer[0], type,
                              bufferSize, &position, sourcePublish);
    // If necessary send the dest indices and setup the source pairs
    if(sendTwo)
      packEntries<ignorePublic>(destPairs, *target_, buffer[0], type,
                                bufferSize, &position, destPublish);


    // Update remote indices for ourself
    if(sendTwo|| includeSelf_)
      unpackCreateRemote(buffer[0], sourcePairs, destPairs, rank, sourcePublish,
                         destPublish, bufferSize, sendTwo, includeSelf_);

    neighbourIds.erase(rank);

    if(neighbourIds.size()==0)
    {
      Dune::dvverb<<rank<<": Sending messages in a ring"<<std::endl;
      // send messages in ring
      for(int proc=1; proc<procs; proc++) {
        // pointers to the current input and output buffers
        char* p_out = buffer[1-(proc%2)];
        char* p_in = buffer[proc%2];

        MPI_Status status;
        if(rank%2==0) {
          MPI_Ssend(p_out, bufferSize, MPI_PACKED, (rank+1)%procs,
                    commTag_, comm_);
          MPI_Recv(p_in, bufferSize, MPI_PACKED, (rank+procs-1)%procs,
                   commTag_, comm_, &status);
        }else{
          MPI_Recv(p_in, bufferSize, MPI_PACKED, (rank+procs-1)%procs,
                   commTag_, comm_, &status);
          MPI_Ssend(p_out, bufferSize, MPI_PACKED, (rank+1)%procs,
                    commTag_, comm_);
        }


        // The process these indices are from
        int remoteProc = (rank+procs-proc)%procs;

        unpackCreateRemote(p_in, sourcePairs, destPairs, remoteProc, sourcePublish,
                           destPublish, bufferSize, sendTwo);

      }

    }
    else
    {
      MPI_Request* requests=new MPI_Request[neighbourIds.size()];
      MPI_Request* req=requests;

      typedef typename std::set<int>::size_type size_type;
      size_type noNeighbours=neighbourIds.size();

      // setup sends
      for(std::set<int>::iterator neighbour=neighbourIds.begin();
          neighbour!= neighbourIds.end(); ++neighbour) {
        // Only send the information to the neighbouring processors
        MPI_Issend(buffer[0], position , MPI_PACKED, *neighbour, commTag_, comm_, req++);
      }

      //Test for received messages

      for(size_type received=0; received <noNeighbours; ++received)
      {
        MPI_Status status;
        // probe for next message
        MPI_Probe(MPI_ANY_SOURCE, commTag_, comm_, &status);
        int remoteProc=status.MPI_SOURCE;
        int size;
        MPI_Get_count(&status, MPI_PACKED, &size);
        // receive message
        MPI_Recv(buffer[1], size, MPI_PACKED, remoteProc,
                 commTag_, comm_, &status);

        unpackCreateRemote(buffer[1], sourcePairs, destPairs, remoteProc, sourcePublish,
                           destPublish, bufferSize, sendTwo);
      }
      // wait for completion of pending requests
      MPI_Status* statuses = new MPI_Status[neighbourIds.size()];

      if(MPI_ERR_IN_STATUS==MPI_Waitall(neighbourIds.size(), requests, statuses)) {
        for(size_type i=0; i < neighbourIds.size(); ++i)
          if(statuses[i].MPI_ERROR!=MPI_SUCCESS) {
            std::cerr<<rank<<": MPI_Error occurred while receiving message."<<std::endl;
            MPI_Abort(comm_, 999);
          }
      }
      delete[] requests;
      delete[] statuses;
    }


    // delete allocated memory
    if(destPairs!=sourcePairs)
      delete[] destPairs;

    delete[] sourcePairs;
    delete[] buffer[0];
    delete[] buffer[1];
    delete[] buffer;
  }

  template<typename T, typename A>
  inline void RemoteIndices<T,A>::unpackIndices(RemoteIndexList& remote,
                                                int remoteEntries,
                                                PairType** local,
                                                int localEntries,
                                                char* p_in,
                                                MPI_Datatype type,
                                                int* position,
                                                int bufferSize,
                                                bool fromOurSelf)
  {
    if(remoteEntries==0)
      return;

    PairType index;
    MPI_Unpack(p_in, bufferSize, position, &index, 1,
               type, comm_);
    GlobalIndex oldGlobal=index.global();
    int n_in=0, localIndex=0;

    //Check if we know the global index
    while(localIndex<localEntries) {
      if(local[localIndex]->global()==index.global()) {
        int oldLocalIndex=localIndex;

        while(localIndex<localEntries &&
              local[localIndex]->global()==index.global()) {
          if(!fromOurSelf || index.local().attribute() !=
             local[localIndex]->local().attribute())
            // if index is from us it has to have a different attribute
            remote.push_back(RemoteIndex(index.local().attribute(),
                                         local[localIndex]));
          localIndex++;
        }

        // unpack next remote index
        if((++n_in) < remoteEntries) {
          MPI_Unpack(p_in, bufferSize, position, &index, 1,
                     type, comm_);
          if(index.global()==oldGlobal)
            // Restart comparison for the same global indices
            localIndex=oldLocalIndex;
          else
            oldGlobal=index.global();
        }else{
          // No more received indices
          break;
        }
        continue;
      }

      if (local[localIndex]->global()<index.global()) {
        // compare with next entry in our list
        ++localIndex;
      }else{
        // We do not know the index, unpack next
        if((++n_in) < remoteEntries) {
          MPI_Unpack(p_in, bufferSize, position, &index, 1,
                     type, comm_);
          oldGlobal=index.global();
        }else
          // No more received indices
          break;
      }
    }

    // Unpack the other received indices without doing anything
    while(++n_in < remoteEntries)
      MPI_Unpack(p_in, bufferSize, position, &index, 1,
                 type, comm_);
  }


  template<typename T, typename A>
  inline void RemoteIndices<T,A>::unpackIndices(RemoteIndexList& send,
                                                RemoteIndexList& receive,
                                                int remoteEntries,
                                                PairType** localSource,
                                                int localSourceEntries,
                                                PairType** localDest,
                                                int localDestEntries,
                                                char* p_in,
                                                MPI_Datatype type,
                                                int* position,
                                                int bufferSize)
  {
    int n_in=0, sourceIndex=0, destIndex=0;

    //Check if we know the global index
    while(n_in<remoteEntries && (sourceIndex<localSourceEntries || destIndex<localDestEntries)) {
      // Unpack next index
      PairType index;
      MPI_Unpack(p_in, bufferSize, position, &index, 1,
                 type, comm_);
      n_in++;

      // Advance until global index in localSource and localDest are >= than the one in the unpacked index
      while(sourceIndex<localSourceEntries && localSource[sourceIndex]->global()<index.global())
        sourceIndex++;

      while(destIndex<localDestEntries && localDest[destIndex]->global()<index.global())
        destIndex++;

      // Add a remote index if we found the global index.
      if(sourceIndex<localSourceEntries && localSource[sourceIndex]->global()==index.global())
        send.push_back(RemoteIndex(index.local().attribute(),
                                   localSource[sourceIndex]));

      if(destIndex < localDestEntries && localDest[destIndex]->global() == index.global())
        receive.push_back(RemoteIndex(index.local().attribute(),
                                      localDest[sourceIndex]));
    }

  }

  template<typename T, typename A>
  inline void RemoteIndices<T,A>::free()
  {
    auto lend = remoteIndices_.end();
    for(auto lists=remoteIndices_.begin(); lists != lend; ++lists) {
      if(lists->second.first==lists->second.second) {
        // there is only one remote index list.
        delete lists->second.first;
      }else{
        delete lists->second.first;
        delete lists->second.second;
      }
    }
    remoteIndices_.clear();
    firstBuild=true;
  }

  template<typename T, typename A>
  inline int RemoteIndices<T,A>::neighbours() const
  {
    return remoteIndices_.size();
  }

  template<typename T, typename A>
  template<bool ignorePublic>
  inline void RemoteIndices<T,A>::rebuild()
  {
    // Test whether a rebuild is Needed.
    if(firstBuild ||
       ignorePublic!=publicIgnored || !
       isSynced()) {
      free();

      buildRemote<ignorePublic>(includeSelf);

      sourceSeqNo_ = source_->seqNo();
      destSeqNo_ = target_->seqNo();
      firstBuild=false;
      publicIgnored=ignorePublic;
    }


  }

  template<typename T, typename A>
  inline bool RemoteIndices<T,A>::isSynced() const
  {
    return sourceSeqNo_==source_->seqNo() && destSeqNo_ ==target_->seqNo();
  }

  template<typename T, typename A>
  template<bool mode, bool send>
  RemoteIndexListModifier<T,A,mode> RemoteIndices<T,A>::getModifier(int process)
  {

    // The user are on their own now!
    // We assume they know what they are doing and just set the
    // remote indices to synced status.
    sourceSeqNo_ = source_->seqNo();
    destSeqNo_ = target_->seqNo();

    typename RemoteIndexMap::iterator found = remoteIndices_.find(process);

    if(found == remoteIndices_.end())
    {
      if(source_ != target_)
        found = remoteIndices_.insert(found, std::make_pair(process,
                                             std::make_pair(new RemoteIndexList(),
                                                            new RemoteIndexList())));
      else{
        RemoteIndexList* rlist = new RemoteIndexList();
        found = remoteIndices_.insert(found,
                                      std::make_pair(process,
                                             std::make_pair(rlist, rlist)));
      }
    }

    firstBuild = false;

    if(send)
      return RemoteIndexListModifier<T,A,mode>(*source_, *(found->second.first));
    else
      return RemoteIndexListModifier<T,A,mode>(*target_, *(found->second.second));
  }

  template<typename T, typename A>
  inline typename RemoteIndices<T,A>::const_iterator
  RemoteIndices<T,A>::find(int proc) const
  {
    return remoteIndices_.find(proc);
  }

  template<typename T, typename A>
  inline typename RemoteIndices<T,A>::const_iterator
  RemoteIndices<T,A>::begin() const
  {
    return remoteIndices_.begin();
  }

  template<typename T, typename A>
  inline typename RemoteIndices<T,A>::const_iterator
  RemoteIndices<T,A>::end() const
  {
    return remoteIndices_.end();
  }


  template<typename T, typename A>
  bool RemoteIndices<T,A>::operator==(const RemoteIndices& ri) const
  {
    if(neighbours()!=ri.neighbours())
      return false;

    const auto rend = remoteIndices_.end();

    for(auto rindex = remoteIndices_.begin(), rindex1=ri.remoteIndices_.begin(); rindex!=rend; ++rindex, ++rindex1) {
      if(rindex->first != rindex1->first)
        return false;
      if(*(rindex->second.first) != *(rindex1->second.first))
        return false;
      if(*(rindex->second.second) != *(rindex1->second.second))
        return false;
    }
    return true;
  }

  template<class T, class A, bool mode>
  RemoteIndexListModifier<T,A,mode>::RemoteIndexListModifier(const ParallelIndexSet& indexSet,
                                                             RemoteIndexList& rList)
    : rList_(&rList), indexSet_(&indexSet), iter_(rList.beginModify()), end_(rList.end()), first_(true)
  {
    if(MODIFYINDEXSET) {
      assert(indexSet_);
      for(ConstIterator iter=iter_; iter != end_; ++iter)
        glist_.push_back(iter->localIndexPair().global());
      giter_ = glist_.beginModify();
    }
  }

  template<typename T, typename A, bool mode>
  RemoteIndexListModifier<T,A,mode>::RemoteIndexListModifier(const RemoteIndexListModifier<T,A,mode>& other)
    : rList_(other.rList_), indexSet_(other.indexSet_),
      glist_(other.glist_), iter_(other.iter_), giter_(other.giter_), end_(other.end_),
      first_(other.first_), last_(other.last_)
  {}

  template<typename T, typename A, bool mode>
  inline void RemoteIndexListModifier<T,A,mode>::repairLocalIndexPointers()
  {
    if(MODIFYINDEXSET) {
      // repair pointers to local index set.
#ifdef DUNE_ISTL_WITH_CHECKING
      if(indexSet_->state()!=GROUND)
        DUNE_THROW(InvalidIndexSetState, "Index has to be in ground mode for repairing pointers to indices");
#endif
      auto giter = glist_.begin();
      auto index = indexSet_->begin();

      for(auto iter=rList_->begin(); iter != end_; ++iter) {
        while(index->global()<*giter) {
          ++index;
#ifdef DUNE_ISTL_WITH_CHECKING
          if(index == indexSet_->end())
            DUNE_THROW(InvalidPosition, "No such global index in set!");
#endif
        }

#ifdef DUNE_ISTL_WITH_CHECKING
        if(index->global() != *giter)
          DUNE_THROW(InvalidPosition, "No such global index in set!");
#endif
        iter->localIndex_ = &(*index);
      }
    }
  }

  template<typename T, typename A, bool mode>
  inline void RemoteIndexListModifier<T,A,mode>::insert(const RemoteIndex& index)
  {
    static_assert(!mode,"Not allowed if the mode indicates that new indices"
                        "might be added to the underlying index set. Use "
                        "insert(const RemoteIndex&, const GlobalIndex&) instead");

#ifdef DUNE_ISTL_WITH_CHECKING
    if(!first_ && index.localIndexPair().global()<last_)
      DUNE_THROW(InvalidPosition, "Modification of remote indices have to occur with ascending global index.");
#endif
    // Move to the correct position
    while(iter_ != end_ && iter_->localIndexPair().global() < index.localIndexPair().global()) {
      ++iter_;
    }

    // No duplicate entries allowed
    assert(iter_==end_ || iter_->localIndexPair().global() != index.localIndexPair().global());
    iter_.insert(index);
    last_ = index.localIndexPair().global();
    first_ = false;
  }

  template<typename T, typename A, bool mode>
  inline void RemoteIndexListModifier<T,A,mode>::insert(const RemoteIndex& index, const GlobalIndex& global)
  {
    static_assert(mode,"Not allowed if the mode indicates that no new indices"
                       "might be added to the underlying index set. Use "
                       "insert(const RemoteIndex&) instead");
#ifdef DUNE_ISTL_WITH_CHECKING
    if(!first_ && global<last_)
      DUNE_THROW(InvalidPosition, "Modification of remote indices have to occur with ascending global index.");
#endif
    // Move to the correct position
    while(iter_ != end_ && *giter_ < global) {
      ++giter_;
      ++iter_;
    }

    // No duplicate entries allowed
    assert(iter_->localIndexPair().global() != global);
    iter_.insert(index);
    giter_.insert(global);

    last_ = global;
    first_ = false;
  }

  template<typename T, typename A, bool mode>
  bool RemoteIndexListModifier<T,A,mode>::remove(const GlobalIndex& global)
  {
#ifdef DUNE_ISTL_WITH_CHECKING
    if(!first_ && global<last_)
      DUNE_THROW(InvalidPosition, "Modification of remote indices have to occur with ascending global index.");
#endif

    bool found= false;

    if(MODIFYINDEXSET) {
      // Move to the correct position
      while(iter_!=end_ && *giter_< global) {
        ++giter_;
        ++iter_;
      }
      if(*giter_ == global) {
        giter_.remove();
        iter_.remove();
        found=true;
      }
    }else{
      while(iter_!=end_ && iter_->localIndexPair().global() < global)
        ++iter_;

      if(iter_->localIndexPair().global()==global) {
        iter_.remove();
        found = true;
      }
    }

    last_ = global;
    first_ = false;
    return found;
  }

  template<typename T, typename A>
  template<bool send>
  inline typename RemoteIndices<T,A>::CollectiveIteratorT RemoteIndices<T,A>::iterator() const
  {
    return CollectiveIterator<T,A>(remoteIndices_, send);
  }

  template<typename T, typename A>
  inline MPI_Comm RemoteIndices<T,A>::communicator() const
  {
    return comm_;

  }

  template<typename T, typename A>
  CollectiveIterator<T,A>::CollectiveIterator(const RemoteIndexMap& pmap, bool send)
  {

    const auto end = pmap.end();
    for(auto process = pmap.begin(); process != end; ++process) {
      const RemoteIndexList* list = send ? process->second.first : process->second.second;
      using ri_iterator = typename RemoteIndexList::const_iterator;
      map_.insert(std::make_pair(process->first,
                                 std::pair<ri_iterator, const ri_iterator>(list->begin(), list->end())));
    }
  }

  template<typename T, typename A>
  inline void CollectiveIterator<T,A>::advance(const GlobalIndex& index)
  {
    const auto end = map_.end();

    for(auto iter = map_.begin(); iter != end;) {
      // Step the iterator until we are >= index
      typename RemoteIndexList::const_iterator current = iter->second.first;
      typename RemoteIndexList::const_iterator rend = iter->second.second;
      RemoteIndex remoteIndex;
      if(current != rend)
        remoteIndex = *current;

      while(iter->second.first!=iter->second.second && iter->second.first->localIndexPair().global()<index)
        ++(iter->second.first);

      // erase from the map if there are no more entries.
      if(iter->second.first == iter->second.second)
        map_.erase(iter++);
      else{
        ++iter;
      }
    }
    index_=index;
    noattribute=true;
  }

  template<typename T, typename A>
  inline void CollectiveIterator<T,A>::advance(const GlobalIndex& index,
                                               const Attribute& attribute)
  {
    const auto end = map_.end();

    for(auto iter = map_.begin(); iter != end;) {
      // Step the iterator until we are >= index
      typename RemoteIndexList::const_iterator current = iter->second.first;
      typename RemoteIndexList::const_iterator rend = iter->second.second;
      RemoteIndex remoteIndex;
      if(current != rend)
        remoteIndex = *current;

      // Move to global index or bigger
      while(iter->second.first!=iter->second.second && iter->second.first->localIndexPair().global()<index)
        ++(iter->second.first);

      // move to attribute or bigger
      while(iter->second.first!=iter->second.second
            && iter->second.first->localIndexPair().global()==index
            && iter->second.first->localIndexPair().local().attribute()<attribute)
        ++(iter->second.first);

      // erase from the map if there are no more entries.
      if(iter->second.first == iter->second.second)
        map_.erase(iter++);
      else{
        ++iter;
      }
    }
    index_=index;
    attribute_=attribute;
    noattribute=false;
  }

  template<typename T, typename A>
  inline CollectiveIterator<T,A>& CollectiveIterator<T,A>::operator++()
  {
    const auto end = map_.end();

    for(auto iter = map_.begin(); iter != end;) {
      // Step the iterator until we are >= index
      auto current = iter->second.first;
      auto rend = iter->second.second;

      // move all iterators pointing to the current global index to next value
      if(iter->second.first->localIndexPair().global()==index_ &&
         (noattribute || iter->second.first->localIndexPair().local().attribute() == attribute_))
        ++(iter->second.first);

      // erase from the map if there are no more entries.
      if(iter->second.first == iter->second.second)
        map_.erase(iter++);
      else{
        ++iter;
      }
    }
    return *this;
  }

  template<typename T, typename A>
  inline bool CollectiveIterator<T,A>::empty() const
  {
    return map_.empty();
  }

  template<typename T, typename A>
  inline typename CollectiveIterator<T,A>::iterator
  CollectiveIterator<T,A>::begin()
  {
    if(noattribute)
      return iterator(map_.begin(), map_.end(), index_);
    else
      return iterator(map_.begin(), map_.end(), index_,
                      attribute_);
  }

  template<typename T, typename A>
  inline typename CollectiveIterator<T,A>::iterator
  CollectiveIterator<T,A>::end()
  {
    return iterator(map_.end(), map_.end(), index_);
  }

  template<typename TG, typename TA>
  inline std::ostream& operator<<(std::ostream& os, const RemoteIndex<TG,TA>& index)
  {
    os<<"[global="<<index.localIndexPair().global()<<", remote attribute="<<index.attribute()<<" local attribute="<<index.localIndexPair().local().attribute()<<"]";
    return os;
  }

  template<typename T, typename A>
  inline std::ostream& operator<<(std::ostream& os, const RemoteIndices<T,A>& indices)
  {
    int rank;
    MPI_Comm_rank(indices.comm_, &rank);
    const auto rend = indices.remoteIndices_.end();

    for(auto rindex = indices.remoteIndices_.begin(); rindex!=rend; ++rindex) {
      os<<rank<<": Process "<<rindex->first<<":";

      if(!rindex->second.first->empty()) {
        os<<" send:";

        const auto send= rindex->second.first->end();

        for(auto index = rindex->second.first->begin();
            index != send; ++index)
          os<<*index<<" ";
        os<<std::endl;
      }
      if(!rindex->second.second->empty()) {
        os<<rank<<": Process "<<rindex->first<<": "<<"receive: ";

        for(const auto& index : *(rindex->second.second))
          os << index << " ";
      }
      os<<std::endl<<std::flush;
    }
    return os;
  }
  /** @} */
}

#endif // HAVE_MPI

#endif
