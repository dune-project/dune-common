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

  /* just an idea... the pointbased specialisation would need additional information
     template <DofStoragePolicy p, int N>
     struct DofStorageUtility {
     static int containedDof(int num);
     static int component(int num);
     };

     template <int N>
     struct DofStorageUtility<PointBased> {
     static int containedDof(int num) { return num/N; }
     static int component(int num) { return num/N; }
     };

     template <int N>
     struct DofStorageUtility<VariableBased> {
     static int containedDof(int num) { return num/N; }
     static int component(int num) { return num/N; }
     };
   */

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
      mapper_.mapToGlobal(en, local);
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
  private:
    //- Private typedefs

  private:
    //- Private methods
    CombinedSpace(const ThisType& other);

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
      baseFunctionSet_(bfSet)
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
      return baseFunctionSet_.getNumberOfBaseFunctions()*N;
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

    int containedDof(int combinedDofNum) const;
    int component(int combinedDofNum) const;
    void expand(int baseFunct,
                const ContainedRangeType& arg,
                RangeType& dest) const;

  private:
    //- Data members
    mutable ContainedRangeType containedResult_;
    const ContainedBaseFunctionSetType& baseFunctionSet_;
  }; // end class CombinedBaseFunctionSet

  //! Wrapper class for mappers. This class is to be used in conjunction with
  //! the CombinedSpace
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedMapper : public DofMapperDefault<
                             CombinedMapper<DiscreteFunctionSpaceImp, N, policy>
                             >
  {
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
      mapper_(mapper)
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
    void calcInsertPoints () { const_cast<ContainedMapperType&>(mapper_).calcInsertPoints(); }

    //! return max number of local dofs per entity
    int numberOfDofs () const { return mapper_.numberOfDofs()*N; }

    //! returns true if index is new ( for dof compress )
    bool indexNew (int num) const {
      //assert(false); // do I need to translate that into num of mapper?
      return mapper_.indexNew(containedDof(num));
    }

    //! return old index in dof array of given index ( for dof compress )
    int oldIndex (int num) const {
      assert(false); // gets a bit more complicated here...
      return -1;
    }

    //! return new index in dof array
    int newIndex (int num) const {
      assert(false);
      return -1;
    }

    //! return estimate for size that is addtional needed for restriction
    //! of data
    int additionalSizeEstimate() const {
      return mapper_.additionalSizeEstimate()*N;
    }

  private:
    //- Private methods
    CombinedMapper(const ThisType& other);

    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum, Int2Type<PointBased>) const;
    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum, Int2Type<VariableBased>) const;

    /*
       int newIndex(int num, Int2Type<PointBased>) const;
       int newIndex(int num, Int2Type<VariableBased>) const;

       int oldIndex(int num, Int2Type<PointBased>) const;
       int oldIndex(int num, Int2Type<VariableBased>) const;

       // calculates the global index for the contained mapper out of a global index of the combined mapper
       int containedIndex(int globalIndex, Int2Type<PointBased>) const;
       int containedIndex(int globalIndex, Int2Type<VariableBased>) const;
     */
    // determines the local index (index on a given entity) for the contained mapper
    int containedDof(int combinedDofNum) const;
    // determines which index in the range vector this local index belongs to
    int component(int combinedDofNum) const;

  private:
    //- Data members
    const ContainedDiscreteFunctionSpaceType& spc_;
    const ContainedMapperType& mapper_;
  }; // end class CombinedMapper

} // end namespace Dune

// include implementation
#include "combinedspace.cc"

#endif
