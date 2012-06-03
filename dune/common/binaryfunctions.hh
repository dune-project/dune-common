// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BINARYFUNCTIONS_HH
#define DUNE_BINARYFUNCTIONS_HH

/** \file
 * \brief Various helper classes derived from from std::binary_function for
 *    stl-style functional programming
 */
#include <functional>
#include <algorithm>

namespace Dune
{
  template<typename Type>
  struct Min
    : std::binary_function<Type,Type,Type>
  {
    Type operator()(const Type& t1, const Type& t2) const
    {
      return std::min(t1,t2);
    }
  };

  template<typename Type>
  struct Max
    : std::binary_function<Type,Type,Type>
  {
    Type operator()(const Type& t1, const Type& t2) const
    {
      return std::max(t1,t2);
    }
  };
}

#endif
