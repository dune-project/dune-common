// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_IOS_STATE_HH
#define DUNE_COMMON_IOS_STATE_HH

#include <ios>

namespace Dune {
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief  Utility class for storing and resetting stream attributes.
   * @author Markus Blatt
   */
  /**
   * @brief Utility class for storing and resetting stream attributes.
   *
   * The constructor saves the attributes currently set in the ios_base
   * object and the destructor restores these attributes again.  The
   * attributes can also be restores at any time by calling the method
   * restore().
   *
   * The saved attributes are the format flags, precision, and width.
   *
   * @note The interface of this class is meant to be drop-in compatible the
   *       the class of the same name from <boost/io/ios_state.hpp>.
   */
  class ios_base_all_saver
  {
  public:
    /** @brief Export type of object we save the state for */
    typedef std::ios_base state_type;

    /**
     * @brief Constructor that stores the currently used flags.
     * @param ios_ The ios_base object whose flags are to be saved and
     *             restored.  Any stream object should work here.
     *
     * @note A reference to the ios_base object is store in this object.  Thus
     *       the ios_base object must remain valid until the destructor of
     *       this object has been called.
     */
    ios_base_all_saver(state_type& ios_);

    /**
     * @brief Destructor that restores the flags stored by the constructor.
     */
    ~ios_base_all_saver();

    /**
     * @brief Restore flags now
     *
     * The flags will also be restored at destruction time even if this method
     * was used.
     */
    void restore();

  private:
    /** @brief the ios object to restore the flags to. */
    state_type& ios;
    /** @brief The flags used when the constructor was called. */
    state_type::fmtflags oldflags;
    /** @brief The precision in use when the constructor was called. */
    std::streamsize oldprec;
    /** @brief The width in use when the constructor was called. */
    std::streamsize oldwidth;
  };

  /** }@ */
}

#endif // DUNE_COMMON_IOS_STATE_HH
