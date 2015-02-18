// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALIGNMENT_HH
#define DUNE_ALIGNMENT_HH

#include <cstddef>
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

  namespace
  {

    /**
     * @brief Helper class to measure alignment requirement.
     * @tparam T The type we want to measure the alignment requirement for.
     */
    template<class T>
    struct AlignmentStruct
    {
      char c;
      T t;
      void hack();
    };

    /**
     * @brief Helper class to measure alignment requirement.
     * @tparam T The type we want to measure the alignment requirement for.
     */
    template<class T, std::size_t N>
    struct AlignmentHelper
    {
      enum { N2 = sizeof(AlignmentStruct<T>) - sizeof(T) - N };
      char padding1[N];
      T t;
      char padding2[N2];
    };

#define ALIGNMENT_MODULO(a, b)   (a % b == 0 ? \
                                  static_cast<std::size_t>(b) : \
                                  static_cast<std::size_t>(a % b))
#define ALIGNMENT_MIN(a, b)      (static_cast<std::size_t>(a) <   \
                                  static_cast<std::size_t>(b) ? \
                                  static_cast<std::size_t>(a) : \
                                  static_cast<std::size_t>(b))
    /**  @brief does the actual calculations. */
    template <class T, std::size_t N>
    struct AlignmentTester
    {
      typedef AlignmentStruct<T>        s;
      typedef AlignmentHelper<T, N>     h;
      typedef AlignmentTester<T, N - 1> next;
      enum
      {
        a1       = ALIGNMENT_MODULO(N        , sizeof(T)),
        a2       = ALIGNMENT_MODULO(h::N2    , sizeof(T)),
        a3       = ALIGNMENT_MODULO(sizeof(h), sizeof(T)),
        a        = sizeof(h) == sizeof(s) ? ALIGNMENT_MIN(a1, a2) : a3,
        result   = ALIGNMENT_MIN(a, next::result)
      };
    };

    /**  @brief does the actual calculations. */
    template <class T>
    struct AlignmentTester<T, 0>
    {
      enum
      {
        result = ALIGNMENT_MODULO(sizeof(AlignmentStruct<T>), sizeof(T))
      };
    };
  } //end anonymous namespace

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
