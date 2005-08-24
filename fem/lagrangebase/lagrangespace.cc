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
    baseFuncSet_(0),
    dm_(dm),
    grid_(g)
  {
    makeFunctionSpace(g);
  }


  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline void LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  makeFunctionSpace (GridPartType& gridPart)
  {
    // add index set to list of indexset of dofmanager

    dm_.addIndexSet( gridPart.grid() , gridPart.indexSet() );

    mapper_ = 0;

    //std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";

    // search the macro grid for diffrent element types
    typedef typename GridType::template Codim<0>::LevelIterator LevelIteratorType;
    IteratorType endit  = gridPart.template end<0>();
    for(IteratorType it = gridPart.template begin<0>(); it != endit; ++it) {
      GeometryType geo = (*it).geometry().type(); // Hack
      int dimension = static_cast<int>(LevelIteratorType::Entity::mydimension);
      GeometryIdentifier::IdentifierType id =
        GeometryIdentifier::fromGeo(dimension, geo);
      if(baseFuncSet_[id] == 0 )
        baseFuncSet_[id] = setBaseFuncSetPointer(*it, gridPart.indexSet());
    }

    // for empty functions space which can happen for BSGrid
    if(!mapper_) makeBaseSet<line,0> (gridPart.indexSet());
    assert(mapper_);
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  ~LagrangeDiscreteFunctionSpace ()
  {
    for(unsigned int i=0; i<baseFuncSet_.dim(); i++)
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
    GeometryType geo =  en.geometry().type();
    int dimension = static_cast<int>(EntityType::mydimension);
    return *baseFuncSet_[GeometryIdentifier::fromGeo(dimension, geo)];
  }

  template <
      class FunctionSpaceImp, class GridPartImp, int polOrd, class DofManagerImp
      >
  template< class EntityType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp>::
  evaluateLocal ( int baseFunc, EntityType &en, const DomainType &local, RangeType & ret) const
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
    return dm_.addDofSet( df.getStorageType() , grid_.grid() , *mapper_, df.name() );
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
  setBaseFuncSetPointer ( EntityType &en, IndexSetType& iset )
  {
    switch (en.geometry().type())
    {
    case line         : return makeBaseSet<line,polOrd> (iset);
    case triangle     : return makeBaseSet<triangle,polOrd> (iset);
    case quadrilateral : return makeBaseSet<quadrilateral,polOrd> (iset);
    case tetrahedron  : return makeBaseSet<tetrahedron,polOrd> (iset);
    case pyramid      : return makeBaseSet<pyramid,polOrd> (iset);
    case prism        : return makeBaseSet<prism,polOrd> (iset);
    case hexahedron   : return makeBaseSet<hexahedron,polOrd> (iset);

    case simplex :
      switch (EntityType::dimension) {
      case 1 : return makeBaseSet<line,polOrd> (iset);
      case 2 : return makeBaseSet<triangle,polOrd> (iset);
      case 3 : return makeBaseSet<tetrahedron,polOrd> (iset);
      default :
        DUNE_THROW(NotImplemented, "No Lagrange function spaces for simplices of dimension "
                   << EntityType::dimension << "!");
      }

    case cube :
      switch (EntityType::dimension) {
      case 1 : return makeBaseSet<line,polOrd> (iset);
      case 2 : return makeBaseSet<quadrilateral,polOrd> (iset);
      case 3 : return makeBaseSet<hexahedron,polOrd> (iset);
      default :
        DUNE_THROW(NotImplemented,
                   "No Lagrange function spaces for cubes of dimension "
                   << EntityType::dimension << "!");
      }

    default : {
      DUNE_THROW(NotImplemented, "Element type "
                 << en.geometry().type() << " is not provided yet!");
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
  makeBaseSet (IndexSetType& iset)
  {
    typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType,
        ElType, pO > BaseFuncSetType;

    BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );

    mapper_ = new LagrangeMapperType (iset,
                                      baseFuncSet->getNumberOfBaseFunctions(),
                                      0);

    return baseFuncSet;
  }

} // end namespace Dune

#endif
