// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTIONSPACE_HH__
#define __DUNE_DISCRETEFUNCTIONSPACE_HH__

#include "../common/functionspace.hh"
#include "basefunctions.hh"

namespace Dune {

  template< class FunctionSpaceType , class GridTemp,
      class DiscreteFunctionSpaceImp, class BaseFunctionSetInter >
  class DiscreteFunctionSpaceInterface
    : public FunctionSpaceType
  {

  public:
    //! the interface defines the type of the BaseFunctionSet
    typedef BaseFunctionSetInter BaseFunctionSetType;
    typedef GridTemp GridType;

    //! Constructor
    DiscreteFunctionSpaceInterface ( GridType & g, int ident ) :
      FunctionSpaceType (ident),
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
    int size ( int level ) const { return asImp().size(level); };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return asImp().mapToGlobal ( en , localNum );
    };

  protected:
    //! Barton-Nackman trick
    DiscreteFunctionSpaceImp &asImp()
    {
      return static_cast<DiscreteFunctionSpaceImp&>(*this);
    }
    const DiscreteFunctionSpaceImp &asImp() const
    {
      return static_cast<const DiscreteFunctionSpaceImp&>(*this);
    }

    //! the corresponding Grid
    const GridType & grid_ ;
  };

} // end namespace Dune

#endif
