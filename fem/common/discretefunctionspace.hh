// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DISCRETEFUNCTIONSPACE_HH__
#define __DUNE_DISCRETEFUNCTIONSPACE_HH__

#include <dune/common/functionspace.hh>
#include "basefunctions.hh"

namespace Dune {


  /** @defgroup DiscreteFunctionSpace DiscreteFunctionSpace
     @ingroup DiscreteFunction
     This provides the interfaces for discrete function spaces.

     @{
   */

  enum DFSpaceIdentifier {  LagrangeSpace_id , DGSpace_id , RaviartThomasSpace_id, PerLagrangeSpace_id };


  //**************************************************************************
  //
  //  --DiscreteFunctionSpaceInterface
  //
  //! This is the interface for discrete function spaces. All methods
  //! declared here have to be implemented by the implementation class.
  //! The discrete function space always depends on a given grid.
  //! For all diffrent element types of the grid the function space provides
  //! a set of base functions for the different elements.
  //! Because of the knowledge of on the one hand the grid an on the oterh
  //! the base functions sets, the discrete function space provides the size
  //! of the function space and a mapping from entity and local dof number
  //! to global dof number of the level of the entity.
  //!
  //**************************************************************************
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
      FunctionSpaceType (ident),  grid_ (g) {};

    //! Get base function set for given entity.
    //! For a type of element the base function set is unique.
    template <class EntityType>
    const BaseFunctionSetType &
    getBaseFunctionSet ( EntityType &en ) const
    {
      return asImp().getBaseFunctionSet( en );
    }

    //! return true if we have global continuous discrete functions.
    bool continuous () const
    {
      return asImp().continuous();
    }

    //! Get maximal polynom order of functionspace.
    int polynomOrder () const
    {
      return asImp().polynomOrder();
    }

    //! Get polynom order for special element.
    template <class EntityType>
    int localPolynomOrder ( EntityType &en ) const
    {
      return asImp().localPolynomOrder( en );
    }

    //! return the corresponding Grid
    GridType & getGrid () const { return grid_; }

    //! return number of degrees of freedom for spezified grid and level
    //! depends also on the base function set
    int size ( int level ) const { return asImp().size(level); };

    //! return number of degrees of freedom for spezified grid and leaf
    //! level, depends also on the base function set
    int leafsize ( int level ) const { return asImp().leafsize(level); };

    //! For given entity map local dof number to global dof number
    //! at the level of the given entity.
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
    GridType & grid_ ;
  };

  //**************************************************************************
  //
  // --DiscreteFunctionSpaceDefault
  //
  //! This is the class with default implementations for discrete function
  //! space.
  //!
  //**************************************************************************
  template< class FunctionSpaceType , class GridTemp,
      class DiscreteFunctionSpaceImp, class BaseFunctionSetInter >
  class DiscreteFunctionSpaceDefault
    : public DiscreteFunctionSpaceInterface < FunctionSpaceType , GridTemp,
          DiscreteFunctionSpaceImp, BaseFunctionSetInter>
  {

  public:
    // at the moment nothing
  private:
    //! Barton-Nackman trick
    DiscreteFunctionSpaceImp &asImp()
    {
      return static_cast<DiscreteFunctionSpaceImp&>(*this);
    }
    const DiscreteFunctionSpaceImp &asImp() const
    {
      return static_cast<const DiscreteFunctionSpaceImp&>(*this);
    }

  };

  /** @} end documentation group */

} // end namespace Dune

#endif
