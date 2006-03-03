// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGESPACE_CC
#define DUNE_LAGRANGESPACE_CC

#include <algorithm>

namespace Dune {

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  LagrangeDiscreteFunctionSpace (GridPartType & g) :
    DefaultType(id),
    baseFuncSet_(GeometryIdentifier::numTypes,0),
    grid_(g),
    mapper_(0)
  {
    makeFunctionSpace(g);
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  inline void LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  makeFunctionSpace (GridPartType& gridPart)
  {
    // add index set to list of indexset of dofmanager
    typedef DofManager<GridType> DofManagerType;
    typedef DofManagerFactory<DofManagerType> DofManagerFactoryType;
    DofManagerType & dm = DofManagerFactoryType::getDofManager(gridPart.grid());
    dm.addIndexSet(gridPart.grid(), const_cast<typename GridPartType::IndexSetType&>(gridPart.indexSet()));

    //std::cout << "Constructor of LagrangeDiscreteFunctionSpace! \n";
    // search the macro grid for diffrent element types
    const std::vector<GeometryType>& geomTypes =
      gridPart.indexSet().geomTypes(0) ;

    for(size_t i=0; i<geomTypes.size(); ++i)
    {
      GeometryIdentifier::IdentifierType id =
        GeometryIdentifier::fromGeo(geomTypes[i]);
      if(baseFuncSet_[id] == 0 ) {
        baseFuncSet_[id] = setBaseFuncSetPointer(geomTypes[i]);
        mapper_ = new typename
                  Traits::MapperType(const_cast<IndexSetType&>(gridPart.indexSet()),
                                     baseFuncSet_[id]->numBaseFunctions());
      }
    }
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  inline LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  ~LagrangeDiscreteFunctionSpace ()
  {
    for(unsigned int i=0; i<baseFuncSet_.size(); i++)
      if (baseFuncSet_[i] != 0)
        delete baseFuncSet_[i];

    delete(mapper_);
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  inline DFSpaceIdentifier LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  type () const
  {
    return LagrangeSpace_id;
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  template <class EntityType>
  inline const
  typename LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::BaseFunctionSetType &
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  getBaseFunctionSet (EntityType &en) const
  {

    GeometryType geo =  en.geometry().type();
    int dimension = static_cast<int>(EntityType::mydimension);
    assert(GeometryIdentifier::fromGeo(dimension,geo)<(int) baseFuncSet_.size());
    assert(GeometryIdentifier::fromGeo(dimension, geo) >= 0);

    assert(baseFuncSet_[GeometryIdentifier::fromGeo(dimension, geo)]);
    return *baseFuncSet_[GeometryIdentifier::fromGeo(dimension, geo)];


  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  template< class EntityType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  evaluateLocal ( int baseFunc, EntityType &en, const DomainType &local, RangeType & ret) const
  {
    const BaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , local , ret);
    return (polOrd != 0);
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  template< class EntityType, class QuadratureType>
  inline bool LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  evaluateLocal (  int baseFunc, EntityType &en, QuadratureType &quad,
                   int quadPoint, RangeType & ret) const
  {
    const BaseFunctionSetType & baseSet = getBaseFunctionSet(en);
    baseSet.eval( baseFunc , quad, quadPoint , ret);
    return (polOrd != 0);
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  size () const
  {
    return mapper_->size ();
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  template< class EntityType>
  inline int LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  mapToGlobal ( EntityType &en, int localNum ) const
  {
    return mapper_->mapToGlobal ( en , localNum );
  }

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  const typename LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::MapperType&
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::mapper() const {
    assert(mapper_);
    return *mapper_;
  }

  /*
     template <class FunctionSpaceImp, class GridPartImp, int polOrd>
     template <class EntityType>
     typename LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     BaseFunctionSetType*
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     setBaseFuncSetPointer(EntityType& en)
     {
     typedef typename ToScalarFunctionSpace<
      typename Traits::FunctionSpaceType>::Type ScalarFunctionSpaceType;

     LagrangeBaseFunctionFactory<
      ScalarFunctionSpaceType, polOrd> fac(en.geometry().type());
     return new BaseFunctionSetType(fac);
     }
   */

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  typename LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  BaseFunctionSetType*
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
  setBaseFuncSetPointer(GeometryType type)
  {
    typedef typename ToScalarFunctionSpace<
        typename Traits::FunctionSpaceType>::Type ScalarFunctionSpaceType;

    LagrangeBaseFunctionFactory<
        ScalarFunctionSpaceType, polOrd> fac(type);
    return new BaseFunctionSetType(fac);
  }

} // end namespace Dune
#endif
