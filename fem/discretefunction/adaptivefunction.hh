// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ADAPTIVEFUNCTION_HH
#define DUNE_ADAPTIVEFUNCTION_HH

//- System includes
#include <string>

//- Dune includes
#include <dune/fem/common/discretefunction.hh>
#include <dune/fem/common/localfunction.hh>
#include <dune/fem/space/combinedspace.hh>
#include <dune/fem/space/subspace.hh>
#include <dune/fem/dofmanager.hh>

//- Local includes
#include "adaptiveimp.hh"

namespace Dune {

  //- Forward declarations
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveDiscreteFunction;
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveLocalFunction;

  //- Class definitions
  //! Traits class for AdaptiveDiscreteFunction and AdaptiveLocalFunction
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  struct AdaptiveDiscreteFunctionTraits {
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    typedef DofManagerImp DofManagerType;

    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp, DofManagerImp>
    DiscreteFunctionType;
    typedef AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>
    LocalFunctionType;

    typedef typename DiscreteFunctionSpaceType::RangeFieldType DofType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;
    typedef typename DiscreteFunctionSpaceType::MapperType MapperType;

    typedef DofArray<DofType> DofStorageType;
    typedef typename DofManagerType::template Traits<MapperType, DofStorageType>::MemObjectType MemObjectType;

    typedef typename DofStorageType::DofIteratorType DofIteratorType;
    typedef typename DofStorageType::ConstDofIteratorType ConstDofIteratorType;

  }; // end class AdaptiveDiscreteFunctionTraits


  //! An adaptive discrete function
  //! This class is comparable to DFAdapt, except that it provides a
  //! specialisation for CombinedSpace objects which provides enriched
  //! functionality (access to subfunctions) and runtime optimisations
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveDiscreteFunction :
    public DiscreteFunctionDefault<
        AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp, DofManagerImp> >,
    private AdaptiveFunctionImplementation<
        DiscreteFunctionSpaceImp, DofManagerImp>
  {
  public:
    //- friends

  private:
    typedef AdaptiveDiscreteFunction<
        DiscreteFunctionSpaceImp, DofManagerImp> MyType;
    typedef AdaptiveFunctionImplementation<
        DiscreteFunctionSpaceImp, DofManagerImp> Imp;
    typedef AdaptiveDiscreteFunctionTraits<
        DiscreteFunctionSpaceImp, DofManagerImp> MyTraits;
    typedef DiscreteFunctionDefault<MyTraits> BaseType;

  public:
    //- Typedefs and enums
    //! Traits class with all necessary type definitions
    typedef MyTraits Traits;
    //! Class containing the actual implementation
    typedef Imp ImplementationType;
    //! Discrete function space this discrete function belongs to
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;

    //! Local function type
    typedef typename Traits::LocalFunctionType LocalFunctionType;
    //! Discrete function type (identical to this type, needed as
    //! Barton-Nackman parameter
    typedef typename Traits::DiscreteFunctionType DiscreteFunctionType;

    //! Intrinsic type used for the dofs (typically a float type)
    typedef typename Traits::DofType DofType;
    //! Intrinsic type used for the range field (identical to DofType)
    typedef typename Traits::RangeFieldType RangeFieldType;
    //! Vector type used for the range field
    typedef typename Traits::RangeType RangeType;
    //! Vector type used for the domain field
    typedef typename Traits::DomainType DomainType;
    //! Mapper type (from the space)
    typedef typename Traits::MapperType MapperType;

    //! Container class type for the dofs (managed by the DofManager)
    typedef typename Traits::DofStorageType DofStorageType;
    //! Memory management class for DofStorageType (manager by the DofManager)
    typedef typename Traits::MemObjectType MemObjectType;

    //! Iterator over dof container
    typedef typename Traits::DofIteratorType DofIteratorType;
    //! Read-only iterator over dof container
    typedef typename Traits::ConstDofIteratorType ConstDofIteratorType;
  public:
    //- Public methods
    //! Constructor
    AdaptiveDiscreteFunction(std::string name,
                             const DiscreteFunctionSpaceType& spc) :
      BaseType(spc),
      Imp(name, spc)
    {}

    //! Constructor for SubDiscreteFunctions
    //! This constructor is only called internally
    AdaptiveDiscreteFunction(std::string name,
                             const DiscreteFunctionSpaceType& spc,
                             DofStorageType& dofVec) :
      BaseType(spc),
      Imp(name, spc, dofVec)
    {}

    //! Copy constructor
    //! The copy constructor copies the dofs
    AdaptiveDiscreteFunction(const MyType& other) :
      BaseType(other.space()),
      Imp(other)
    {}

    using Imp::name;
    using Imp::size;
    using Imp::dbegin;
    using Imp::dend;
    using Imp::newLocalFunction;
    using Imp::localFunction;
    using Imp::write_xdr;
    using Imp::read_xdr;
    using Imp::write_ascii;
    using Imp::read_ascii;
    using Imp::write_pgm;
    using Imp::read_pgm;
  private:
    //- Forbidden members
  }; // end class AdaptiveDiscreteFunction

  // Note: could use Traits class for Barton-Nackman instead
  //! Local function belonging to AdaptiveDiscreteFunction
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveLocalFunction :
    public LocalFunctionDefault<
        DiscreteFunctionSpaceImp,
        AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp> >
  {
  public:
    friend class AdaptiveFunctionImplementation<
        DiscreteFunctionSpaceImp, DofManagerImp>;

  private:
    typedef AdaptiveLocalFunction<
        DiscreteFunctionSpaceImp, DofManagerImp> ThisType;
  public:
    //- Public typedefs and enums
    //! The discrete function space this local function belongs to
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    //! The discrete function this local function belongs to
    typedef AdaptiveDiscreteFunction<
        DiscreteFunctionSpaceImp, DofManagerImp> DiscreteFunctionType;
    //! Traits class with type definitions for AdaptiveDiscreteFunction and
    //! AdaptiveLocalFunction
    typedef AdaptiveDiscreteFunctionTraits<
        DiscreteFunctionSpaceType, DofManagerImp> Traits;
    //! Traits class of DiscreteFunctionSpaceType
    typedef typename DiscreteFunctionSpaceType::Traits SpaceTraits;

    //! Function space type
    typedef typename SpaceTraits::FunctionSpaceType FunctionSpaceType;
    //! The base function set of DiscreteFunctionSpaceType
    typedef typename SpaceTraits::BaseFunctionSetType BaseFunctionSetType;

    //! Intrinsic data type for range field
    typedef typename Traits::RangeFieldType RangeFieldType;
    //! Vector type for the domain field
    typedef typename Traits::DomainType DomainType;
    //! Vector type for the range field
    typedef typename Traits::RangeType RangeType;
    //! Tensor type for the jacobian
    typedef typename Traits::JacobianRangeType JacobianRangeType;
    //! Intrinsic data type for the degrees of freedom (dof)
    typedef RangeFieldType DofType;

    //! Container class type for the dofs
    typedef typename Traits::DofStorageType DofStorageType;

    //! Dimension of the range field
    enum { dimRange = DiscreteFunctionSpaceType::DimRange };

  public:
    //- Public methods
    //- Constructors and destructors

    //! Constructor
    AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                          DofStorageType& dofVec);

    //! Copy constructor
    AdaptiveLocalFunction(const ThisType& other);

    //! Destructor
    ~AdaptiveLocalFunction();

    //- Operators
    //! Random access operator
    DofType& operator[] (int num);

    //! Cosnt random access operator
    const DofType& operator[] (int num) const;

    //- Methods
    //! Number of dofs on this element
    int numberOfDofs() const;

    //! Evaluation of the discrete function
    template <class EntityType>
    void evaluateLocal(EntityType& en,
                       const DomainType& x,
                       RangeType & ret) const;

    //! Evaluation of the discrete function
    template <class EntityType, class QuadratureType>
    void evaluate(EntityType& en,
                  QuadratureType& quad,
                  int quadPoint,
                  RangeType& ret) const;

    //! Jacobian of the discrete function
    template <class EntityType>
    void jacobianLocal(EntityType& en,
                       const DomainType& x,
                       JacobianRangeType& ret) const;

    //! Jacobian of the discrete function
    template <class EntityType, class QuadratureType>
    void jacobian(EntityType& en,
                  QuadratureType& quad,
                  int quadPoint,
                  JacobianRangeType& ret) const;

  private:
    //- Forbidden methods
    //! Copy constructor
    ThisType& operator=(const ThisType& other);

    template <class EntityType>
    void init(EntityType& en);
  private:
    //- Data members
    const DiscreteFunctionSpaceType& spc_;
    DofStorageType& dofVec_;

    mutable std::vector<RangeFieldType*> values_;

    mutable RangeType tmp_;
    mutable JacobianRangeType tmpGrad_;
  }; // end class AdaptiveLocalFunction

  //- Specialisations
  //! Specialised version of AdaptiveDiscreteFunction for CombinedSpace
  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  class AdaptiveDiscreteFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp> :
    public DiscreteFunctionDefault<AdaptiveDiscreteFunctionTraits<CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp > >,
    private AdaptiveFunctionImplementation<CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>
  {
  private:
    typedef CombinedSpace<
        ContainedFunctionSpaceImp, N, p> DiscreteFunctionSpaceImp;
    typedef AdaptiveDiscreteFunction<
        DiscreteFunctionSpaceImp, DofManagerImp> MyType;
    typedef AdaptiveFunctionImplementation<
        DiscreteFunctionSpaceImp, DofManagerImp> Imp;
    typedef AdaptiveDiscreteFunctionTraits<
        DiscreteFunctionSpaceImp, DofManagerImp> MyTraits;
    typedef DiscreteFunctionDefault<MyTraits> BaseType;

  public:
    //- Typedefs and enums
    typedef MyTraits Traits;
    typedef Imp ImplementationType;
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;

    typedef typename Traits::LocalFunctionType LocalFunctionType;
    typedef typename Traits::DiscreteFunctionType DiscreteFunctionType;

    typedef typename Traits::DofType DofType;
    typedef typename Traits::RangeFieldType RangeFieldType;
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::DomainType DomainType;
    typedef typename Traits::MapperType MapperType;

    typedef typename Traits::DofStorageType DofStorageType;
    typedef typename Traits::MemObjectType MemObjectType;

    typedef typename Traits::DofIteratorType DofIteratorType;
    typedef typename Traits::ConstDofIteratorType ConstDofIteratorType;

    //- Additional typedefs
    typedef SubSpace<DiscreteFunctionSpaceType> SubSpaceType;
    typedef AdaptiveDiscreteFunction<
        SubSpaceType, DofManagerImp> SubDiscreteFunctionType;

  public:
    //- Public methods
    //! Constructor
    AdaptiveDiscreteFunction(std::string name,
                             const DiscreteFunctionSpaceType& spc) :
      BaseType(spc),
      Imp(name, spc),
      subSpaces_(0)
    {}

    //! Constructor
    AdaptiveDiscreteFunction(std::string name,
                             const DiscreteFunctionSpaceType& spc,
                             DofStorageType& dofVec) :
      BaseType(spc),
      Imp(name, spc, memObj),
      subSpaces_(0)
    {}

    //! Copy constructor
    AdaptiveDiscreteFunction(const MyType& other) :
      BaseType(other.space()),
      Imp(other),
      subSpaces_(0) // Don't copy them
    {}

    ~AdaptiveDiscreteFunction();

    using Imp::name;
    using Imp::size;
    using Imp::dbegin;
    using Imp::dend;
    using Imp::newLocalFunction;
    using Imp::localFunction;
    using Imp::write_xdr;
    using Imp::read_xdr;
    using Imp::write_ascii;
    using Imp::read_ascii;
    using Imp::write_pgm;
    using Imp::read_pgm;

    //- Additional methods
    SubDiscreteFunctionType subFunction(int component);

    int numComponents() const { return N; }

  private:
    std::vector<SubSpaceType*> subSpaces_;
  }; // end class AdaptiveDiscreteFunction (specialised for CombinedSpace)

  //- class AdaptiveLocalFunction (specialised)
  //! Specialised version of AdaptiveLocalFunction for CombinedSpace
  template <
      class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp
      >
  class AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>
    : public LocalFunctionDefault<
          CombinedSpace<ContainedFunctionSpaceImp, N, p>,
          AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>,
              DofManagerImp>
          >
  {
  public:
    //- Friends
    friend class AdaptiveFunctionImplementation<
        CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>;
    //friend class AdaptiveDiscreteFunction<
    //  CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>;

  public:
    //- Public typedefs and enums
    typedef CombinedSpace<
        ContainedFunctionSpaceImp, N, p> DiscreteFunctionSpaceType;
    typedef AdaptiveLocalFunction<
        DiscreteFunctionSpaceType, DofManagerImp> ThisType;
    typedef AdaptiveDiscreteFunctionTraits<
        DiscreteFunctionSpaceType, DofManagerImp> Traits;
    typedef typename DiscreteFunctionSpaceType::Traits SpaceTraits;

    enum { dimRange = DiscreteFunctionSpaceType::DimRange };

    typedef typename SpaceTraits::ContainedRangeType ContainedRangeType;
    typedef typename SpaceTraits::ContainedJacobianRangeType
    ContainedJacobianRangeType;
    typedef typename SpaceTraits::BaseFunctionSetType BaseFunctionSetType;

    typedef typename Traits::RangeFieldType RangeFieldType;
    typedef typename Traits::DomainType DomainType;
    typedef typename Traits::RangeType RangeType;
    typedef typename Traits::JacobianRangeType JacobianRangeType;
    typedef typename Traits::DofType DofType;

    typedef typename Traits::DiscreteFunctionType DiscreteFunctionType;
    typedef typename Traits::DofStorageType DofStorageType;

    typedef FieldVector<DofType, dimRange> DofVectorType;

  public:
    //- Public methods
    //- Constructors and destructors

    //! Constructor
    AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                          DofStorageType& dofVec);

    //! Copy constructor
    AdaptiveLocalFunction(const ThisType& other);

    //! Destructor
    ~AdaptiveLocalFunction();

    //- Operators
    //! Random access operator
    DofType& operator[] (int num);

    //! Cosnt random access operator
    const DofType& operator[] (int num) const;

    //- Methods
    int numberOfDofs() const;

    template <class EntityType>
    void evaluateLocal(EntityType& en,
                       const DomainType& x,
                       RangeType & ret) const;

    template <class EntityType, class QuadratureType>
    void evaluate(EntityType& en,
                  QuadratureType& quad,
                  int quadPoint,
                  RangeType & ret) const;

    template <class EntityType>
    void jacobianLocal(EntityType& en,
                       const DomainType& x,
                       JacobianRangeType& ret) const;

    template <class EntityType, class QuadratureType>
    void jacobian(EntityType& en,
                  QuadratureType& quad,
                  int quadPoint,
                  JacobianRangeType& ret) const;

    //- Additional methods for specialisation
    void assign(int dofNum, const DofVectorType& dofs);

    int numberOfBaseFunctions() const;

  private:
    //- Private methods
    template <class EntityType>
    void init(EntityType& en);

  private:
    //- Member data
    const DiscreteFunctionSpaceType& spc_;
    DofStorageType& dofVec_;

    mutable std::vector<FieldVector<DofType*, N> > values_;

    mutable RangeType tmp_;
    mutable ContainedRangeType cTmp_;
    mutable ContainedJacobianRangeType cTmpGrad_;
    mutable JacobianRangeType tmpGrad_;
  }; // end class AdaptiveLocalFunction (specialised for CombinedSpace)

} // end namespace Dune

#include "adaptivefunction.cc"

#endif
