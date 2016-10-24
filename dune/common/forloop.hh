#warning The header dune/common/forloop.hh is deprecated. Use directly "Hibryd::forEach" and include "dune/common/hybridutilities.hh".

#ifndef DUNE_COMMON_FORLOOP_HH
#define DUNE_COMMON_FORLOOP_HH

#include <utility>

#include <dune/common/unused.hh>

namespace Dune
{
  template< template< class, class > class Operation, template< int > class Value, int first, int last >
  class GenericForLoop
    : public Operation< Value< first >, GenericForLoop< Operation, Value, first+1, last > >
  {
    static_assert( (first <= last), "GenericForLoop: first > last" );
  };

  template< template< class, class > class Operation, template< int > class Value, int last >
  class GenericForLoop< Operation, Value, last, last >
    : public Value< last >
  {};

  namespace ForLoopHelper
  {

    template< class A, class B >
    struct Apply
    {

      template< typename... Params >
      static void apply ( Params&&... params )
      {
        A::apply( std::forward<Params>(params)... );
        B::apply( std::forward<Params>(params)... );
      }

    };

  }

  template< template< int > class Operation, int first, int last >
  class DUNE_DEPRECATED_MSG("Use Hybrid::forEach instead!") ForLoop
    : public GenericForLoop< ForLoopHelper::Apply, Operation, first, last >
  {
    static_assert( (first <= last), "ForLoop: first > last" );
  };

}

#endif // #ifndef DUNE_COMMON_FORLOOP_HH
