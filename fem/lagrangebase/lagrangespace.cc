// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGESPACE_CC
#define DUNE_LAGRANGESPACE_CC

#include <algorithm>

namespace Dune {

  // Constructor
  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  LagrangeDiscreteFunctionSpace (GridPartType & g, DofManagerType & dm) :
    DefaultType(id),
    dm_(dm),
    grid_(g)
  {
    makeFunctionSpace();
  }


  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline void LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  makeFunctionSpace ()
  {
    // add index set to list of indexset of dofmanager
    dm_.addIndexSet( grid_.grid() , grid_.indexSet() );

    mapper_ = 0;
    maxNumBase_ = 0;

    //std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";
    for(int i=0; i<numOfDiffBase_; i++)
      baseFuncSet_[i] = 0;

    {
      // search the macro grid for diffrent element types
      typedef typename GridType::template Codim<0>::LevelIterator LevelIteratorType;
      IteratorType endit  = end();
      for(IteratorType it = begin(); it != endit; ++it) {
        GeometryType type = (*it).geometry().type(); // Hack
        if(baseFuncSet_[type] == 0 )
          baseFuncSet_[type] = setBaseFuncSetPointer(*it);
      }
    }

    for(int i=0; i<numOfDiffBase_; i++)
    {
      if(baseFuncSet_[i])
        maxNumBase_ = std::max(baseFuncSet_[i]->getNumberOfBaseFunctions(),maxNumBase_);
    }

    // for empty functions space which can happen for BSGrid
    if(!mapper_) makeBaseSet<line,0> ();
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  ~LagrangeDiscreteFunctionSpace ()
  {
    for(int i=0; i<numOfDiffBase_; i++)
      if (baseFuncSet_[i] != 0)
        delete baseFuncSet_[i];

    delete(mapper_);
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline DFSpaceIdentifier LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  type () const
  {
    return LagrangeSpace_id;
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template <class EntityType>
  inline const
  typename LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::BaseFunctionSetType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  getBaseFunctionSet (EntityType &en) const
  {
    GeometryType type =  en.geometry().type();
    return *baseFuncSet_[type];
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class EntityType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  evaluateLocal ( int baseFunc, EntityType &en, DomainType &local, RangeType & ret) const
  {
    const BaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , local , ret);
    return (polOrd != 0);
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class EntityType, class QuadratureType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  evaluateLocal (  int baseFunc, EntityType &en, QuadratureType &quad,
                   int quadPoint, RangeType & ret) const
  {
    const BaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , quad, quadPoint , ret);
    return (polOrd != 0);
  }


  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  maxNumberBase () const
  {
    return maxNumBase_;
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  size () const
  {
    return mapper_->size ();
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class EntityType>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  mapToGlobal ( EntityType &en, int localNum ) const
  {
    return mapper_->mapToGlobal ( en , localNum );
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class DiscFuncType >
  inline typename DiscFuncType :: MemObjectType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  signIn (DiscFuncType & df) const
  {
    // only for gcc to pass type DofType
    assert(mapper_ != 0);
    return dm_.addDofSet( df.getStorage() , grid_.grid() , grid_.indexSet() , *mapper_, df.name() );
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class DiscFuncType>
  inline bool
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  signOut (DiscFuncType & df) const
  {
    return dm_.removeDofSet( df.memObj() );
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class EntityType>
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  BaseFunctionSetType*
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  setBaseFuncSetPointer ( EntityType &en )
  {
    switch (en.geometry().type())
    {
    case line         : return makeBaseSet<line,polOrd> ();
    case triangle     : return makeBaseSet<triangle,polOrd> ();
    case quadrilateral : return makeBaseSet<quadrilateral,polOrd> ();
    case tetrahedron  : return makeBaseSet<tetrahedron,polOrd> ();
    case pyramid      : return makeBaseSet<pyramid,polOrd> ();
    case prism        : return makeBaseSet<prism,polOrd> ();
    case hexahedron   : return makeBaseSet<hexahedron,polOrd> ();

    case simplex :
      switch (EntityType::dimension) {
      case 1 : return makeBaseSet<line,polOrd> ();
      case 2 : return makeBaseSet<triangle,polOrd> ();
      case 3 : return makeBaseSet<tetrahedron,polOrd> ();
      default :
        DUNE_THROW(NotImplemented, "No Lagrange function spaces for simplices of dimension "
                   << EntityType::dimension << "!");
      }

    case cube :
      switch (EntityType::dimension) {
      case 1 : return makeBaseSet<line,polOrd> ();
      case 2 : return makeBaseSet<quadrilateral,polOrd> ();
      case 3 : return makeBaseSet<hexahedron,polOrd> ();
      default :
        DUNE_THROW(NotImplemented, "No Lagrange function spaces for cubes of dimension "
                   << EntityType::dimension << "!");
      }

    default : {
      DUNE_THROW(NotImplemented, "Element type " << en.geometry().type() << " is not provided yet!");
    }
    }
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template <GeometryType ElType, int pO >
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  BaseFunctionSetType *
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  makeBaseSet ()
  {
    typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType,
        ElType, pO > BaseFuncSetType;

    BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );

    mapper_ = new LagrangeMapperType (grid_.indexSet(),
                                      baseFuncSet->getNumberOfBaseFunctions(),
                                      grid_.grid().maxlevel() );

    return baseFuncSet;
  }

} // end namespace Dune

#endif
