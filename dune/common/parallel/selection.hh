// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_SELECTION_HH
#define DUNE_SELECTION_HH

#include "indexset.hh"
#include <dune/common/iteratorfacades.hh>

namespace Dune
{
  /** @addtogroup Common_Parallel
   *
   * @{
   */
  /**
   * @file
   * @brief Provides classes for selecting
   * indices based on attribute flags.
   * @author Markus Blatt
   */

  /**
   * @brief A const iterator over an uncached selection.
   */
  template<typename TS, typename TG, typename TL, int N>
  class SelectionIterator
  {
  public:
    /**
     * @brief The type of the Set of attributes.
     *
     * It has to provide a static method
     * \code bool contains(AttributeType a); \endcode
     * that returns true if a is in the set.
     * Such types are EnumItem, EnumRange, Combine.
     */
    typedef TS AttributeSet;

    /**
     * @brief The type of the underlying index set.
     */
    typedef Dune::ParallelIndexSet<TG,TL,N> ParallelIndexSet;

    //typedef typename ParallelIndexSet::const_iterator ParallelIndexSetIterator;

    typedef ConstArrayListIterator<IndexPair<TG,TL>, N, std::allocator<Dune::IndexPair<TG,TL> > > ParallelIndexSetIterator;
    /**
     * @brief Constructor.
     * @param iter The iterator over the index set.
     * @param end The iterator over the index set positioned at the end.
     */
    SelectionIterator(const ParallelIndexSetIterator& iter, const ParallelIndexSetIterator& end)
      : iter_(iter), end_(end)
    {
      // Step to the first valid entry
      while(iter_!=end_ && !AttributeSet::contains(iter_->local().attribute()))
        ++iter_;
    }

    void operator++()
    {
      assert(iter_!=end_);
      for(++iter_; iter_!=end_; ++iter_)
        if(AttributeSet::contains(iter_->local().attribute()))
          break;
    }


    uint32_t operator*() const
    {
      return iter_->local().local();
    }

    bool operator==(const SelectionIterator<TS,TG,TL,N>& other) const
    {
      return iter_ == other.iter_;
    }

    bool operator!=(const SelectionIterator<TS,TG,TL,N>& other) const
    {
      return iter_ != other.iter_;
    }

  private:
    ParallelIndexSetIterator iter_;
    const ParallelIndexSetIterator end_;
  };


  /**
   * @brief An uncached selection of indices.
   */
  template<typename TS, typename TG, typename TL, int N>
  class UncachedSelection
  {
  public:
    /**
     * @brief The type of the Set of attributes.
     *
     * It has to provide a static method
     * \code bool contains(AttributeType a); \endcode
     * that returns true if a is in the set.
     * Such types are EnumItem, EnumRange, Combine.
     */
    typedef TS AttributeSet;

    /**
     * @brief The type of the global index of the underlying index set.
     */
    typedef TG GlobalIndex;

    /**
     * @brief The type of the local index of the underlying index set.
     *
     * It has to provide a function
     * \code AttributeType attribute(); \endcode
     */
    typedef TL LocalIndex;

    /**
     * @brief The type of the underlying index set.
     */
    typedef Dune::ParallelIndexSet<GlobalIndex,LocalIndex,N> ParallelIndexSet;

    /**
     * @brief The type of the iterator of the selected indices.
     */
    typedef SelectionIterator<TS,TG,TL,N> iterator;

    /**
     * @brief The type of the iterator of the selected indices.
     */
    typedef iterator const_iterator;

    UncachedSelection()
      : indexSet_()
    {}

    UncachedSelection(const ParallelIndexSet& indexset)
      : indexSet_(&indexset)
    {}
    /**
     * @brief Set the index set of the selection.
     * @param indexset The index set to use.
     */
    void setIndexSet(const ParallelIndexSet& indexset);

    /**
     * @brief Get the index set we are a selection for.
     */
    //const ParallelIndexSet& indexSet() const;

    /**
     * @brief Get an iterator over the selected indices.
     * @return An iterator positioned at the first selected index.
     */
    const_iterator begin() const;

    /**
     * @brief Get an iterator over the selected indices.
     * @return An iterator positioned at the first selected index.
     */
    const_iterator end() const;


  private:
    const ParallelIndexSet* indexSet_;

  };

  /**
   * @brief A cached selection of indices.
   */
  template<typename TS, typename TG, typename TL, int N>
  class Selection
  {
  public:
    /**
     * @brief The type of the set of attributes.
     *
     * It has to provide a static method
     * \code bool contains(AttributeType a); \endcode
     * that returns true if a is in the set.
     * Such types are EnumItem, EnumRange, Combine.
     */
    typedef TS AttributeSet;

    /**
     * @brief The type of the global index of the underlying index set.
     */
    typedef TG GlobalIndex;

    /**
     * @brief The type of the local index of the underlying index set.
     *
     * It has to provide a function
     * \code AttributeType attribute(); \endcode
     */
    typedef TL LocalIndex;

    /**
     * @brief The type of the underlying index set.
     */
    typedef Dune::ParallelIndexSet<GlobalIndex,LocalIndex,N> ParallelIndexSet;

    /**
     * @brief The type of the iterator of the selected indices.
     */
    typedef uint32_t* iterator;

    /**
     * @brief The type of the iterator of the selected indices.
     */
    typedef uint32_t* const_iterator;

    Selection()
      : selected_()
    {}

    Selection(const ParallelIndexSet& indexset)
      : selected_(), size_(0), built_(false)
    {
      setIndexSet(indexset);
    }

    ~Selection();

    /**
     * @brief Set the index set of the selection.
     * @param indexset The index set to use.
     */
    void setIndexSet(const ParallelIndexSet& indexset);

    /**
     * @brief Free allocated memory.
     */
    void free();

    /**
     * @brief Get the index set we are a selection for.
     */
    //IndexSet indexSet() const;

    /**
     * @brief Get an iterator over the selected indices.
     * @return An iterator positioned at the first selected index.
     */
    const_iterator begin() const;

    /**
     * @brief Get an iterator over the selected indices.
     * @return An iterator positioned at the first selected index.
     */
    const_iterator end() const;


  private:
    uint32_t* selected_;
    size_t size_;
    bool built_;

  };

  template<typename TS, typename TG, typename TL, int N>
  inline void Selection<TS,TG,TL,N>::setIndexSet(const ParallelIndexSet& indexset)
  {
    if(built_)
      free();

    // Count the number of entries the selection has to hold
    typedef typename ParallelIndexSet::const_iterator const_iterator;
    const const_iterator end = indexset.end();
    int entries = 0;

    for(const_iterator index = indexset.begin(); index != end; ++index)
      if(AttributeSet::contains(index->local().attribute()))
        ++entries;

    selected_ = new uint32_t[entries];
    built_ = true;

    entries = 0;
    for(const_iterator index = indexset.begin(); index != end; ++index)
      if(AttributeSet::contains(index->local().attribute()))
        selected_[entries++]= index->local().local();

    size_=entries;
    built_=true;
  }

  template<typename TS, typename TG, typename TL, int N>
  uint32_t* Selection<TS,TG,TL,N>::begin() const
  {
    return selected_;
  }

  template<typename TS, typename TG, typename TL, int N>
  uint32_t* Selection<TS,TG,TL,N>::end() const
  {
    return selected_+size_;
  }

  template<typename TS, typename TG, typename TL, int N>
  inline void Selection<TS,TG,TL,N>::free()
  {
    delete[] selected_;
    size_=0;
    built_=false;
  }

  template<typename TS, typename TG, typename TL, int N>
  inline Selection<TS,TG,TL,N>::~Selection()
  {
    if(built_)
      free();
  }

  template<typename TS, typename TG, typename TL, int N>
  SelectionIterator<TS,TG,TL,N> UncachedSelection<TS,TG,TL,N>::begin() const
  {
    return SelectionIterator<TS,TG,TL,N>(indexSet_->begin(),
                                         indexSet_->end());
  }

  template<typename TS, typename TG, typename TL, int N>
  SelectionIterator<TS,TG,TL,N> UncachedSelection<TS,TG,TL,N>::end() const
  {
    return SelectionIterator<TS,TG,TL,N>(indexSet_->end(),
                                         indexSet_->end());
  }
  template<typename TS, typename TG, typename TL, int N>
  void UncachedSelection<TS,TG,TL,N>::setIndexSet(const ParallelIndexSet& indexset)
  {
    indexSet_ = &indexset;
  }

  /** @} */


}
#endif
