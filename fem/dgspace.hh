// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__DGSPACE_HH__
#define __DUNE__DGSPACE_HH__

#include <dune/grid/common/grid.hh>

#include <dune/fem/common/discretefunctionspace.hh>
#include <dune/fem/dgspace/monomialbase.hh>
#include <dune/fem/dgspace/dgmapper.hh>
#include <dune/fem/dofmanager.hh>

namespace Dune {

  //**********************************************************************
  //
  //!  DiscreteFunctionSpace for discontinous functions
  //
  //**********************************************************************
  template< class FunctionSpaceType, class GridType, int polOrd , class
      DofManagerType = DofManager<GridType,DefaultGridIndexSet<GridType,LevelIndex> > >
  class DGDiscreteFunctionSpace
    : public DiscreteFunctionSpaceInterface
      <  FunctionSpaceType , GridType,
          DGDiscreteFunctionSpace < FunctionSpaceType , GridType,
              polOrd, DofManagerType >,
          MonomialBaseFunctionSet < FunctionSpaceType > >
  {
    enum { DGFSpaceId = 123456789 };

    // to be revised, see LagrangeDiscreteFunctionSpace
    DofManagerType dm_;

    // corresponding IndexSet, here LevelIndexSet
    typedef typename DofManagerType::IndexSetType IndexSetType;

    enum { DimRange = FunctionSpaceType::DimRange };

    typedef DGMapper < IndexSetType, polOrd, DimRange  > DGMapperType;
  public:
    // MemObjects of the DofManager, DofManager is not used for this space at
    // the moment
    typedef typename DofManagerType::MemObjectType MemObjectType;

    // BaseFunctionSet we are using
    typedef MonomialBaseFunctionSet<FunctionSpaceType> BaseFunctionSetType;

    typedef DGDiscreteFunctionSpace
    < FunctionSpaceType , GridType , polOrd , DofManagerType >
    DGDiscreteFunctionSpaceType;

    typedef DiscreteFunctionSpaceInterface
    <FunctionSpaceType , GridType, DGDiscreteFunctionSpaceType,
        MonomialBaseFunctionSet <FunctionSpaceType> >
    DiscreteFunctionSpaceType;

    /** \todo Please doc me! */
    DGDiscreteFunctionSpace ( GridType & g , int level ) :
      DiscreteFunctionSpaceType (g, DGFSpaceId, level),
      dm_ ( g ), base_(*this, polOrd),
      mapper_(dm_.indexSet(), base_.getNumberOfBaseFunctions(), level)
    {}

    /** \todo Please doc me! */
    template <class DiscFuncType>
    MemObjectType & signIn (DiscFuncType & df)
    {
      // do notin' at the moment
    }

    /** \todo Please doc me! */
    template <class DiscFuncType>
    bool signOut (DiscFuncType & df)
    {
      return false;
    }

    /** \todo Please doc me! */
    virtual ~DGDiscreteFunctionSpace () {}

    /** \todo Please doc me! */
    DFSpaceIdentifier type () const
    {
      return DGSpace_id;
    }

    //! Get base function set for given entity.
    //! For a type of element the base function set is unique.
    template <class EntityType>
    const BaseFunctionSetType &
    getBaseFunctionSet ( EntityType &en ) const
    {
      return base_;
    }

    //! return true if we have continuous discrete functions
    bool continuous ( ) const
    {
      return false;
    }

    //! get maximal global polynom order
    int polynomOrder ( ) const
    {
      return polOrd;
    }

    //! get local polynom order on entity
    template <class EntityType>
    int localPolynomOrder ( EntityType &en ) const
    {
      return polOrd;
    }

    //! length of the dof vector
    //! size knows the correct way to calculate the size of the functionspace
    int size () const
    {
      return mapper_.size ();
    };

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const
    {
      return mapper_.mapToGlobal ( en , localNum );
    };

  private:
    BaseFunctionSetType base_;
    // mapper for function space
    DGMapperType mapper_;
  };

} // end namespace Dune

#endif
