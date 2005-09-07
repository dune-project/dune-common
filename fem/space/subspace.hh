// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SUBSPACE_HH
#define DUNE_SUBSPACE_HH

//- System includes

//- Dune includes
#include <dune/fem/common/discretefunctionspace.hh>
#include <dune/fem/common/basefunctions.hh>
#include <dune/fem/common/dofmapperinterface.hh>

namespace Dune {

  //- Forward declarations
  template <class CombinedSpaceImp>
  class SubSpace;
  template <class CombinedSpaceImp>
  class SubBaseFunctionSet;
  template <class CombinedSpaceImp>
  class SubMapper;

  template <class CombinedSpaceImp>
  struct SubSpaceTraits {
  private:
    typedef CombinedSpaceImp CombinedSpaceType;
    typedef typename CombinedSpaceType::Traits CombinedTraits;

    typedef typename CombinedTraits::RangeFieldType DofType;
    typedef typename CombinedTraits::RangeType CombinedRangeType;
    typedef typename CombinedTraits::JacobianRangeType CombinedJacobianRangeType;

    typedef typename CombinedTraits::BaseFunctionSetType CombinedBaseFunctionSetType;
    typedef typename CombinedTraits::MapperType CombinedMapperType;

    typedef typename CombinedTraits::ContainedMapperType ContainedMapperType;

    enum { CombinedDimRange = CombinedTraits::DimRange };

  public:
    // Assumption: only scalar contained function spaces
    enum { DimDomain = CombinedTraits::DimDomain,
           DimRange = 1 };

    typedef FunctionSpace<
        DofType, DofType, DimDomain, DimRange> FunctionSpaceType;
    typedef SubSpace<CombinedSpaceImp> DiscreteFunctionSpaceType;
    typedef SubBaseFunctionSet<CombinedSpaceImp> BaseFunctionSetType;
    typedef SubMapper<CombinedSpaceImp> MapperType;

    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainFieldType DomainFieldType;
    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::JacobianRangeType JacobianRangeType;

    typedef typename CombinedTraits::GridType GridType;
    typedef typename CombinedTraits::IteratorType IteratorType;
    typedef typename CombinedTraits::DofConversionType DofConversionType;

  public:
    //- Friends
    friend class DiscreteFunctionType;
    friend class BaseFunctionSetType;
    friend class MapperType;
  };

  template <class CombinedSpaceImp>
  class SubSpace :
    public DiscreteFunctionSpaceDefault<SubSpaceTraits<CombinedSpaceImp> >
  {
  public:
    //- Typedefs and enums
    typedef CombinedSpaceImp CombinedSpaceType;

    typedef SubSpace<CombinedSpaceType> ThisType;
    typedef SubSpaceTraits<CombinedSpaceType> Traits;
    typedef DiscreteFunctionSpaceDefault<Traits> BaseType;

    typedef typename Traits::GridType GridType;
    typedef typename Traits::IteratorType IteratorType;
    typedef typename Traits::MapperType MapperType;
    typedef typename Traits::BaseFunctionSetType BaseFunctionSetType;

    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::DomainType DomainType;

  public:
    //- Public methods
    //! constructor
    SubSpace(const CombinedSpaceType& spc, int component);

    //! the type of the space
    int type() const { return spaceId_; }

    //! is data continuous?
    bool continuous() const { return spc_.continuous(); }

    //! polynom order
    int polynomOrder() const { return spc_.polynomOrder(); }

    //! begin iterator
    IteratorType begin() const { return spc_.begin(); }

    //! end iterator
    IteratorType end() const { return spc_.end(); }

    //! total number of dofs
    int size() const { return mapper_.size(); }

    //! map a local dof number to a global one
    template <class EntityType>
    int mapToGlobal(EntityType& en, int local) const {
      return mapper_.mapToGlobal(en, local);
    }

    // access to baseFunctionSet
    template <class EntityType>
    const BaseFunctionSetType& getBaseFunctionSet(EntityType& en) const
    {
      GeometryType geo = en.geometry().type();
      int dimension = static_cast<int>(EntityType::mydimension);

      assert(baseSetVec_[GeometryIdentifier::fromGeo(dimension, geo)]);
      return *baseSetVec_[GeometryIdentifier::fromGeo(dimension, geo)];
    }

    //! access to grid (const version)
    const GridType& grid() const { return spc_.grid(); }

    //! access to mapper
    const MapperType& mapper() const { return mapper_; }

  private:
    //- Forbidden methods
    SubSpace(const ThisType& other);
    ThisType& operator=(const ThisType& other);

  private:
    //- Data members
    const CombinedSpaceType& spc_;
    MapperType mapper_;
    int component_;

    std::vector<BaseFunctionSetType*> baseSetVec_;
    static const int spaceId_;
  };

  // Idea: wrap contained base function set, since this is exactly what you
  // need here (except for when you go for ranges of subfunctions...)
  template <class CombinedSpaceImp>
  class SubBaseFunctionSet :
    public BaseFunctionSetDefault<SubSpaceTraits<CombinedSpaceImp> >
  {
  public:
    //- Typedefs and enums
    typedef CombinedSpaceImp CombinedSpaceType;

    typedef SubBaseFunctionSet<CombinedSpaceType> ThisType;
    typedef SubSpaceTraits<CombinedSpaceType> Traits;

    typedef typename Traits::DiscreteFunctionSpaceType
    DiscreteFunctionSpaceType;
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::DomainType DomainType;
    typedef typename Traits::JacobianRangeType JacobianRangeType;
    typedef typename Traits::CombinedBaseFunctionSetType CombinedBaseFunctionSetType;
    typedef typename Traits::CombinedRangeType CombinedRangeType;

    typedef int deriType;
  public:
    //- Public methods
    SubBaseFunctionSet(const CombinedBaseFunctionSetType& bSet,
                       int component) :
      bSet_(bSet),
      component_(component),
      tmp_(0.0)
    {}

    int getNumberOfBaseFunctions() const DUNE_DEPRECATED {
      assert(bSet_.getNumberOfBaseFunctions()%CombinedDimRange == 0);
      return bSet_.getNumberOfBaseFunctions()/CombinedDimRange;
    }

    //! Number of base functions
    int numBaseFunctions() const {
      assert(bSet_.numBaseFunctions()%CombinedDimRange == 0);
      return bSet_.numBaseFunctions()/CombinedDimRange;
    }

    //! evaluate base function
    template <int diffOrd>
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   const DomainType & x, RangeType & phi ) const;

    //! evaluate base function at quadrature point
    template <int diffOrd, class QuadratureType >
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   QuadratureType & quad,
                   int quadPoint, RangeType & phi ) const;

  private:
    //- Data members
    const CombinedBaseFunctionSetType& bSet_;
    const int component_;

    mutable CombinedRangeType tmp_;
  };

  template <class CombinedSpaceImp>
  class SubMapper :
    public DofMapperDefault<SubMapper<CombinedSpaceImp> >
  {
  public:
    //- Typedefs and enums
    typedef CombinedSpaceImp CombinedSpaceType;

    typedef SubMapper<CombinedSpaceType> ThisType;
    typedef SubSpaceTraits<CombinedSpaceType> Traits;

    typedef typename Traits::CombinedMapperType CombinedMapperType;
    typedef typename Traits::ContainedMapperType ContainedMapperType;
    typedef typename Traits::DofConversionType DofConversionType;

  public:
    //- Public methods
    SubMapper(const CombinedSpaceType& spc,
              const ContainedMapperType& mapper,
              int component) :
      spc_(spc),
      mapper_(mapper),
      component_(component),
      utilGlobal_(spc.policy() == PointBased ?
                  spc.numComponents() :
                  spc.size()/spc.numComponents())
    {}

    //! Total number of degrees of freedom
    int size() const;

    //! Map a local degree of freedom on an entity to a global one
    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum) const;

    //- Method inherited from mapper interface
    //! if grid has changed determine new size
    //! (to be called once per timestep, therefore virtual )
    int newSize() const {
      assert(false); // should never get here
    }

    //! old size
    int oldSize() const {
      assert(false); // should never get here
    }

    //! calc new insertion points for dof of different codim
    //! (to be called once per timestep, therefore virtual )
    void calcInsertPoints () {
      assert(false); // should never get here
    }

    //! return max number of local dofs per entity
    int numberOfDofs () const DUNE_DEPRECATED {
      assert(false); // should never get here
    }

    //! return max number of local dofs per entity
    int numDofs () const DUNE_DEPRECATED {
      assert(false); // should never get here
    }

    //! returns true if index is new ( for dof compress )
    bool indexNew (int num) const {
      assert(false); // should never get here
    }

    //! return old index in dof array of given index ( for dof compress )
    int oldIndex (int num) const {
      assert(false); // should never get here
    }

    //! return new index in dof array
    int newIndex (int num) const {
      assert(false); // should never get here
    }

    //! return estimate for size that is addtional needed for restriction
    //! of data
    int additionalSizeEstimate() const {
      assert(false); // should never get here
    }

  private:
    //- Forbidden member
    SubMapper(const ThisType& other);
    ThisType& operator=(const ThisType& other);

  private:
    //- Data members
    const CombinedSpaceType& spc_;
    const ContainedMapperType& mapper_;
    const int component_;

    DofConversionType utilGlobal_;
  };
} // end namespace Dune

#include "subspace.cc"

#endif
