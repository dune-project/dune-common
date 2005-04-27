// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_LOCALINDEX_HH
#define DUNE_LOCALINDEX_HH

#include "config.h"

namespace Dune
{


  /** @addtogroup ISTL_Comm
   *
   * @{
   */
  /**
   * @file
   * @brief Provides classes for use as the local index in IndexSet.
   * @author Markus Blatt
   */
  /**
   * @brief The states avaiable for the local indices.
   * @see LocalIndex::state()
   */
  enum LocalIndexState {VALID, DELETED};


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
     * @brief Constructor.
     * @param index The value of the index.
     */
    LocalIndex(uint32_t index) :
      localIndex_(index), state_(VALID){}
    /**
     * @brief get the local index.
     * @return The local index.
     */
    inline const uint32_t& local() const;

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



  inline const uint32_t& LocalIndex::local() const {
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

  /** @} */

} // namespace Dune

#endif
