// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FUNCTIONSPACE_HH__
#define __DUNE_FUNCTIONSPACE_HH__

#include "../common/matvec.hh"
#include "../common/functionspace.hh"

namespace Dune {

  template< typename DomainFieldType, typename RangeFieldType,
      int n, int m, class GridType, class DiscreteFunctionSpaceImp >
  class DiscreteFunctionSpace
    : public FunctionSpace < DomainFieldType, RangeFieldType, n, m >
  {

  public:

    DiscreteFunctionSpace ( GridType & g, int ident ) :
      FunctionSpace<DomainFieldType, RangeFieldType, n, m> (ident),
      grid_ (g) {};

    template <class EntityType>
    const BaseFunctionSet < DiscreteFunctionSpaceImp > & getBaseFunctionSet ( EntityType &en ) const {
      return asImp().getBaseFunctionSet( en );
    }

  protected:
    const GridType & grid_ ;
  };

}

#endif
