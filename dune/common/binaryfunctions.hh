// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BINARYFUNCTIONS_HH
#define DUNE_BINARYFUNCTIONS_HH

/** \file
 * \brief helper classes to provide unique types for standard functions
 */

#include <dune/common/deprecated.hh>

#include <algorithm>

namespace Dune
{
  template<typename Type>
  struct Min
  {
    using first_argument_type DUNE_DEPRECATED_MSG("This type alias is deprecated following similar deprecations in C++17") = Type;

    using second_argument_type DUNE_DEPRECATED_MSG("This type alias is deprecated following similar deprecations in C++17") = Type;

    using result_type DUNE_DEPRECATED_MSG("This type alias is deprecated following similar deprecations in C++17") = Type;

    Type operator()(const Type& t1, const Type& t2) const
    {
      return std::min(t1,t2);
    }
  };

  template<typename Type>
  struct Max
  {
    using first_argument_type DUNE_DEPRECATED_MSG("This type alias is deprecated following similar deprecations in C++17") = Type;

    using second_argument_type DUNE_DEPRECATED_MSG("This type alias is deprecated following similar deprecations in C++17") = Type;

    using result_type DUNE_DEPRECATED_MSG("This type alias is deprecated following similar deprecations in C++17") = Type;

    Type operator()(const Type& t1, const Type& t2) const
    {
      return std::max(t1,t2);
    }
  };
}

#endif
