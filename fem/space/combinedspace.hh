// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMBINEDSPACE_HH
#define DUNE_COMBINEDSPACE_HH

//- System includes
#include <vector>

//- Dune includes
#include <dune/fem/common/discretefunctionspace.hh>
#include <dune/fem/common/basefunctions.hh>
#include <dune/fem/common/dofmapperinterface.hh>
#include <dune/fem/space/subspace.hh>
#include <dune/common/misc.hh>

namespace Dune {

  // Note: the methods component and contained dof do actually the same
  // in the Mapper and in the BaseFunctionSet. It would be a good idea to
  // factor them out in a common class (private inheritance? BaseFunctionSet
  // provides them for the mapper?

  //! Indicates how the dofs shall be stored in the discrete functions
  //! Point based means that all dofs belonging to one local degree in a
  //! contained spaced are stored consecutively, whereas in the variable based
  //! approach all dofs belonging to one subspace are stored consecutively
  enum DofStoragePolicy { PointBased, VariableBased };


  //! Utility class that helps in the transformation between dofs in the
  //! combined space and its enclosed spaces
  template <DofStoragePolicy p>
  class DofConversionUtility {
  public:
    DofConversionUtility(int size);

    void newSize(int size);

    int component(int combinedIndex) const;
    int containedDof(int combinedIndex) const;

    int combinedDof(int enclosedIndex, int component) const;
  private:
    int size_;
  };

  template <>
  class DofConversionUtility<PointBased> {
  public:
    DofConversionUtility(int numComponents) :
      numComponents_(numComponents)
    {}

    void newSize(int size) {} // just do nothing

    int component(int combinedIndex) const {
      return combinedIndex%numComponents_;
    }
    int containedDof(int combinedIndex) const {
      return combinedIndex/numComponents_;
    }

    int combinedDof(int containedIndex, int component) const {
      return containedIndex*numComponents_ + component;
    }

  private:
    const int numComponents_;
  };

  template <>
  class DofConversionUtility<VariableBased> {
  public:
    DofConversionUtility(int size) :
      size_(size)
    {}

    void newSize(int size) { size_ = size; }

    int component(int combinedIndex) const {
      return combinedIndex/size_;
    }
    int containedDof(int combinedIndex) const {
      return combinedIndex%size_;
    }

    int combinedDof(int containedIndex, int component) const {
      return containedIndex + component*size_;
    }

  private:
    int size_;
  };

  // Forward declarations
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedSpace;
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedMapper;
  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  class CombinedBaseFunctionSet;

  //! Traits class for CombinedSpace
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  struct CombinedSpaceTraits {
  private:
    typedef DiscreteFunctionSpaceImp ContainedDiscreteFunctionSpaceType;

    typedef typename ContainedDiscreteFunctionSpaceType::Traits
    ContainedSpaceTraits;
    typedef typename ContainedSpaceTraits::FunctionSpaceType
    ContainedFunctionSpaceType;
    typedef typename ContainedSpaceTraits::BaseFunctionSetType
    ContainedBaseFunctionSetType;
    typedef typename ContainedSpaceTraits::MapperType ContainedMapperType;

    enum { ContainedDimRange = ContainedFunctionSpaceType::DimRange,
           ContainedDimDomain = ContainedFunctionSpaceType::DimDomain };
  public:
    typedef typename ContainedFunctionSpaceType::DomainFieldType
    DomainFieldType;
    typedef typename ContainedFunctionSpaceType::RangeFieldType
    RangeFieldType;
    typedef typename ContainedFunctionSpaceType::RangeType
    ContainedRangeType;
    typedef typename ContainedFunctionSpaceType::JacobianRangeType
    ContainedJacobianRangeType;

    typedef CombinedSpace<
        DiscreteFunctionSpaceImp, N, policy> DiscreteFunctionSpaceType;
    typedef FunctionSpace<
        DomainFieldType, RangeFieldType,
        ContainedDimDomain, ContainedDimRange*N> FunctionSpaceType;
    typedef CombinedBaseFunctionSet<
        DiscreteFunctionSpaceImp, N, policy> BaseFunctionSetType;
    typedef CombinedMapper<DiscreteFunctionSpaceImp, N, policy> MapperType;

    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::JacobianRangeType JacobianRangeType;

    typedef typename ContainedSpaceTraits::GridType GridType;
    typedef typename ContainedSpaceTraits::IteratorType IteratorType;

    typedef DofConversionUtility<policy> DofConversionType;

    enum { DimRange = FunctionSpaceType::DimRange,
           DimDomain = FunctionSpaceType::DimDomain };
  public:
    //- Friends
    friend class DiscreteFunctionSpaceType;
    friend class BaseFunctionSetType;
    friend class MapperType;
  };

  //! Class to combine N scalar spaces
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedSpace :
    public DiscreteFunctionSpaceDefault<
        CombinedSpaceTraits<DiscreteFunctionSpaceImp, N, policy>
        >
  {
  private:
    //- Private typedefs
    typedef DiscreteFunctionSpaceDefault<
        CombinedSpaceTraits<DiscreteFunctionSpaceImp, N, policy>
        > BaseType;
  public:
    //- Public typedefs and enums
    typedef CombinedSpace<DiscreteFunctionSpaceImp, N, policy> ThisType;
    typedef CombinedSpaceTraits<DiscreteFunctionSpaceImp, N, policy> Traits;
    typedef DiscreteFunctionSpaceImp ContainedDiscreteFunctionSpaceType;

    typedef typename Traits::IteratorType IteratorType;

    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::DomainType DomainType;
    typedef typename Traits::RangeFieldType RangeFieldType;
    typedef typename Traits::DomainFieldType DomainFieldType;

    typedef typename Traits::ContainedRangeType ContainedRangeType;
    typedef typename Traits::ContainedJacobianRangeType ContainedJacobianRangeType;

    typedef typename Traits::BaseFunctionSetType BaseFunctionSetType;
    typedef typename Traits::MapperType MapperType;

    typedef typename Traits::DofConversionType DofConversionType;

    CompileTimeChecker<(Traits::ContainedDimRange == 1)> use_CombinedSpace_only_with_scalar_spaces;
  public:
    //- Public methods
    //! constructor
    CombinedSpace(ContainedDiscreteFunctionSpaceType& spc);

    //! destructor
    ~CombinedSpace();

    //! type
    int type() const { return spaceId_; }

    //! continuous?
    bool continuous() const { return spc_.continous(); }

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

    //! access to base function set
    template <class EntityType>
    const BaseFunctionSetType& getBaseFunctionSet(EntityType& en) const
    {
      GeometryType geo = en.geometry().type();
      int dimension = static_cast<int>(EntityType::mydimension);

      assert(baseSetVec_[GeometryIdentifier::fromGeo(dimension, geo)]);
      return *baseSetVec_[GeometryIdentifier::fromGeo(dimension, geo)];
    }

    //! access to grid
    GridType& grid() { return spc_.grid(); }

    //! access to grid (const version)
    const GridType& grid() const { return spc_.grid(); }

    //! access to mapper
    const MapperType& mapper() const { return mapper_; }

    //- Additional methods
    //! number of components
    int numComponents() const { return N; }

    //! policy of this space
    DofStoragePolicy policy() const { return policy; }
  private:
    //- Private typedefs
    typedef typename Traits::ContainedMapperType ContainedMapperType;

    //- Friend
    friend class SubSpace<ThisType>;

  private:
    //- Private methods
    CombinedSpace(const ThisType& other);

    const ContainedMapperType& containedMapper() const {
      return mapper_.containedMapper();
    }

  private:
    //- Member data
    ContainedDiscreteFunctionSpaceType& spc_;

    MapperType mapper_;
    std::vector<BaseFunctionSetType*> baseSetVec_;

    static const int spaceId_;
  }; // end class CombinedSpace

  //! Wrapper class for base function sets. This class is used within
  //! CombinedSpace
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedBaseFunctionSet :
    public BaseFunctionSetDefault<
        CombinedSpaceTraits<DiscreteFunctionSpaceImp, N, policy>
        >
  {
  public:
    //- Typedefs and enums
    enum { numComponents = N };
    typedef CombinedBaseFunctionSet<
        DiscreteFunctionSpaceImp, N, policy> ThisType;
    typedef CombinedSpaceTraits<
        DiscreteFunctionSpaceImp, N, policy> Traits;

    typedef typename Traits::DiscreteFunctionSpaceType
    DiscreteFunctionSpaceType;
    typedef typename Traits::ContainedBaseFunctionSetType ContainedBaseFunctionSetType;
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::DomainType DomainType;
    typedef typename Traits::ContainedRangeType ContainedRangeType;
    typedef typename Traits::ContainedJacobianRangeType ContainedJacobianRangeType;
  public:
    //- Public methods
    //! Constructor
    CombinedBaseFunctionSet(const ContainedBaseFunctionSetType& bfSet) :
      containedResult_(0.0),
      baseFunctionSet_(bfSet),
      util_(N)
    {}

    //! Number of base functions
    //! The number of base functions equals the total number of degrees of
    //! freedom (dof), since the dofs are considered to be scalar and the
    //! combined base functions to be vector valued
    int getNumberOfBaseFunctions() const {
      return baseFunctionSet_.getNumberOfBaseFunctions()*N;
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

    //- Additional methods
    int numContainedFunctions() const {
      return baseFunctionSet_.getNumberOfBaseFunctions();
    }

    //! evaluate base function
    void evaluateContained(int baseFunct,
                           const DomainType& x,
                           ContainedRangeType& phi) const;

    //! evaluate base function at quadrature point
    void jacobianContained (int baseFunct,
                            const DomainType& x,
                            ContainedJacobianRangeType& phi) const;


  private:
    //- Private methods
    CombinedBaseFunctionSet(const ThisType& other);

    //int containedDof(int combinedDofNum) const;
    //int component(int combinedDofNum) const;
    void expand(int baseFunct,
                const ContainedRangeType& arg,
                RangeType& dest) const;

  private:
    //- Data members
    mutable ContainedRangeType containedResult_;
    const ContainedBaseFunctionSetType& baseFunctionSet_;
    const DofConversionUtility<PointBased> util_;
  }; // end class CombinedBaseFunctionSet

  //! Wrapper class for mappers. This class is to be used in conjunction with
  //! the CombinedSpace
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedMapper : public DofMapperDefault<
                             CombinedMapper<DiscreteFunctionSpaceImp, N, policy>
                             >
  {
  public:
    //- Friends
    friend class CombinedSpace<DiscreteFunctionSpaceImp, N, policy>;

  public:
    //- Typedefs and enums
    enum { numComponents = N };
    typedef CombinedMapper<DiscreteFunctionSpaceImp, N, policy> ThisType;

    typedef typename CombinedSpaceTraits<DiscreteFunctionSpaceImp, N, policy> SpaceTraits;
    typedef DiscreteFunctionSpaceImp ContainedDiscreteFunctionSpaceType;
    typedef typename DiscreteFunctionSpaceImp::MapperType ContainedMapperType;
  public:
    //- Public methods
    //! Constructor
    CombinedMapper(const ContainedDiscreteFunctionSpaceType& spc,
                   const ContainedMapperType& mapper) :
      spc_(spc),
      mapper_(mapper),
      utilLocal_(N),
      utilGlobal_(policy == PointBased ? N : spc.size())
    {}

    //! Total number of degrees of freedom
    int size() const;

    //! Map a local degree of freedom on an entity to a global one
    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum) const;

    //- Method inherited from mapper interface
    //! if grid has changed determine new size
    //! (to be called once per timestep, therefore virtual )
    int newSize() const { return mapper_.newSize()*N; }

    //! old size
    int oldSize() const { return mapper_.oldSize()*N; }

    //! calc new insertion points for dof of different codim
    //! (to be called once per timestep, therefore virtual )
    void calcInsertPoints () {
      const_cast<ContainedMapperType&>(mapper_).calcInsertPoints();
      // * is this right here?
      utilGlobal_.newSize(mapper_.size());
    }

    //! return max number of local dofs per entity
    int numberOfDofs () const { return mapper_.numberOfDofs()*N; }

    //! returns true if index is new ( for dof compress )
    bool indexNew (int num) const {
      assert(false); // * check correctness first: do I correctly map between combinedMapper and containedMapper indices with utilGlobal
      return mapper_.indexNew(utilGlobal_.containedDof(num));
    }

    //! return old index in dof array of given index ( for dof compress )
    int oldIndex (int num) const;

    //! return new index in dof array
    int newIndex (int num) const;

    //! return estimate for size that is addtional needed for restriction
    //! of data
    int additionalSizeEstimate() const {
      return mapper_.additionalSizeEstimate()*N;
    }

  private:
    //- Private methods
    CombinedMapper(const ThisType& other);

    const ContainedMapperType& containedMapper() const {
      return mapper_;
    }

    static int chooseSize(int pointBased, int variableBased,
                          Int2Type<PointBased>) {
      return pointBased;
    }

    static int chooseSize(int pointBased, int variableBased,
                          Int2Type<VariableBased>) {
      return variableBased;
    }

  private:
    //- Data members
    const ContainedDiscreteFunctionSpaceType& spc_;
    const ContainedMapperType& mapper_;

    const DofConversionUtility<PointBased> utilLocal_;
    DofConversionUtility<policy> utilGlobal_;
  }; // end class CombinedMapper

} // end namespace Dune

// include implementation
#include "combinedspace.cc"

#endif
