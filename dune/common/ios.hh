// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_IOS_HH
#define DUNE_IOS_HH

#include <ios>

#include <dune/common/deprecated.hh>

namespace Dune {
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief  Utility class for storing and restting old std::ios flags.
   * @author Markus Blatt
   */
  /**
   * @brief Utility class for storing and restting old std::ios flags.
   *
   * The constructor stores the flags currently set in the ios and the
   * destructor sets these flags  in the ios object again.
   *
   * \deprecated Use ios_base_all_saver instead.
   */
  class DUNE_DEPRECATED IosFlagsRestorer
  {
  public:
    /**
     * @brief Constructor that stores the currently used flags.
     * @param ios_ The ios object whose flags are to be saved and restored.
     */
    IosFlagsRestorer(std::ios_base& ios_)
      : ios(ios_), oldflags(ios.flags())
    {}

    /**
     * @brief Destructor that restores the flags stored by the constructor.
     */
    ~IosFlagsRestorer()
    {
      ios.flags(oldflags);
    }
  private:
    /** @brief the ios object to restore the flags to.*/
    std::ios_base& ios;
    /** @brief The flags used when the constructor was called. */
    std::ios_base::fmtflags oldflags;
  };

  /** }@ */
}

#endif
