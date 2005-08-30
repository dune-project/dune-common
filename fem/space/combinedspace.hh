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

  // Forward declarations
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedSpace;
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedMapper;
  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  class CombinedBaseFunctionSet;

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  struct CombinedSpaceTraits {
  private:
    typedef DiscreteFunctionSpaceImp ContainedDiscreteFunctionSpaceType;
    //   typedef ContainedDiscreteFunctionSpaceType::MapperType ContainedMapperType;
    typedef typename ContainedDiscreteFunctionSpaceType::Traits
    ContainedTraits;
    typedef typename ContainedTraits::FunctionSpaceType
    ContainedFunctionSpaceType;
    typedef typename ContainedTraits::BaseFunctionSetType
    ContainedBaseFunctionSetType;

    enum { ContainedDimRange = ContainedFunctionSpaceType::DimRange,
           ContainedDimDomain = ContainedFunctionSpaceType::DimDomain };
  public:
    typedef typename ContainedFunctionSpaceType::DomainFieldType
    DomainFieldType;
    typedef typename ContainedFunctionSpaceType::RangeFieldType
    RangeFieldType;

    // * Require contained space to have dimRange == 1? (ie to be scalar)
    typedef FunctionSpace<
        DomainFieldType, RangeFieldType,
        ContainedDimDomain, ContainedDimRange*N
        > FunctionSpaceType;

    typedef typename FunctionSpaceType::RangeType RangeType;
    typedef typename FunctionSpaceType::DomainType DomainType;
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename FunctionSpaceType::DomainFieldType DomainFieldType;

    typedef CombinedSpace<
        DiscreteFunctionSpaceImp, N, policy
        > DiscreteFunctionSpaceType;

    typedef typename ContainedTraits::GridType GridType;
    typedef typename ContainedTraits::IteratorType IteratorType;
    typedef CombinedBaseFunctionSet<
        ContainedBaseFunctionSetType, N, policy
        > BaseFunctionSetType;

    friend class DiscreteFunctionSpaceType;
  };

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  class CombinedSpace :
    public DiscreteFunctionSpaceDefault<
        CombinedSpaceTraits<DiscreteFunctionSpaceImp, N, policy>
        >
  {
  private:
    //- Private typedefs
    typedef CombinedMapper<DiscreteFunctionSpaceImp, N, policy> MapperType;
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
  public:
    //- Public methods
    //! constructor
    CombinedSpace(ContainedDiscreteFunctionSpaceType& spc);

    //! destructor
    ~CombinedSpace();

    //! continuous?
    bool continuous() const { return spc_.continous(); }

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

    template <class EntityType>
    const BaseFunctionSetType& getBaseFunctionSet(EntityType& en) const
    {
      GeometryType geo = en.geometry().type();
      int dimension = static_cast<int>(EntityType::mydimension);
      return *baseSetVec_[GeometryIdentifier::fromGeo(dimension, geo)];
    }

    GridType& grid() { return spc_.grid(); }
    const GridType& grid() const { return spc_.grid(); }


    template <class DiscFuncType>
    typename DiscFuncType::MemObjectType &
    signIn(DiscFuncType & df) const { return spc_.signIn(df); }

    //! sign out to dofmanager, dofmanager frees the memory
    template <class DiscFuncType>
    bool signOut ( DiscFuncType & df) const { return spc_.signOut(df); }

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

  }; // end class CombinedSpace

  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  struct CombinedBaseFunctionSetTraits {
  private:
    typedef typename BaseFunctionSetImp::DiscreteFunctionSpaceType
    ContainedFunctionSpaceType;
  public:
    typedef CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy>
    BaseFunctionSetType;

    typedef CombinedSpace<ContainedFunctionSpaceType, N, policy>
    DiscreteFunctionSpaceType;

    typedef DiscreteFunctionSpaceType::RangeType RangeType;
    typedef DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType
    JacobianRangeType;

    typedef typename DiscreteFunctionSpaceType::HessianRangeType
    HessianRangeType;

    enum { DimDomain = DiscreteFunctionSpaceType::DimDomain };
    enum { DimRange  = DiscreteFunctionSpaceType::DimRange  };
  };

  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  class CombinedBaseFunctionSet :
    public BaseFunctionSetDefault<
        CombinedBaseFunctionSetTraits<BaseFunctionSetImp, N, policy>
        >
  {
  private:
    //- Private typedefs
    typedef BaseFunctionSetImp ContainedBaseFunctionSetType;
    typedef ContainedBaseFunctionSetType::RangeType ContainedRangeType;

  public:
    //- Typedefs and enums
    enum { numComponents = N };
    typedef CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy> ThisType;

    typedef CombinedBaseFunctionSetTraits<BaseFunctionSetImp, N, policy> Traits;
    typedef typename Traits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::DomainType DomainType;

  public:
    //- Public methods
    CombinedBaseFunctionSet(const ContainedBaseFunctionSetType& bfSet) :
      containedResult_(0.0),
      baseFunctionSet_(bfSet)
    {}

    int getNumberOfBaseFunctions() const {
      return baseFunctionSet_.getNumberOfBaseFunctions()*N;
    }

    template <int diffOrd>
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   const DomainType & x, RangeType & phi ) const;

    template <int diffOrd, class QuadratureType >
    void evaluate (int baseFunct,
                   const FieldVector<deriType, diffOrd> &diffVariable,
                   QuadratureType & quad,
                   int quadPoint, RangeType & phi ) const;

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

  public:
    //- Public methods
    CombinedMapper(const ContainedDiscreteFunctionSpaceType& spc) :
      spc_(spc)
    {}

    int size() const;

    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum) const;

  private:
    //- Private methods
    CombinedMapper(const ThisType& other);

    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum, Int2Type<PointBased>) const;

    template <class EntityType>
    int mapToGlobal(EntityType& en, int localNum, Int2Type<VariableBased>) const;

    int containedDof(int combinedDofNum) const;
    int component(int combinedDofNum) const;

  private:
    //- Data members
    const ContainedDiscreteFunctionSpaceType& spc_;

    //int numBaseLoc_;
  }; // end class CombinedMapper

} // end namespace Dune

// include implementation
#include "combinedspace.cc"



#endif
