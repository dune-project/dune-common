// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGESPACE_CC
#define DUNE_LAGRANGESPACE_CC

#include <algorithm>

namespace Dune {

  // Constructor
  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  LagrangeDiscreteFunctionSpace (const GridType & g, IndexSetType & iset, DofManagerType & dm , int level) :
    DiscreteFunctionSpaceType (g,id, level) , dm_ ( dm ) , indexSet_ (iset)
  {
    makeFunctionSpace(level);
  }

  // Constructor
  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  LagrangeDiscreteFunctionSpace (const GridType & g, IndexSetType & iset, int level) :
    DiscreteFunctionSpaceType (g,id, level) , dm_ ( DofManagerFactoryType::getDofManager(g) ) , indexSet_ (iset)
  {
    makeFunctionSpace(level);
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline void LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  makeFunctionSpace (int level)
  {
    // add index set to list of indexset of dofmanager
    dm_.addIndexSet( this->grid_ , indexSet_ );

    mapper_ = 0;
    maxNumBase_ = 0;

    //std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";
    for(int i=0; i<numOfDiffBase_; i++)
      baseFuncSet_[i] = 0;

    {
      // search the macro grid for diffrent element types
      typedef typename GridType::template Codim<0>::LevelIterator LevelIteratorType;
      LevelIteratorType endit  = this->grid_.template lend<0>(level);
      for(LevelIteratorType it = this->grid_.template lbegin<0>(level); it != endit; ++it)
      {
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

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  ~LagrangeDiscreteFunctionSpace ()
  {
    for(int i=0; i<numOfDiffBase_; i++)
      if (baseFuncSet_[i] != 0)
        delete baseFuncSet_[i];

    delete(mapper_);
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline DFSpaceIdentifier LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  type () const
  {
    return LagrangeSpace_id;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template <class EntityType>
  inline const
  typename LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  FastBaseFunctionSetType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  getBaseFunctionSet (EntityType &en) const
  {
    GeometryType type =  en.geometry().type();
    return *baseFuncSet_[type];
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class EntityType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  evaluateLocal ( int baseFunc, EntityType &en, Domain &local, Range & ret) const
  {
    const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , local , ret);
    return (polOrd != 0);
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class EntityType, class QuadratureType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  evaluateLocal (  int baseFunc, EntityType &en, QuadratureType &quad,
                   int quadPoint, Range & ret) const
  {
    const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , quad, quadPoint , ret);
    return (polOrd != 0);
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  continuous ( ) const
  {
    bool ret = (polOrd == 0) ? false : true;
    return ret;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  maxNumberBase () const
  {
    return maxNumBase_;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  polynomOrder ( ) const
  {
    return polOrd;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class EntityType>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  localPolynomOrder ( EntityType &en ) const
  {
    return polOrd;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  dimensionOfValue () const
  {
    return dimVal;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  size () const
  {
    return mapper_->size ();
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  inline BoundaryType LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  boundaryType (int id) const
  {
    // here we need a boundary id map class
    return (id > 0) ? Dirichlet : Neumann;
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class EntityType>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  mapToGlobal ( EntityType &en, int localNum ) const
  {
    return mapper_->mapToGlobal ( en , localNum );
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class DiscFuncType >
  inline typename DiscFuncType :: MemObjectType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  signIn (DiscFuncType & df) const
  {
    // only for gcc to pass type DofType
    assert(mapper_ != 0);
    return dm_.addDofSet( df.getStorageType() , this->grid_ , indexSet_ , *mapper_ , df.name() );
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class DiscFuncType>
  inline bool
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  signOut (const DiscFuncType & df) const
  {
    return dm_.removeDofSet( df.memObj() );
  }

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template< class EntityType>
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  FastBaseFunctionSetType *
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
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

  template< class FunctionSpaceT, class GridType, class IndexSetType , int polOrd, class DofManagerType >
  template <GeometryType ElType, int pO >
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  FastBaseFunctionSetType *
  LagrangeDiscreteFunctionSpace<FunctionSpaceT,GridType,IndexSetType,polOrd,DofManagerType>::
  makeBaseSet ()
  {
    typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType,
        ElType, pO > BaseFuncSetType;

    BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );

    mapper_ = new LagrangeMapperType (indexSet_, baseFuncSet->getNumberOfBaseFunctions() , this->level_ );

    return baseFuncSet;
  }

} // end namespace Dune

#endif
