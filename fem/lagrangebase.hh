// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAGRANGEBASE_HH
#define DUNE_LAGRANGEBASE_HH

#include <dune/grid/common/grid.hh>

#include "common/discretefunctionspace.hh"
#include "lagrangebase/lagrangebasefunctions.hh"
#include "lagrangebase/lagrangemapper.hh"
#include "dofmanager.hh"
//#include "dgspace/dgmapper.hh"

namespace Dune {

  // Forfard declarations
  template <
      class FunctionSpaceImp,
      class GridPartImp,
      int polOrd,
      class DofManagerImp
      >
  class LagrangeDiscreteFunctionSpace;

  template <
      class FunctionSpaceImp,
      class GridPartImp,
      int polOrd,
      class DofManagerImp
      >
  struct LagrangeDiscreteFunctionSpaceTraits {
    typedef FunctionSpaceImp FunctionSpaceType;
    typedef GridPartImp GridPartType;
    typedef DofManagerImp DofManagerType;
    typedef LagrangeDiscreteFunctionSpace<
        FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp
        > DiscreteFunctionSpaceType;
    typedef FastBaseFunctionSet<DiscreteFunctionSpaceType> BaseFunctionSetType;

    typedef typename GridPartType::GridType GridType;
    typedef typename GridPartType::IndexSetType IndexSetType;
    typedef typename GridPartType::template Codim<0>::IteratorType IteratorType;
  };

  //****************************************************************
  //
  //  --LagrangeDiscreteFunctionSpace
  //
  //! Provides access to base function set for different element
  //! type in one grid and size of functionspace
  //! and map from local to global dof number
  //
  //****************************************************************
  template<class FunctionSpaceImp, class GridPartImp, int polOrd,
      class DofManagerImp = DofManager<typename GridPartImp::GridType> >
  class LagrangeDiscreteFunctionSpace :
    public DiscreteFunctionSpaceDefault
    <
        LagrangeDiscreteFunctionSpaceTraits<FunctionSpaceImp, GridPartImp,
            polOrd, DofManagerImp>
    >
  {
    typedef DofManagerImp DofManagerType;

    typedef DofManagerFactory<DofManagerType> DofManagerFactoryType;

  public:
    typedef typename GridPartImp::GridType GridType;

    /** \todo Please doc me! */
    typedef LagrangeDiscreteFunctionSpace<
        FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp
        > LagrangeDiscreteFunctionSpaceType;

    typedef LagrangeDiscreteFunctionSpaceTraits<
        FunctionSpaceImp, GridPartImp, polOrd, DofManagerImp
        > Traits;

    /** \todo Please doc me! */
    typedef DiscreteFunctionSpaceDefault<Traits> DefaultType;

    /** \todo Please doc me! */
    typedef typename Traits::BaseFunctionSetType BaseFunctionSetType;
    /** \todo Please doc me! */
    typedef typename Traits::IndexSetType IndexSetType;

    typedef typename Traits::GridPartType GridPartType;

    typedef typename Traits::IteratorType IteratorType;

    typedef typename Traits::FunctionSpaceType FunctionSpaceType;

    //! id is neighbor of the beast
    static const IdentifierType id = 665;

    // Lagrange 1 , to be revised in this matter
    /** \todo Please doc me! */
    enum { DimRange = FunctionSpaceType::DimRange };

    /** \todo Please doc me! */
    typedef LagrangeMapper<IndexSetType,polOrd,DimRange> LagrangeMapperType;

    /** \todo Please doc me! */
    //typedef typename DofManagerType::MemObjectType MemObjectType;
    template <class DofStorageType>
    struct DofTraits
    {
      typedef typename DofManagerType:: template Traits< LagrangeMapperType ,
          DofStorageType > ::  MemObjectType MemObjectType;
    };

    /** \todo Please doc me! */
    typedef typename FunctionSpaceType::DomainType DomainType;
    /** \todo Please doc me! */
    typedef typename FunctionSpaceType::RangeType RangeType;
    /** \todo Please doc me! */
    typedef typename FunctionSpaceType::RangeFieldType DofType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainFieldType DomainFieldType;

    //! dimension of value
    enum { dimVal = 1 };

    //! remember polynomial order
    enum { polynomialOrder =  polOrd };

  public:

    //! Constructor generating for each different element type of the grid a
    //! LagrangeBaseSet with polOrd
    LagrangeDiscreteFunctionSpace(GridPartType & g, DofManagerType & dm);

    //! Desctructor
    virtual ~LagrangeDiscreteFunctionSpace ();

    //! continuous
    bool continuous() const { return true; }

    //! return max number of baseset that holds this space
    //int maxNumberBase () const;

    //! return type of this fucntion space
    DFSpaceIdentifier type () const;

    //! returns polynomial order
    int polynomOrder() const { return polynomialOrder; }

    //! begin iterator
    IteratorType begin() const { return grid_.template begin<0>(); }

    //! end iterator
    IteratorType end() const { return grid_.template end<0>(); }

    //! provide the access to the base function set for a given entity
    template <class EntityType>
    const BaseFunctionSetType& getBaseFunctionSet ( EntityType &en ) const;

    //! default for polOrd 0
    template <class EntityType>
    bool evaluateLocal (int baseFunc, EntityType &en, const DomainType &local, RangeType & ret) const;

    //! default for polOrd 0
    template <class EntityType, class QuadratureType>
    bool evaluateLocal ( int baseFunc, EntityType &en, QuadratureType &quad,
                         int quadPoint, RangeType & ret) const;


    //! get dimension of value
    int dimensionOfValue () const;


    //! number of unknows for this function space
    int size () const;

    //! for given entity map local dof number to global dof number
    template <class EntityType>
    int mapToGlobal ( EntityType &en, int localNum ) const;

    //! sign in to dofmanager, return is the memory
    template <class DiscFuncType>
    typename DiscFuncType :: MemObjectType &
    signIn (DiscFuncType & df) const;

    //! sign out to dofmanager, dofmanager frees the memory
    template <class DiscFuncType>
    bool signOut ( DiscFuncType & df) const;

  protected:
    // create functions space with basefunction set for given level
    void makeFunctionSpace (GridPartType& gridPart);

    //! get the right BaseFunctionSet for a given Entity
    template <class EntityType>
    BaseFunctionSetType* setBaseFuncSetPointer(EntityType &en,
                                               IndexSetType& iset);

    //! make base function set depending on GeometryType and polynomial order
    template <GeometryType ElType, int pO >
    BaseFunctionSetType* makeBaseSet (IndexSetType& iset);

  protected:
    //! the corresponding vector of base function sets
    //! length is different element types
    std::vector < BaseFunctionSetType * > baseFuncSet_;

    //! DofManager manages the memory
    mutable DofManagerType & dm_;

    //! the index set, used by the mapper for mapping between grid and space
    mutable GridPartType& grid_;
  private:
    //! the corresponding LagrangeMapper
    LagrangeMapperType *mapper_;

  }; // end class LagrangeDiscreteFunctionSpace


  //*******************************************************************
  //
  //! DGSpace using Lagrange basis functions, used for visualisation
  //
  //*******************************************************************
  // * Would need to be adapted to new interface
  // template< class FunctionSpaceType, class GridType,class IndexSetType, int polOrd, class
  // DofManagerType = DofManager<GridType> >
  // class LagrangeDGSpace
  // : public LagrangeDiscreteFunctionSpace<  FunctionSpaceType , GridType, IndexSetType ,
  //     polOrd , DofManagerType >
  // {
  // public:
  //   typedef LagrangeDiscreteFunctionSpace
  //       < FunctionSpaceType , GridType , IndexSetType , polOrd  , DofManagerType > ThisType;
  //   typedef ThisType LagrangeDiscreteFunctionSpaceType;

  //   typedef DiscreteFunctionSpaceInterface <
  //       FunctionSpaceType , GridType, LagrangeDiscreteFunctionSpaceType,
  //     FastBaseFunctionSet < LagrangeDiscreteFunctionSpaceType > >  DiscreteFunctionSpaceType;

  //   typedef FastBaseFunctionSet <LagrangeDiscreteFunctionSpaceType> BaseFunctionSetType;
  //   typedef BaseFunctionSetType FastBaseFunctionSetType;

  //   //! id is neighbor of the beast
  //   static const IdentifierType id = 668;

  //   // Lagrange 1 , to be revised in this matter
  //   enum { numOfDiffBase_ = 20 };
  //   enum { DimRange = FunctionSpaceType::DimRange };

  // public:
  //   typedef DGMapper<IndexSetType,polOrd,DimRange> DGMapperType;

  //   template <class DofStorageType>
  //   struct DofTraits
  //   {
  //     typedef typename DofManagerType:: template Traits< DGMapperType ,
  //             DofStorageType > ::  MemObjectType MemObjectType;
  //   };

  //   typedef typename FunctionSpaceType::Domain     Domain;
  //   typedef typename FunctionSpaceType::Range      Range;
  //   typedef typename FunctionSpaceType::RangeField DofType;
  //   typedef typename FunctionSpaceType::DomainField     DomainField;

  //   //! dimension of value
  //   enum { dimVal = 1 };

  //   //! remember polynomial order
  //   enum { polynomialOrder =  polOrd };

  //   //! Constructor generating for each different element type of the grid a
  //   //! LagrangeBaseSet with polOrd
  //   LagrangeDGSpace ( GridType & g, IndexSetType & iset, DofManagerType & dm , int level ) :
  //    LagrangeDiscreteFunctionSpaceType (g, iset , dm ,level) , mapper_(0)
  //   {
  //     typedef typename GridType::template codim<0> :: LevelIterator LevIt;
  //     LevIt it = g.template lbegin<0>(0);
  //     if(it != g.template lend<0>(0))
  //     {
  //       int basenum = this->getBaseFunctionSet(*it).getNumberOfBaseFunctions();
  //       mapper_ = new DGMapperType ( this->indexSet_ , basenum , level );
  //     }
  //   };

  //   //! Destructor
  //   ~LagrangeDGSpace () {
  //     if (mapper_) delete mapper_;
  //   }

  //   //! function where you dont need to know waht is done
  //   template <class DiscFuncType>
  //   typename DiscFuncType :: MemObjectType & signIn (DiscFuncType & df)
  //   {
  //     // only for gcc to pass type DofType
  //     assert(mapper_ != 0);
  //     return this->dm_.addDofSet( df.getStorage() , this->grid_ , *mapper_, df.name() );
  //     // do notin' at the moment
  //   }

  //   //! return max number of baseset that holds this space
  //   int maxNumberBase () const;

  //   //! return type of this fucntion space
  //   DFSpaceIdentifier type () const { return DGSpace_id; }

  //   //! return true if we have continuous discrete functions
  //   bool continuous () const { return false; }

  //   //! number of unknows for this function space
  //   int size () const { return mapper_->size() ; }

  //   //! for given entity map local dof number to global dof number
  //   template <class EntityType>
  //   int mapToGlobal ( EntityType &en, int localNum ) const
  //   {
  //     return mapper_->mapToGlobal(en,localNum);
  //   }

  // private:
  //   //! the corresponding LagrangeMapper
  //   DGMapperType *mapper_;

  // }; // end class LagrangeDiscreteFunctionSpace

} // end namespace Dune

// contains the implementation of LagrangeSpace
#include "lagrangebase/lagrangespace.cc"

#endif
