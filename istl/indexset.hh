// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_INDEXSET__HH
#define DUNE_INDEXSET__HH

#include <algorithm>
#include <dune/common/arraylist.hh>
#include <dune/common/exceptions.hh>
#include <iostream>

namespace Dune
{

  /**
   * @brief The states avaiable for the local indices.
   * @see LocalIndex::state()
   */
  enum LocalIndexState {VALID, DELETED};

  // forward declarations

  template<class TG, class TL>
  class IndexPair;

  template<class TG, class TL>
  std::ostream& operator<<(std::ostream& os, const IndexPair<TG,TL>&);

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

  template<typename T>
  class MPITraits;

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
    friend class MPITraits<IndexPair<TG,TL> >;

  public:
    /**
     * @brief the type of the global index.
     *
     * This type has to provide at least a operator&lt; for sorting.
     */
    typedef TG GlobalIndexType;

    /**
     * @brief the type of the local index.
     *
     * This class to provide the following functions:
     * <pre>
     * LocalIndexType operator=(int);
     * operator int() const;
     * LocalIndexState state() const;
     * void setState(LocalIndexState);
     * </pre>
     */
    typedef TL LocalIndexType;

    /**
     * @brief Constructs a new Pair.
     *
     * @param global The global index.
     * @param local The local index.
     */
    IndexPair(const GlobalIndexType& global, const LocalIndexType& local);

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
    IndexPair(const GlobalIndexType& global);

    /**
     * @brief Get the global index.
     *
     * @return The global index.
     */
    inline const GlobalIndexType& global() const;

    /**
     * @brief Get the local index.
     *
     * @return The local index.
     */
    inline LocalIndexType& local();

    /**
     * @brief Get the local index.
     *
     * @return The local index.
     */
    inline const LocalIndexType& local() const;

    /**
     * @brief Set the local index.
     *
     * @param index The index to set it to.
     */
    inline void setLocal(int index);
  private:
    /** @brief The global index. */
    GlobalIndexType global_;
    /** @brief The local index. */
    LocalIndexType local_;
  };


  /**
   * @brief An index present on the local process.
   */
  class LocalIndex
  {
  public:
    /**
     * @brief Constructor.
     * known to other processes.
     */
    LocalIndex() :
      localIndex_(0), state_(VALID){}

    /**
     * @brief get the local index.
     * @return The local index.
     */
    inline uint32_t local() const;

    /**
     * @brief Convert to the local index represented by an int.
     */
    inline operator uint32_t() const;

    /**
     * @brief Assign a new local index.
     *
     * @param index The new local index.
     */
    inline LocalIndex& operator=(uint32_t index);

    /**
     * @brief Get the state.
     * @return The state.
     */
    inline LocalIndexState state() const;

    /**
     * @brief Set the state.
     * @param state The state to set.
     */
    inline void setState(LocalIndexState state);

  private:
    /** @brief The local index. */
    uint32_t localIndex_;

    /**
     * @brief The state of the index.
     *
     * Has to be one of LocalIndexState!
     * @see LocalIndexState.
     */
    char state_;

  };

  /**
   * @brief The states the index set can be in.
   * @see IndexSet::state_
   */
  enum IndexSetState
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
  class InvalidIndexSetState : public Exception {};

  /**
   * @brief Manager class for the mapping between local indices and globally unique indices.
   *
   * The mapping is between a globally unique id and local index. The local index is consecutive
   * and non persistent while the global id might not be consecutive but definitely is persistent.
   */
  template<typename TG, typename TL, int N=100>
  class IndexSet
  {

  public:
    /**
     * @brief the type of the global index.
     * This type has to provide at least a operator&lt; for sorting.
     */
    typedef TG GlobalIndexType;

    /**
     * @brief The type of the local index.
     *
     * This class to provide the following functions:
     * <pre>
     * LocalIndexType operator=(int);
     * operator int() const;
     * LocalIndexState state() const;
     * void setState(LocalIndexState);
     * </pre>
     */
    typedef TL LocalIndexType;


    enum {
      /**
       * @brief The size of the individual arrays in the underlying ArrayList.
       *
       * The default value is 100.
       * @see ArrayList::size
       */
      arraySize= (N>0) ? N : 1
    };

    /** @brief The iterator over the pairs. */
    typedef typename
    ArrayList<IndexPair<GlobalIndexType,LocalIndexType>,N>::iterator iterator;

    /** @brief The constant iterator over the pairs. */
    typedef typename
    ArrayList<IndexPair<GlobalIndexType,LocalIndexType>,N>::const_iterator
    const_iterator;

    /**
     * @brief Constructor.
     */
    IndexSet();

    /**
     * @brief Get the state the index set is in.
     * @return The state of the index set.
     */
    inline const IndexSetState& state();

    /**
     * @brief Indicate that the index set is to be resized.
     * @exception InvalidState If index set was not in
     * IndexSetState::GROUND mode.
     */
    void beginResize() throw(InvalidIndexSetState);

    /**
     * @brief Add an new index to the set.
     *
     * The local index is created by the default constructor.
     * @param global The globally unique id of the index.
     * @exception InvalidState If index set is not in
     * IndexSetState::RESIZE mode.
     */
    inline void add(const GlobalIndexType& global) throw(InvalidIndexSetState);

    /**
     * @brief Add an new index to the set.
     *
     * @param global The globally unique id of the index.
     * @param local The local index.
     * @exception InvalidState If index set is not in
     * IndexSetState::RESIZE mode.
     */
    inline void add(const GlobalIndexType& global, const LocalIndexType& local)
    throw(InvalidIndexSetState);

    /**
     * @brief Delete an index.
     *
     * If there is no pair with that global index, nothing is done.
     * @param global The globally unique id.
     * @exception InvalidState If index set is not in IndexSetState::RESIZE mode.
     */
    inline void remove(const GlobalIndexType& global)
    throw(InvalidIndexSetState);

    /**
     * @brief Indicate that the resizing finishes.
     * Now every index will get its local id. The local indices will be ordered
     * according to the global indices:
     * Let $(g_i,l_i)_{i=0}^N$ be the set of all indices then $l_i < l_j$
     * if and
     * only if $g_i < g_j$ for arbitrary $i \neq j$.
     * @exception InvalidState If index set was not in
     * IndexSetState::RESIZE mode.
     */
    void endResize() throw(InvalidIndexSetState);

    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefor has complexity
     * N log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @exception NoSuchEntry Thrown if the global id is not known.
     */
    inline IndexPair<GlobalIndexType,LocalIndexType>&
    operator[](const GlobalIndexType& global);


    /**
     * @brief Find the index pair with a specific global id.
     *
     * This starts a binary search for the entry and therefor has complexity
     * N log(N).
     * @param global The globally unique id of the pair.
     * @return The pair of indices for the id.
     * @exception NoSuchEntry Thrown if the global id is not known.
     */
    inline const IndexPair<GlobalIndexType,LocalIndexType>&
    operator[](const GlobalIndexType& global) const;

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
     * $(g_i,l_i)$, $(g_j,l_j)$ be two arbituary index
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
     * @brief Get the number of indices which are public.
     * @return The number of indices which are public.
     */
    inline int noPublic() const;

    /**
     * @brief Get the total number (public and nonpublic) indices.
     * @return The total number (public and nonpublic) indices.
     */
    inline int size() const;

  private:
    /** @brief The index pairs. */
    ArrayList<IndexPair<GlobalIndexType,LocalIndexType>,N> localIndices_;
    /** @brief The new indices for the RESIZE state. */
    ArrayList<IndexPair<GlobalIndexType,LocalIndexType>,N> newIndices_;
    /** @brief The state of the index set. */
    IndexSetState state_;
    /** @brief Number to keep track of the number of resizes. */
    int seqNo_;
    /** @brief Number of public indices. */
    int noPublic_;
    /**
     * @brief Merges the _localIndices and newIndices arrays and creates a new
     * localIndices array.
     */
    inline void merge();
  };

  template<class TG, class TL>
  inline std::ostream& operator<<(std::ostream& os, const IndexPair<TG,TL>& pair)
  {
    os<<"{global="<<pair.global_<<", local="<<pair.local_<<"}";
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

  inline uint32_t LocalIndex::local() const {
    return localIndex_;
  }

  inline LocalIndex::operator uint32_t() const {
    return localIndex_;
  }

  inline LocalIndex& LocalIndex::operator=(uint32_t index){
    localIndex_ = index;
    return *this;
  }

  inline LocalIndexState LocalIndex::state() const {
    return static_cast<LocalIndexState>(state_);
  }

  inline void LocalIndex::setState(LocalIndexState state){
    state_ = static_cast<char>(state);
  }

  template<class TG, class TL, int N>
  IndexSet<TG,TL,N>::IndexSet()
    : state_(GROUND), seqNo_(0), noPublic_(0)
  {}

  template<class TG, class TL, int N>
  void IndexSet<TG,TL,N>::beginResize() throw(InvalidIndexSetState)
  {

    // Checks in unproductive code
#ifndef NDEBUG
    if(state_!=GROUND)
      DUNE_THROW(InvalidIndexSetState,
                 "IndexSet has to be in GROUND state, when "
                 << "beginResize() is called!");
#endif

    state_ = RESIZE;
    /*
       for(iterator iter=localIndices_.begin(), end=localIndices_.end();
        iter != end; ++iter)
       {
        iter->local().setState(OLD);
       }
     */
  }

  template<class TG, class TL, int N>
  inline void IndexSet<TG,TL,N>::add(const GlobalIndexType& global)
  throw(InvalidIndexSetState)
  {
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "Indices can only be added "
                 <<"while in RESIZE state!");
#endif
    newIndices_.push_back(IndexPair<TG,TL>(global));
  }

  template<class TG, class TL, int N>
  inline void IndexSet<TG,TL,N>::add(const TG& global, const TL& local)
  throw(InvalidIndexSetState)
  {
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "Indices can only be added "
                 <<"while in RESIZE state!");
#endif
    newIndices_.push_back(IndexPair<TG,TL>(global,local));
  }

  template<class TG, class TL, int N>
  inline void IndexSet<TG,TL,N>::remove(const TG& global)
  throw(InvalidIndexSetState){
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "Indices can only be removed "
                 <<"while in RESIZE state!");
#endif
    operator[](global).setState(DELETED);
  }

  template<class TG, class TL, int N>
  void IndexSet<TG,TL,N>::endResize() throw(InvalidIndexSetState){
    // Checks in unproductive code
#ifndef NDEBUG
    if(state_ != RESIZE)
      DUNE_THROW(InvalidIndexSetState, "endResize called while not "
                 <<"in RESIZE state!");
#endif

    std::sort(newIndices_.begin(), newIndices_.end());
    merge();
    seqNo_++;
    state_ = GROUND;
  }

  template<class TG, class TL, int N>
  inline void IndexSet<TG,TL,N>::merge(){
    if(localIndices_.size()==0)
    {
      iterator added=newIndices_.begin();
      const const_iterator endadded=newIndices_.end();

      while(added!= endadded) {
        if(added->local().isPublic())
          noPublic_++;
        ++added;
      }

      localIndices_=newIndices_;
      newIndices_.clear();
    }
    else if(newIndices_.size()>0)
    {
      ArrayList<IndexPair<TG,TL>,N> tempPairs;
      iterator old=localIndices_.begin();
      iterator added=newIndices_.begin();
      const const_iterator endold=localIndices_.end();
      const const_iterator endadded=newIndices_.end();

      noPublic_=0;

      while(old != endold && added!= endadded)
      {
        if(old->local().state()==DELETED) {
          old.eraseToHere();
        }
        else if(old->global() < added->global())
        {
          tempPairs.push_back(*old);
          if(old->local().isPublic())
            noPublic_++;
          old.eraseToHere();
        }
        else
        {
          tempPairs.push_back(*added);
          if(added->local().isPublic())
            noPublic_++;
          added.eraseToHere();
        }
      }

      while(old != endold)
      {
        if(old->local().state()!=DELETED) {
          if(old->local().isPublic())
            noPublic_++;
          tempPairs.push_back(*old);
        }
        old.eraseToHere();
      }

      while(added!= endadded)
      {
        tempPairs.push_back(*added);
        if(added->local().isPublic())
          noPublic_++;
        added.eraseToHere();
      }
      localIndices_ = tempPairs;
    }
  }


  template<class TG, class TL, int N>
  inline const IndexPair<TG,TL>&
  IndexSet<TG,TL,N>::operator[](const TG& global) const
  {
    // perform a binary search
    int low=0, high=localIndices_.size(), probe=-1;

    while(low<high)
    {
      probe = (high + low) / 2;
      if(global <= localIndices_[probe].global())
        high = probe;
      else
        low = probe+1;
    }

#ifdef DUNE_ISTL_WITH_CHECKING
    if(probe==-1)
      DUNE_THROW(RangeError, "No entries!");

    if( localIndices_[probe].global() != global)
      DUNE_THROW(RangeError, "Could not find entry of "<<global);
    else
#endif
    return localIndices_[low];
  }


  template<class TG, class TL, int N>
  inline IndexPair<TG,TL>& IndexSet<TG,TL,N>::operator[](const TG& global)
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

#ifdef DUNE_ISTL_WITH_CHECKING
    if(probe==-1)
      DUNE_THROW(RangeError, "No entries!");

    if( localIndices_[probe].global() != global)
      DUNE_THROW(RangeError, "Could not find entry of "<<global);
    else
#endif
    return localIndices_[low];
  }

  template<class TG, class TL, int N>
  inline typename ArrayList<IndexPair<TG,TL>,N>::iterator
  IndexSet<TG,TL,N>::begin()
  {
    return localIndices_.begin();
  }


  template<class TG, class TL, int N>
  inline typename ArrayList<IndexPair<TG,TL>,N>::iterator
  IndexSet<TG,TL,N>::end()
  {
    return localIndices_.end();
  }

  template<class TG, class TL, int N>
  inline typename ArrayList<IndexPair<TG,TL>,N>::const_iterator
  IndexSet<TG,TL,N>::begin() const
  {
    return localIndices_.begin();
  }


  template<class TG, class TL, int N>
  inline typename ArrayList<IndexPair<TG,TL>,N>::const_iterator
  IndexSet<TG,TL,N>::end() const
  {
    return localIndices_.end();
  }

  template<class TG, class TL, int N>
  void IndexSet<TG,TL,N>::renumberLocal(){
#ifndef NDEBUG
    if(state_==RESIZE)
      DUNE_THROW(InvalidIndexSetState, "IndexSet has to be in "
                 <<"GROUND state for renumberLocal()");
#endif

    typedef typename ArrayList<IndexPair<TG,TL>,N>::iterator iterator;
    const const_iterator end_ = end();
    uint32_t index=0;

    for(iterator pair=begin(); pair!=end_; index++, ++pair)
      pair->local()=index;
  }

  template<class TG, class TL, int N>
  inline int IndexSet<TG,TL,N>::seqNo() const
  {
    return seqNo_;
  }

  template<class TG, class TL, int N>
  inline int IndexSet<TG,TL,N>::noPublic() const
  {
    return noPublic_;
  }

  template<class TG, class TL, int N>
  inline int IndexSet<TG,TL,N>::size() const
  {
    return localIndices_.size();
  }
}
#endif
