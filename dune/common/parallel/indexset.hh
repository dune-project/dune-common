// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_INDEXSET_HH
#define DUNE_INDEXSET_HH

#include <algorithm>
#include <dune/common/arraylist.hh>
#include <dune/common/exceptions.hh>
#include <iostream>

#include "localindex.hh"

#include <stdint.h> // for uint32_t

namespace Dune
{
  /** @addtogroup Common_Parallel
   *
   * @{
   */
  /**
   * @file
   * @brief Provides a map between global and local indices.
   * @author Markus Blatt
   */
  // forward declarations

  template<class TG, class TL>
  class IndexPair;

  /**
   * @brief Print an index pair.
   * @param os The outputstream to print to.
   * @param pair The index pair to print.
   */
  template<class TG, class TL>
  std::ostream& operator<<(std::ostream& os, const IndexPair<TG,TL>& pair);

  template<class TG, class TL>
  bool operator==(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);

  template<class TG, class TL>
  bool operator!=(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);

  template<class TG, class TL>
  bool operator<(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);

  template<class TG, class TL>
  bool operator>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);

  template<class TG, class TL>
  bool operator<=(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);

  template<class TG, class TL>
  bool operator >=(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);

  template<class TG, class TL>
  bool operator==(const IndexPair<TG,TL>&, const TG&);

  template<class TG, class TL>
  bool operator!=(const IndexPair<TG,TL>&, const TG&);

  template<class TG, class TL>
  bool operator<(const IndexPair<TG,TL>&, const TG&);

  template<class TG, class TL>
  bool operator>(const IndexPair<TG,TL>&, const TG&);

  template<class TG, class TL>
  bool operator<=(const IndexPair<TG,TL>&, const TG&);

  template<class TG, class TL>
  bool operator >=(const IndexPair<TG,TL>&, const TG&);

  template<typename T>
  struct MPITraits;

  /**
   * @brief A pair consisting of a global and local index.
   */
  template<class TG, class TL>
  class IndexPair
  {
    friend std::ostream& operator<<<>(std::ostream&, const IndexPair<TG,TL>&);
    friend bool operator==<>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);
    friend bool operator!=<>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);
    friend bool operator< <>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);
    friend bool operator><>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);
    friend bool operator<=<>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);
    friend bool operator>=<>(const IndexPair<TG,TL>&, const IndexPair<TG,TL>&);
    friend bool operator==<>(const IndexPair<TG,TL>&, const TG &);
    friend bool operator!=<>(const IndexPair<TG,TL>&, const TG &);
    friend bool operator< <>(const IndexPair<TG,TL>&, const TG &);
    friend bool operator> <>(const IndexPair<TG,TL>&, const TG &);
    friend bool operator<=<>(const IndexPair<TG,TL>&, const TG &);
    friend bool operator>=<>(const IndexPair<TG,TL>&, const TG &);
    friend struct MPITraits<IndexPair<TG,TL> >;

  public:
    /**
     * @brief the type of the global index.
     *
     * This type has to provide at least a operator&lt; for sorting.
     */
    typedef TG GlobalIndex;

    /**
     * @brief the type of the local index.
     *
     * This class to provide the following functions:
     * \code
     * LocalIndex operator=(int);
     * operator int() const;
     * LocalIndexState state() const;
     * void setState(LocalIndexState);
     * \endcode
     */
    typedef TL LocalIndex;

    /**
     * @brief Constructs a new Pair.
     *
     * @param global The global index.
     * @param local The local index.
     */
    IndexPair(const GlobalIndex& global, const LocalIndex& local);

    /**
     * @brief Construct a new Pair.
     */
    IndexPair();
    /**
     * @brief Constructs a new Pair.
     *
     * The local index will be 0.
     * @param global The global index.
     */
    IndexPair(const GlobalIndex& global);

    /**
     * @brief Get the global index.
     *
     * @return The global index.
     */
    inline const GlobalIndex& global() const;

    /**
     * @brief Get the local index.
     *
     * @return The local index.
     */
    inline LocalIndex& local();

    /**
     * @brief Get the local index.
     *
     * @return The local index.
     */
    inline const LocalIndex& local() const;

    /**
     * @brief Set the local index.
     *
     * @param index The index to set it to.
     */
    inline void setLocal(int index);
  private:
    /** @brief The global index. */
    GlobalIndex global_;
    /** @brief The local index. */
    LocalIndex local_;
  };

  /**
   * @brief The states the index set can be in.
   * @see ParallelIndexSet::state_
   */
  enum ParallelIndexSetState
  {
    /**
     * @brief The default mode.
     * Indicates that the index set is ready to be used.
     */
    GROUND,
    /**
     * @brief Indicates that the index set is currently being resized.
     */
    RESIZE
    /**
     * @brief Indicates that all previously deleted indices are now deleted.
     *
       CLEAN,
     **
     * @brief Indicates that the index set is currently being reordered.
     *
       REORDER
     */
  };

  /**
   * @brief Exception indicating that the index set is not in the expected state.
   */
  class InvalidIndexSetState : public InvalidStateException {};

  // Forward declaration
  template<class I> class GlobalLookupIndexSet;

  /**
   * @brief Manager class for the mapping between local indices and globally unique indices.
   *
   * The mapping is between a globally unique id and local index. The local index is consecutive
   * and non persistent while the global id might not be consecutive but definitely is persistent.
   */
  template<typename TG, typename TL, int N=100>
  class ParallelIndexSet
  {
    friend class GlobalLookupIndexSet<ParallelIndexSet<TG,TL,N> >;

  public:
    /**
     * @brief the type of the global index.
     * This type has to provide at least a operator&lt; for sorting.
     */
    typedef TG GlobalIndex;

    /**
     * @brief The type of the local index, e.g. ParallelLocalIndex.
     *
     * This class to provide the following functions:
     * \code
     * LocalIndex operator=(int);
     * operator int() const;
     * LocalIndexState state() const;
     * void setState(LocalIndexState);
     * \endcode
     */
    typedef TL LocalIndex;

    /**
     * @brief The type of the pair stored.
     */
    typedef Dune::IndexPair<GlobalIndex,LocalIndex> IndexPair;

    /**
     * @brief The size of the individual arrays in the underlying ArrayList.
     *
     * The default value is 100.
     * @see ArrayList::size
     */
    constexpr static int arraySize = (N>0) ? N : 1;

    /** @brief The iterator over the pairs. */
    class iterator :
      public ArrayList<IndexPair,N>::iterator
    {
      typedef typename ArrayList<IndexPair,N>::iterator
      Father;
      friend class ParallelIndexSet<GlobalIndex,LocalIndex,N>;
    public:
      iterator(ParallelIndexSet<TG,TL,N>& indexSet, const Father& father)
        : Father(father), indexSet_(&indexSet)
      {}

    private:
      /**
       * @brief Mark the index as deleted.
       *
       * The deleted flag will be set in the local index.
       * The index will be removed in the endResize method of the
       * index set.
       *
       * @exception InvalidIndexSetState only when NDEBUG is not defined
       */
      inline void markAsDeleted() const
      {
#ifndef NDEBUG
        if(indexSet_->state_ != RESIZE)
          DUNE_THROW(InvalidIndexSetState, "Indices can only be removed "
                     <<"while in RESIZE state!");
#endif
        Father::operator*().local().setState(DELETED);
      }

      /** @brief The index set we are an iterator of. */
      ParallelIndexSet<TG,TL,N>* indexSet_;

    };



    /** @brief The constant iterator over the pairs. */
    typedef typename
    ArrayList<IndexPair,N>::const_iterator
    const_iterator;

    /**
     * @brief Constructor.
     */
    ParallelIndexSet();

    /**
     * @brief Get the state the index set is in.
     * @return The state of the index set.
     */
    inline const ParallelIndexSetState& state()
    {
      return state_;
    }

    /**
     * @brief Indicate that the index set is to be resized.
     * @exception InvalidState If index set was not in
     * ParallelIndexSetState::GROUND mode.
     */
    void beginResize();

    /**
     * @brief Add an new index to the set.
     *
     * The local index is created by the default constructor.
     * @param global The globally unique id of the index.
     * @exception InvalidState If index set is not in
     * ParallelIndexSetState::RESIZE mode.
     */
    inline void add(const GlobalIndex& global);

    /**
     * @brief Add an new index to the set.
     *
     * @param global The globally unique id of the index.
     * @param local The local index.
     * @exception InvalidState If index set is not in
     * ParallelIndexSetState::RESIZE mode.
     */
    inline void add(const GlobalIndex& global, const LocalIndex& local);

    /**
     * @brief Mark an index as deleted.
     *
     * The index will be deleted during endResize().
     * @param position An iterator at the position we want to delete.
     * @exception InvalidState If index set is not in ParallelIndexSetState::RESIZE mode.
     */
    inline void markAsDeleted(const iterator& position);

    /**
     * @brief Indicate that the resizing finishes.
     *
     * @warning Invalidates all pointers stored to the elements of this index set.
     * The local indices will be ordered
     * according to the global indices:
     * Let \f$(g_i,l_i)_{i=0}^N \f$ be the set of all indices then \f$l_i < l_j\f$
     * if and
     * only if \f$g_i < g_j\f$ for arbitrary \f$i \neq j\f$.
     * @exception InvalidState If index set was not in
     * ParallelIndexSetState::RESIZE mode.
     */
    void endResize();

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefore has complexity
     * log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @warning If the global index is not in the set a wrong or even a
     * null reference might be returned. To be save use the throwing alternative at.
     */
    inline IndexPair&
    operator[](const GlobalIndex& global);

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefore has complexity
     * log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @exception RangeError Thrown if the global id is not known.
     */
    inline IndexPair&
    at(const GlobalIndex& global);

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefore has complexity
     * log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @exception RangeError Thrown if the global id is not known.
     */
    inline bool
    exists (const GlobalIndex& global) const;

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefore has complexity
     * log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @warning If the global index is not in the set a wrong or even a
     * null reference might be returned. To be save use the throwing alternative at.
     */
    inline const IndexPair&
    operator[](const GlobalIndex& global) const;

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefore has complexity
     * log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @exception RangeError Thrown if the global id is not known.
     */
    inline const IndexPair&
    at(const GlobalIndex& global) const;

    /**
     * @brief Get an iterator over the indices positioned at the first index.
     * @return Iterator over the local indices.
     */
    inline iterator begin();

    /**
     * @brief Get an iterator over the indices positioned after the last index.
     * @return Iterator over the local indices.
     */
    inline iterator end();

    /**
     * @brief Get an iterator over the indices positioned at the first index.
     * @return Iterator over the local indices.
     */
    inline const_iterator begin() const;

    /**
     * @brief Get an iterator over the indices positioned after the last index.
     * @return Iterator over the local indices.
     */
    inline const_iterator end() const;

    /**
     * @brief Renumbers the local index numbers.
     *
     * After this function returns the indices are
     * consecutively numbered beginning from 0. Let
     * $(g_i,l_i)$, $(g_j,l_j)$ be two arbitrary index
     * pairs with $g_i<g_j$ then after renumbering
     * $l_i<l_j$ will hold.
     */
    inline void renumberLocal();

    /**
     * @brief Get the internal sequence number.
     *
     * Is initially 0 is incremented for each resize.
     * @return The sequence number.
     */
    inline int seqNo() const;

    /**
     * @brief Get the total number (public and nonpublic) indices.
     * @return The total number (public and nonpublic) indices.
     */
    inline size_t size() const;

  private:
    /** @brief The index pairs. */
    ArrayList<IndexPair,N> localIndices_;
    /** @brief The new indices for the RESIZE state. */
    ArrayList<IndexPair,N> newIndices_;
    /** @brief The state of the index set. */
    ParallelIndexSetState state_;
    /** @brief Number to keep track of the number of resizes. */
    int seqNo_;
    /** @brief Whether entries were deleted in resize mode. */
    bool deletedEntries_;
    /**
     * @brief Merges the _localIndices and newIndices arrays and creates a new
     * localIndices array.
     */
    inline void merge();
  };


  /**
   * @brief Print an index set.
   * @param os The outputstream to print to.
   * @param indexSet The index set to print.
   */
  template<class TG, class TL, int N>
  std::ostream& operator<<(std::ostream& os, const ParallelIndexSet<TG,TL,N>& indexSet);

  /**
   * @brief Decorates an index set with the possibility to find a global index
   * that is mapped to a specific local.
   *
   */
  template<class I>
  class GlobalLookupIndexSet
  {
  public:
    /**
     * @brief The type of the index set.
     */
    typedef I ParallelIndexSet;

    /**
     * @brief The type of the local index.
     */
    typedef typename ParallelIndexSet::LocalIndex LocalIndex;

    /**
     * @brief The type of the global index.
     */
    typedef typename ParallelIndexSet::GlobalIndex GlobalIndex;

    /**
     * @brief The iterator over the index pairs.
     */
    typedef typename ParallelIndexSet::const_iterator const_iterator;

    typedef Dune::IndexPair<typename I::GlobalIndex, typename I::LocalIndex> IndexPair;

    /**
     * @brief Constructor.
     * @param indexset The index set we want to be able to lookup the corresponding
     * global index of a local index.
     * @param size The number of indices present, i.e. one more than the maximum local index.
     */
    GlobalLookupIndexSet(const ParallelIndexSet& indexset, std::size_t size);

    /**
     * @brief Constructor.
     * @param indexset The index set we want to be able to lookup the corresponding
     * global index of a local index.
     */
    GlobalLookupIndexSet(const ParallelIndexSet& indexset);

    /**
     * @brief Destructor.
     */
    ~GlobalLookupIndexSet();

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefore has complexity
     * log(N). This method is forwarded to the underlying index set.
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @exception RangeError Thrown if the global id is not known.
     */
    inline const IndexPair&
    operator[](const GlobalIndex& global) const;

    /**
     * @brief Get the index pair corresponding to a local index.
     */
    inline const IndexPair*
    pair(const std::size_t& local) const;

    /**
     * @brief Get an iterator over the indices positioned at the first index.
     * @return Iterator over the local indices.
     */
    inline const_iterator begin() const;

    /**
     * @brief Get an iterator over the indices positioned after the last index.
     * @return Iterator over the local indices.
     */
    inline const_iterator end() const;

    /**
     * @brief Get the internal sequence number.
     *
     * Is initially 0 is incremented for each resize.
     * @return The sequence number.
     */
    inline int seqNo() const;

    /**
     * @brief Get the total number (public and nonpublic) indices.
     * @return The total number (public and nonpublic) indices.
     */
    inline size_t size() const;
  private:
    /**
     * @brief The index set we lookup in.
     */
    const ParallelIndexSet& indexSet_;

    /**
     * @brief The number of indices.
     */
    std::size_t size_;

    /**
     * @brief Array with the positions of the corresponding index pair of the index set.
     */
    std::vector<const IndexPair*> indices_;

  };


  template<typename T>
  struct LocalIndexComparator
  {
    static bool compare([[maybe_unused]] const T& t1, [[maybe_unused]] const T& t2)
    {
      return false;
    }
  };

  template<class TG, class TL>
  struct IndexSetSortFunctor
  {
    bool operator()(const IndexPair<TG,TL>& i1, const IndexPair<TG,TL>& i2)
    {
      return i1.global()<i2.global() || (i1.global()==i2.global() &&
                                         LocalIndexComparator<TL>::compare(i1.local(),
                                                                           i2.local()));
    }
  };



  template<class TG, class TL>
  inline std::ostream& operator<<(std::ostream& os, const IndexPair<TG,TL>& pair)
  {
    os<<"{global="<<pair.global_<<", local="<<pair.local_<<"}";
    return os;
  }

  template<class TG, class TL, int N>
  inline std::ostream& operator<<(std::ostream& os, const ParallelIndexSet<TG,TL,N>& indexSet)
  {
    typedef typename ParallelIndexSet<TG,TL,N>::const_iterator Iterator;
    Iterator end = indexSet.end();
    os<<"{";
    for(Iterator index = indexSet.begin(); index != end; ++index)
      os<<*index<<" ";
    os<<"}";
    return os;

  }

  template<class TG, class TL>
  inline bool operator==(const IndexPair<TG,TL>& a, const IndexPair<TG,TL>& b)
  {
    return a.global_==b.global_;
  }

  template<class TG, class TL>
  inline bool operator!=(const IndexPair<TG,TL>& a, const IndexPair<TG,TL>& b)
  {
    return a.global_!=b.global_;
  }

  template<class TG, class TL>
  inline bool operator<(const IndexPair<TG,TL>& a, const IndexPair<TG,TL>& b)
  {
    return a.global_<b.global_;
  }

  template<class TG, class TL>
  inline bool operator>(const IndexPair<TG,TL>& a, const IndexPair<TG,TL>& b)
  {
    return a.global_>b.global_;
  }

  template<class TG, class TL>
  inline bool operator<=(const IndexPair<TG,TL>& a, const IndexPair<TG,TL>& b)
  {
    return a.global_<=b.global_;
  }

  template<class TG, class TL>
  inline bool operator >=(const IndexPair<TG,TL>& a, const IndexPair<TG,TL>& b)
  {
    return a.global_>=b.global_;
  }

  template<class TG, class TL>
  inline bool operator==(const IndexPair<TG,TL>& a, const TG& b)
  {
    return a.global_==b;
  }

  template<class TG, class TL>
  inline bool operator!=(const IndexPair<TG,TL>& a, const TG& b)
  {
    return a.global_!=b;
  }

  template<class TG, class TL>
  inline bool operator<(const IndexPair<TG,TL>& a, const TG& b)
  {
    return a.global_<b;
  }

  template<class TG, class TL>
  inline bool operator>(const IndexPair<TG,TL>& a, const TG& b)
  {
    return a.global_>b;
  }

  template<class TG, class TL>
  inline bool operator<=(const IndexPair<TG,TL>& a, const TG& b)
  {
    return a.global_<=b;
  }

  template<class TG, class TL>
  inline bool operator >=(const IndexPair<TG,TL>& a, const TG& b)
  {
    return a.global_>=b;
  }

#ifndef DOXYGEN

  template<class TG, class TL>
  IndexPair<TG,TL>::IndexPair(const TG& global, const TL& local)
    : global_(global), local_(local){}

  template<class TG, class TL>
  IndexPair<TG,TL>::IndexPair(const TG& global)
    : global_(global), local_(){}

  template<class TG, class TL>
  IndexPair<TG,TL>::IndexPair()
    : global_(), local_(){}

  template<class TG, class TL>
  inline const TG& IndexPair<TG,TL>::global() const {
    return global_;
  }

  template<class TG, class TL>
  inline TL& IndexPair<TG,TL>::local() {
    return local_;
  }

  template<class TG, class TL>
  inline const TL& IndexPair<TG,TL>::local() const {
    return local_;
  }

  template<class TG, class TL>
  inline void IndexPair<TG,TL>::setLocal(int local){
    local_=local;
  }

  template<class TG, class TL, int N>
  ParallelIndexSet<TG,TL,N>::ParallelIndexSet()
    : state_(GROUND), seqNo_(0), deletedEntries_()
  {}

  template<class TG, class TL, int N>
  void ParallelIndexSet<TG,TL,N>::beginResize()
  {

    // Checks in unproductive code
#ifndef NDEBUG
    if(state_!=GROUND)
      DUNE_THROW(InvalidIndexSetState,
                 "IndexSet has to be in GROUND state, when "
                 << "beginResize() is called!");
#endif

    state_ = RESIZE;
    deletedEntries_ = false;
  }

  template<class TG, class TL, int N>
  inline void ParallelIndexSet<TG,TL,N>::add(const GlobalIndex& global)
  {
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "Indices can only be added "
                 <<"while in RESIZE state!");
#endif
    newIndices_.push_back(IndexPair(global));
  }

  template<class TG, class TL, int N>
  inline void ParallelIndexSet<TG,TL,N>::add(const TG& global, const TL& local)
  {
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "Indices can only be added "
                 <<"while in RESIZE state!");
#endif
    newIndices_.push_back(IndexPair(global,local));
  }

  template<class TG, class TL, int N>
  inline void ParallelIndexSet<TG,TL,N>::markAsDeleted(const iterator& global)
  {
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "Indices can only be removed "
                 <<"while in RESIZE state!");
#endif
    deletedEntries_ = true;

    global.markAsDeleted();
  }

  template<class TG, class TL, int N>
  void ParallelIndexSet<TG,TL,N>::endResize() {
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "endResize called while not "
                 <<"in RESIZE state!");
#endif

    std::sort(newIndices_.begin(), newIndices_.end(), IndexSetSortFunctor<TG,TL>());
    merge();
    seqNo_++;
    state_ = GROUND;
  }


  template<class TG, class TL, int N>
  inline void ParallelIndexSet<TG,TL,N>::merge(){
    if(localIndices_.size()==0)
    {
      localIndices_=newIndices_;
      newIndices_.clear();
    }
    else if(newIndices_.size()>0 || deletedEntries_)
    {
      ArrayList<IndexPair,N> tempPairs;

      auto old = localIndices_.begin();
      auto added = newIndices_.begin();
      const auto endold = localIndices_.end();
      const auto endadded = newIndices_.end();

      while(old != endold && added!= endadded)
      {
        if(old->local().state()==DELETED) {
          old.eraseToHere();
        }
        else
        {
          if(old->global() < added->global() ||
             (old->global() == added->global()
              && LocalIndexComparator<TL>::compare(old->local(),added->local())))
          {
            tempPairs.push_back(*old);
            old.eraseToHere();
            continue;
          }else
          {
            tempPairs.push_back(*added);
            added.eraseToHere();
          }
        }
      }

      while(old != endold)
      {
        if(old->local().state()!=DELETED) {
          tempPairs.push_back(*old);
        }
        old.eraseToHere();
      }

      while(added!= endadded)
      {
        tempPairs.push_back(*added);
        added.eraseToHere();
      }
      localIndices_ = tempPairs;
    }
  }


  template<class TG, class TL, int N>
  inline const IndexPair<TG,TL>&
  ParallelIndexSet<TG,TL,N>::at(const TG& global) const
  {
    // perform a binary search
    int low=0, high=localIndices_.size()-1, probe=-1;

    while(low<high)
    {
      probe = (high + low) / 2;
      if(global <= localIndices_[probe].global())
        high = probe;
      else
        low = probe+1;
    }

    if(probe==-1)
      DUNE_THROW(RangeError, "No entries!");

    if( localIndices_[low].global() != global)
      DUNE_THROW(RangeError, "Could not find entry of "<<global);
    else
      return localIndices_[low];
  }

  template<class TG, class TL, int N>
  inline const IndexPair<TG,TL>&
  ParallelIndexSet<TG,TL,N>::operator[](const TG& global) const
  {
    // perform a binary search
    int low=0, high=localIndices_.size()-1, probe=-1;

    while(low<high)
    {
      probe = (high + low) / 2;
      if(global <= localIndices_[probe].global())
        high = probe;
      else
        low = probe+1;
    }

    return localIndices_[low];
  }
  template<class TG, class TL, int N>
  inline IndexPair<TG,TL>& ParallelIndexSet<TG,TL,N>::at(const TG& global)
  {
    // perform a binary search
    int low=0, high=localIndices_.size()-1, probe=-1;

    while(low<high)
    {
      probe = (high + low) / 2;
      if(localIndices_[probe].global() >= global)
        high = probe;
      else
        low = probe+1;
    }

    if(probe==-1)
      DUNE_THROW(RangeError, "No entries!");

    if( localIndices_[low].global() != global)
      DUNE_THROW(RangeError, "Could not find entry of "<<global);
    else
      return localIndices_[low];
  }

  template<class TG, class TL, int N>
  inline bool ParallelIndexSet<TG,TL,N>::exists (const TG& global) const
  {
    // perform a binary search
    int low=0, high=localIndices_.size()-1, probe=-1;

    while(low<high)
    {
      probe = (high + low) / 2;
      if(localIndices_[probe].global() >= global)
        high = probe;
      else
        low = probe+1;
    }

    if(probe==-1)
      return false;

    if( localIndices_[low].global() != global)
      return false;
    return true;
  }

  template<class TG, class TL, int N>
  inline IndexPair<TG,TL>& ParallelIndexSet<TG,TL,N>::operator[](const TG& global)
  {
    // perform a binary search
    int low=0, high=localIndices_.size()-1, probe=-1;

    while(low<high)
    {
      probe = (high + low) / 2;
      if(localIndices_[probe].global() >= global)
        high = probe;
      else
        low = probe+1;
    }

    return localIndices_[low];
  }
  template<class TG, class TL, int N>
  inline typename ParallelIndexSet<TG,TL,N>::iterator
  ParallelIndexSet<TG,TL,N>::begin()
  {
    return iterator(*this, localIndices_.begin());
  }


  template<class TG, class TL, int N>
  inline typename ParallelIndexSet<TG,TL,N>::iterator
  ParallelIndexSet<TG,TL,N>::end()
  {
    return iterator(*this,localIndices_.end());
  }

  template<class TG, class TL, int N>
  inline typename ParallelIndexSet<TG,TL,N>::const_iterator
  ParallelIndexSet<TG,TL,N>::begin() const
  {
    return localIndices_.begin();
  }


  template<class TG, class TL, int N>
  inline typename ParallelIndexSet<TG,TL,N>::const_iterator
  ParallelIndexSet<TG,TL,N>::end() const
  {
    return localIndices_.end();
  }

  template<class TG, class TL, int N>
  void ParallelIndexSet<TG,TL,N>::renumberLocal(){
#ifndef NDEBUG
    if(state_==RESIZE)
      DUNE_THROW(InvalidIndexSetState, "IndexSet has to be in "
                 <<"GROUND state for renumberLocal()");
#endif

    const auto end_ = end();
    uint32_t index=0;

    for(auto pair=begin(); pair!=end_; index++, ++pair)
      pair->local()=index;
  }

  template<class TG, class TL, int N>
  inline int ParallelIndexSet<TG,TL,N>::seqNo() const
  {
    return seqNo_;
  }

  template<class TG, class TL, int N>
  inline size_t ParallelIndexSet<TG,TL,N>::size() const
  {
    return localIndices_.size();
  }

  template<class I>
  GlobalLookupIndexSet<I>::GlobalLookupIndexSet(const I& indexset,
                                                std::size_t size)
    : indexSet_(indexset), size_(size),
      indices_(size_, static_cast<const IndexPair*>(0))
  {
    const_iterator end_ = indexSet_.end();

    for(const_iterator pair = indexSet_.begin(); pair!=end_; ++pair) {
      assert(pair->local()<size_);
      indices_[pair->local()] = &(*pair);
    }
  }

  template<class I>
  GlobalLookupIndexSet<I>::GlobalLookupIndexSet(const I& indexset)
    : indexSet_(indexset), size_(0)
  {
    const_iterator end_ = indexSet_.end();
    for(const_iterator pair = indexSet_.begin(); pair!=end_; ++pair)
      size_=std::max(size_,static_cast<std::size_t>(pair->local()));

    indices_.resize(++size_,  0);

    for(const_iterator pair = indexSet_.begin(); pair!=end_; ++pair)
      indices_[pair->local()] = &(*pair);
  }

  template<class I>
  GlobalLookupIndexSet<I>::~GlobalLookupIndexSet()
  {}

  template<class I>
  inline const IndexPair<typename I::GlobalIndex, typename I::LocalIndex>*
  GlobalLookupIndexSet<I>::pair(const std::size_t& local) const
  {
    return indices_[local];
  }

  template<class I>
  inline const IndexPair<typename I::GlobalIndex, typename I::LocalIndex>&
  GlobalLookupIndexSet<I>::operator[](const GlobalIndex& global) const
  {
    return indexSet_[global];
  }

  template<class I>
  typename I::const_iterator GlobalLookupIndexSet<I>::begin() const
  {
    return indexSet_.begin();
  }

  template<class I>
  typename I::const_iterator GlobalLookupIndexSet<I>::end() const
  {
    return indexSet_.end();
  }

  template<class I>
  inline size_t GlobalLookupIndexSet<I>::size() const
  {
    return size_;
  }

  template<class I>
  inline int GlobalLookupIndexSet<I>::seqNo() const
  {
    return indexSet_.seqNo();
  }

  template<typename TG, typename TL, int N, typename TG1, typename TL1, int N1>
  bool operator==(const ParallelIndexSet<TG,TL,N>& idxset,
                  const ParallelIndexSet<TG1,TL1,N1>& idxset1)
  {
    if(idxset.size()!=idxset1.size())
      return false;
    typedef typename ParallelIndexSet<TG,TL,N>::const_iterator Iter;
    typedef typename ParallelIndexSet<TG1,TL1,N1>::const_iterator Iter1;
    Iter iter=idxset.begin();
    for(Iter1 iter1=idxset1.begin(); iter1 != idxset1.end(); ++iter, ++iter1) {
      if(iter1->global()!=iter->global())
        return false;
      typedef typename ParallelIndexSet<TG,TL,N>::LocalIndex PI;
      const PI& pi=iter->local(), pi1=iter1->local();

      if(pi!=pi1)
        return false;
    }
    return true;
  }

  template<typename TG, typename TL, int N, typename TG1, typename TL1, int N1>
  bool operator!=(const ParallelIndexSet<TG,TL,N>& idxset,
                  const ParallelIndexSet<TG1,TL1,N1>& idxset1)
  {
    return !(idxset==idxset1);
  }


#endif // DOXYGEN

}
#endif
