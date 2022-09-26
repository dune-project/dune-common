// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_LOCALINDEX_HH
#define DUNE_COMMON_LOCALINDEX_HH

#include <cstddef>

namespace Dune
{


  /** @addtogroup Common_Parallel
   *
   * @{
   */
  /**
   * @file
   * @brief Provides classes for use as the local index in ParallelIndexSet.
   * @author Markus Blatt
   */
  /**
   * @brief The states available for the local indices.
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
    LocalIndex(std::size_t index) :
      localIndex_(index), state_(VALID){}
    /**
     * @brief get the local index.
     * @return The local index.
     */
    inline const std::size_t& local() const;

    /**
     * @brief Convert to the local index represented by an int.
     */
    inline operator std::size_t() const;

    /**
     * @brief Assign a new local index.
     *
     * @param index The new local index.
     */
    inline LocalIndex& operator=(std::size_t index);

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
    std::size_t localIndex_;

    /**
     * @brief The state of the index.
     *
     * Has to be one of LocalIndexState!
     * @see LocalIndexState.
     */
    char state_;

  };



  inline const std::size_t& LocalIndex::local() const {
    return localIndex_;
  }

  inline LocalIndex::operator std::size_t() const {
    return localIndex_;
  }

  inline LocalIndex& LocalIndex::operator=(std::size_t index){
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
