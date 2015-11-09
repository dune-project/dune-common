// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALIGNMENT_HH
#define DUNE_ALIGNMENT_HH

#warning The header dune/common/alignment.hh is deprecated. Use alignof(T) instead of Dune::AlignmentOf<T>::value.

#include <type_traits>

namespace Dune
{

  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief This file implements a template class to determine alignment
   * requirements of types at compile time.
   * @author Markus Blatt
   */

  /**
   * @brief Calculates the alignment requirement of a type.
   *
   * @see http://en.wikipedia.org/wiki/Data_structure_alignment
   *
   * This will be a safe value and not an optimal one.
   * If TR1 is available it falls back to std::alignment_of.
   */
  template <class T>
  struct AlignmentOf
  {

    enum
    {
      /** @brief The alignment requirement. */
      value = std::alignment_of<T>::value
    };
  };

  /** @} */
}
#endif
