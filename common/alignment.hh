// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef __DUNE_ALIGNMENT_HH__
#define __DUNE_ALIGNMENT_HH__
#include <cstddef>

namespace Dune
{

  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @file
   * @brief This file implements a template class to determine alignment
   * requirements at compile time.
   * @author Markus Blatt
   */
  template<class T>
  struct AlignmentStruct
  {
    char c;
    T t;
  };

  template<class T, std::size_t N>
  struct AlignmentHelper
  {
    enum { N2 = sizeof(AlignmentStruct<T>) - sizeof(T) - N };
    char padding1[N];
    T t;
    char padding2[N2];
  };

#define ALIGNMENT_MODULO(a, b)   ((a) % (b) == 0 ? (b) : (a) % (b))
#define ALIGNMENT_MIN(a, b)      (static_cast<std::size_t>(a) <   \
                                  static_cast<std::size_t>(b) ? (a) : (b))

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

  template <class T>
  struct AlignmentTester<T, 0>
  {
    enum
    {
      result = ALIGNMENT_MODULO(sizeof(AlignmentStruct<T>), sizeof(T))
    };
  };

  /**
   * @brief Calculates the alignment properties of a type.
   */
  template <class T>
  struct AlignmentOf
  {
    enum
    {
      /** @brief The alginment requirement. */
      value = AlignmentTester<T, sizeof(AlignmentStruct<T>) - sizeof(T) - 1>::result
    };
  };

  /** @} */
}
#endif
