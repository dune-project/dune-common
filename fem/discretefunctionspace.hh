// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTIONSPACE_HH__
#define __DUNE_DISCRETEFUNCTIONSPACE_HH__

#include "../common/functionspace.hh"
#include "basefunctions.hh"

namespace Dune {

  template< typename DomainFieldType, typename RangeFieldType,
      int n, int m, class GridTemp, class DiscreteFunctionSpaceImp >
  class DiscreteFunctionSpace
    : public FunctionSpace < DomainFieldType, RangeFieldType, n, m >
  {

  public:
    typedef BaseFunctionSet< DiscreteFunctionSpaceImp> BaseFunctionSetType;
    typedef GridTemp GridType;

    //! Constructor
    DiscreteFunctionSpace ( GridType & g, int ident ) :
      FunctionSpace<DomainFieldType, RangeFieldType, n, m> (ident),
      grid_ (g) {};

    //! get base function set from derived implementation
    template <class EntityType>
    const BaseFunctionSetType &
    getBaseFunctionSet ( EntityType &en ) const
    {
      return asImp().getBaseFunctionSet( en );
    }

    //! return the corresponding Grid
    const GridType & getGrid () const { return grid_; }

    //! return number of degrees of freedom for spezified grid and level
    //! depends also on the base function set
    int size ( int level ) const { asImp().size(level); };

  protected:
    //! the corresponding Grid
    const GridType & grid_ ;
  };

} // end namespace Dune

#endif
