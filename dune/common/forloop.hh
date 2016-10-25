#warning The header dune/common/forloop.hh is deprecated. Use directly "Hibryd::forEach" and include "dune/common/hybridutilities.hh".

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <utility>

#include <dune/common/hybridutilities.hh>
#include <dune/common/std/utility.hh>
#include <dune/common/unused.hh>

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
        [&](auto i){Operation<i>::apply(std::forward<Args>(args)...);});
    }
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
