// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE__DGSPACE_HH__
#define __DUNE__DGSPACE_HH__

#include <dune/grid/common/grid.hh>

#include <dune/fem/common/discretefunctionspace.hh>
#include <dune/fem/dgspace/monomialbase.hh>
#include <dune/fem/dofmanager.hh>

namespace Dune {

  //***************************************************************************
  //
  //!  DG Mapper for mapping of local dof numbers to global dof numbers,
  //!  i.e. the entry in the vector of unknowns
  //
  //***************************************************************************
  template <class IndexSetType, int polOrd>
  class DGMapper
    : public DofMapperDefault < DGMapper <IndexSetType,polOrd> >
  {
    // index set of grid, i.e. the element indices
    IndexSetType &indexSet_;

    // number of dofs on element
    int numberOfDofs_;
    int level_;
  public:
    //! Constructor
    DGMapper ( IndexSetType &iset , int numDof , int level) :
      indexSet_ (iset), numberOfDofs_ (numDof), level_(level)  {};

    //! return size of function space
    //! see dofmanager.hh for definition of IndexSet, which
    //! is a wrapper for en.index
    int size () const
    {
      // return number of dofs * number of elements
      return (numberOfDofs_ * indexSet_.size( level_ , 0 ));
    }

    //! map Entity an local Dof number to global Dof number
    //! see dofmanager.hh for definition of IndexSet, which
    //! is a wrapper for en.index
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      // indexSet_.index<0>(en,0) return 0 entity of codim the codim 0
      // entitys of en which is the element index
      return ((indexSet_.template index<0>(en,0) * numberOfDofs_) + localNum);
    };

    //** size methods needed by the dofmanager ***
    //! calc new start point for element dof , vertex dof ...
    //not needed here at the moment
    virtual void calcInsertPoints () {};

    //! only called once, if grid was adapted
    virtual int newSize() const
    {
      return this->size();
    }
  };

  //**********************************************************************
  //
  //  DiscreteFunctionSpace for discontinous functions
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
    // to be revised, see LagrangeDiscreteFunctionSpace
    DofManagerType dm_;

    // corresponding IndexSet, here LevelIndexSet
    typedef typename DofManagerType::IndexSetType IndexSetType;

    typedef DGMapper < IndexSetType, polOrd > DGMapperType;
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
      DiscreteFunctionSpaceType (g, 123456789),
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
