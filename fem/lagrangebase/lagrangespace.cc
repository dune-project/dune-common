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
    typedef typename GridType :: template Codim<0> :: Entity EntityType;
    IteratorType endit  = gridPart.template end<0>();
    for(IteratorType it = gridPart.template begin<0>(); it != endit; ++it) {
      GeometryType geo = (*it).geometry().type(); // Hack
      int dimension = static_cast<int>( EntityType::mydimension);
      GeometryIdentifier::IdentifierType id =
        GeometryIdentifier::fromGeo(dimension, geo);
      if(baseFuncSet_[id] == 0 ) {
        baseFuncSet_[id] = setBaseFuncSetPointer(*it);
        mapper_ = new typename
                  Traits::MapperType(const_cast<IndexSetType&>(gridPart.indexSet()),
                                     baseFuncSet_[id]->numBaseFunctions());
      }
    }

    // for empty functions space which can happen for BSGrid
    //if(!mapper_) makeBaseSet<line,0> (gridPart.indexSet());
    //assert(mapper_);
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

  /*
     template <class FunctionSpaceImp, class GridPartImp, int polOrd>
     template< class DiscFuncType >
     inline typename DiscFuncType :: MemObjectType &
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     signIn (DiscFuncType & df) const
     {
     // only for gcc to pass type DofType
     assert(mapper_ != 0);

     return dm_.addDofSet( df.getStorageType() , *mapper_, df.name() );
     }

     template <class FunctionSpaceImp, class GridPartImp, int polOrd>
     template <class DiscFuncType>
     inline bool
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     signOut (DiscFuncType & df) const
     {
     return dm_.removeDofSet( df.memObj() );
     }
   */

  template <class FunctionSpaceImp, class GridPartImp, int polOrd>
  const typename LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::MapperType&
  LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::mapper() const {
    assert(mapper_);
    return *mapper_;
  }

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
  /*
     template <class FunctionSpaceImp, class GridPartImp, int polOrd>
     template <class EntityType>
     inline typename
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     BaseFunctionSetType*
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     setBaseFuncSetPointer ( EntityType &en,const IndexSetType& iset )
     {
     switch (en.geometry().type())
        {
        case line         : return makeBaseSet<line,polOrd> (iset);
        case triangle     : return makeBaseSet<triangle,polOrd> (iset);
        case quadrilateral: return makeBaseSet<quadrilateral,polOrd> (iset);
        case tetrahedron  : return makeBaseSet<tetrahedron,polOrd> (iset);
        case pyramid      : return makeBaseSet<pyramid,polOrd> (iset);
        case prism        : return makeBaseSet<prism,polOrd> (iset);
        case hexahedron   : return makeBaseSet<hexahedron,polOrd> (iset);

        case simplex :
          switch (EntityType::dimension) {
          case 1: return makeBaseSet<line,polOrd> (iset);
          case 2: return makeBaseSet<triangle,polOrd> (iset);
          case 3: return makeBaseSet<tetrahedron,polOrd> (iset);
          default:
            DUNE_THROW(NotImplemented, "No Lagrange function spaces for simplices of dimension "
                       << EntityType::dimension << "!");
          }

        case cube :
          switch (EntityType::dimension) {
          case 1: return makeBaseSet<line,polOrd> (iset);
          case 2: return makeBaseSet<quadrilateral,polOrd> (iset);
          case 3: return makeBaseSet<hexahedron,polOrd> (iset);
          default:
            DUNE_THROW(NotImplemented,
                       "No Lagrange function spaces for cubes of dimension "
                       << EntityType::dimension << "!");
          }

        default: {
            DUNE_THROW(NotImplemented, "Element type "
                       << en.geometry().type() << " is not provided yet!");
        }
     }
     }

     template <class FunctionSpaceImp, class GridPartImp, int polOrd>
     template <GeometryType ElType, int pO >
     inline typename
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     BaseFunctionSetType *
     LagrangeDiscreteFunctionSpace<FunctionSpaceImp, GridPartImp, polOrd>::
     makeBaseSet (const IndexSetType& iset)
     {
     typedef LagrangeFastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType,
          ElType, pO > BaseFuncSetType;

     BaseFuncSetType * baseFuncSet = new BaseFuncSetType ( *this );

     mapper_ = new MapperType (const_cast<IndexSetType&>(iset),
                              baseFuncSet->numBaseFunctions());

     return baseFuncSet;
     }
   */
} // end namespace Dune

#endif
