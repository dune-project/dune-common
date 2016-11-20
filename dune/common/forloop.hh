// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#warning The header dune/common/forloop.hh is deprecated. Use directly "Hybrid::forEach" and include dune/common/hybridutilities.hh.

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <utility>

#include <dune/common/deprecated.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/std/utility.hh>

namespace Dune
{

  template< template< int > class Operation, int first, int last >
  struct DUNE_DEPRECATED_MSG("Use Hybrid::forEach instead!") ForLoop
  {
    static_assert( (first <= last), "ForLoop: first > last" );

    template<typename... Args>
    static void apply(Args&&... args)
    {
      Hybrid::forEach(Std::make_index_sequence<last+1-first>{},
        [&](auto i){Operation<i+first>::apply(args...);});
    }
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
