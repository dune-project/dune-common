// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGESPACE_CC
#define DUNE_LAGRANGESPACE_CC

#include <algorithm>

namespace Dune {

  // Constructor
  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  LagrangeDiscreteFunctionSpace ( GridType & g, DofManagerType & dm , int level) :
    DiscreteFunctionSpaceType (g,id, level) , dm_ ( dm )
  {
    mapper_ = 0;
    maxNumBase_ = 0;

    //std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";
    for(int i=0; i<numOfDiffBase_; i++)
      baseFuncSet_[i] = 0;

    {
      // search the macro grid for diffrent element types
      typedef typename GridType::template codim<0>::LevelIterator LevelIteratorType;
      LevelIteratorType endit  = g.template lend<0>(level);
      for(LevelIteratorType it = g.template lbegin<0>(level); it != endit; ++it)
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

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  ~LagrangeDiscreteFunctionSpace ()
  {
    for(int i=0; i<numOfDiffBase_; i++)
      if (baseFuncSet_[i] != 0)
        delete baseFuncSet_[i];
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline DFSpaceIdentifier LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  type () const
  {
    return LagrangeSpace_id;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template <class EntityType>
  inline const
  typename LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  FastBaseFunctionSetType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  getBaseFunctionSet (EntityType &en) const
  {
    GeometryType type =  en.geometry().type();
    return *baseFuncSet_[type];
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class EntityType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  evaluateLocal ( int baseFunc, EntityType &en, Domain &local, Range & ret) const
  {
    const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , local , ret);
    return (polOrd != 0);
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class EntityType, class QuadratureType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  evaluateLocal (  int baseFunc, EntityType &en, QuadratureType &quad,
                   int quadPoint, Range & ret) const
  {
    const FastBaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , quad, quadPoint , ret);
    return (polOrd != 0);
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  continuous ( ) const
  {
    bool ret = (polOrd == 0) ? false : true;
    return ret;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  maxNumberBase () const
  {
    return maxNumBase_;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  polynomOrder ( ) const
  {
    return polOrd;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class EntityType>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  localPolynomOrder ( EntityType &en ) const
  {
    return polOrd;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  dimensionOfValue () const
  {
    return dimVal;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  size () const
  {
    return mapper_->size ();
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  inline BoundaryType LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  boundaryType (int id) const
  {
    // here we need a boundary id map class
    return (id > 0) ? Dirichlet : Neumann;
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class EntityType>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  mapToGlobal ( EntityType &en, int localNum ) const
  {
    return mapper_->mapToGlobal ( en , localNum );
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class DiscFuncType>
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  MemObjectType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  signIn (DiscFuncType & df) const
  {
    // only for gcc to pass type DofType
    assert(mapper_ != 0);
    DofType *fake=0;
    return dm_.addDofSet( fake, this->grid_ , *mapper_, df.name() );
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class DiscFuncType>
  inline bool
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  signOut (DiscFuncType & df) const
  {
    return dm_.removeDofSet( df.memObj() );
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template< class EntityType>
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  FastBaseFunctionSetType *
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
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
    default : {
      std::cerr << en.geometry().type() << " This element type is not provided yet! \n";
      abort();
      return 0;
    }
    }
  }

  template< class FunctionSpaceType, class GridType,int polOrd, class DofManagerType >
  template <GeometryType ElType, int pO >
  inline typename
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  FastBaseFunctionSetType *
  LagrangeDiscreteFunctionSpace<FunctionSpaceType,GridType,polOrd,DofManagerType>::
  makeBaseSet ()
  {
    typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType,
        ElType, pO > BaseFuncSetType;

    BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );

    mapper_ = new LagrangeMapperType (dm_.indexSet(), baseFuncSet->getNumberOfBaseFunctions() , this->level_ );

    return baseFuncSet;
  }

} // end namespace Dune

#endif
