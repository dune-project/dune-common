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

//- Local includes
#include "adaptiveimp.hh"

namespace Dune {

  //- Forward declarations
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveDiscreteFunction;
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveLocalFunction;

  //- Class definitions
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
    typedef typename DiscreteFunctionSpaceType::MapperType MapperType;

    typedef DofArray<DofType> DofStorageType;
    typedef typename DofManagerType::template Traits<MapperType, DofStorageType>::MemObjectType MemObjectType;

    typedef typename DofStorageType::DofIteratorType DofIteratorType;
    typedef typename DofStorageType::ConstDofIteratorType ConstDofIteratorType;

  }; // end class AdaptiveDiscreteFunctionTraits

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveDiscreteFunction :
    public DiscreteFunctionDefault<
        AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp, DofManagerImp> >,
    private AdaptiveFunctionImplementation<
        DiscreteFunctionSpaceImp, DofManagerImp>
  {
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
    typedef MyTraits Traits;
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
  public:
    //- Public methods
    AdaptiveDiscreteFunction(std::string name,
                             const DiscreteFunctionSpaceType& spc) :
      BaseType(spc),
      Imp(name, spc)
    {}

    using Imp::name;
    using Imp::dbegin;
    using Imp::dend;
    using Imp::newLocalFunction;
    using Imp::localFunction;
    using Imp::write_xdr;
    using Imp::read_xdr;
    using Imp::write_ascii;
    using Imp::read_ascii;
  }; // end class AdaptiveDiscreteFunction

  // Note: could use Traits class for Barton-Nackman instead
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  class AdaptiveLocalFunction :
    public LocalFunctionDefault<
        DiscreteFunctionSpaceImp,
        AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp> >
  {
    friend class AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp, DofManagerImp>;
  public:
    //- Public typedefs and enums
    typedef AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp> ThisType;
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp, DofManagerImp> DiscreteFunctionSpaceType;

    //! these are the types for the derived classes
    typedef typename DiscreteFunctionSpaceType::FunctionSpaceType FunctionSpaceType;
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;
    typedef RangeFieldType DofType;
    typedef typename DiscreteFunctionSpaceType::DofStorageType DofStorageType;

    enum { dimRange = DiscreteFunctionSpaceType::DimRange };

  public:
    //- Public methods
    //- Constructors and destructors

    //! Constructor
    AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                          DofStorageType& dofVec);

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
                       RangeType & ret);

    template <class EntityType>
    void jacobianLocal(EntityType& en,
                       const DomainType& x,
                       JacobianRangeType& ret);
  private:
    //- Forbidden methods
    //! Copy constructor
    AdaptiveLocalFunction(const ThisType& other);
    ThisType& operator=(const ThisType& other);

    template <class EntityType>
    void init(EntityType& en);
  private:
    //- Data members
    const DiscreteFunctionSpaceType& spc_;
    DofStorageType& dofVec_;

    mutable std::vector<RangeFieldType*> values_;

    mutable RangeType& tmp_;
    mutable JacobianRangeType& tmpGrad_;
  }; // end class AdaptiveLocalFunction

  //- Specialisations
  /* Do that later

     template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
     class AdaptiveDiscreteFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> > :
     public DiscreteFunctionDefault<
     AdaptiveDiscreteFunctionTraits<CombinedSpace<ContainedFunctionSpaceImp, N, p> >
     >,
     private AdaptiveFunctionImplementation<CombinedSpace<ContainedFunctionSpaceImp, N, p> >
     {
     private:
     typedef CombinedSpace<ContainedFunctionSpaceImp, N, p> DiscreteFunctionSpaceImp;
     typedef AdaptiveDiscretFunction<DiscreteFunctionSpaceImp> MyType;
     typedef AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp> Imp;
     typedef AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp> MyTraits;
     typedef DiscreteFunctionDefault<MyTraits> BaseType;

     public:
     //- Typedefs and enums
     typedef MyTraits Traits;
     typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;

     typedef typename Traits::LocalFunctionType LocalFunctionType;
     typedef typename Traits::DiscreteFunctionType DiscreteFunctionType;

     public:
     //- Public methods
     AdaptiveDiscreteFunction(const DiscreteFunctionSpaceType& spc,
                             std::string name = "no name") :
      BaseType(spc),
      Imp(name)
     {}

     using Imp::name;
     using Imp::dbegin;
     using Imp::dend;
     using Imp::newLocalFunction;
     using Imp::localFunction;
     using Imp::write_xdr;
     using Imp::read_xdr;
     using Imp::write_ascii;
     using Imp::read_ascii;

     //- Additional methods

     private:
     }; // end class AdaptiveDiscreteFunction (specialised for CombinedSpace)
   */
  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p, class DofManagerImp>
  class AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp >
    : public LocalFunctionDefault<
          CombinedSpace<ContainedFunctionSpaceImp, N, p>,
          AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>
          >
  {
  public:
    //- Public typedefs and enums
    typedef CombinedSpace<
        ContainedFunctionSpaceImp, N, p> DiscreteFunctionSpaceType;
    typedef AdaptiveLocalFunction<
        DiscreteFunctionSpaceType, DofManagerImp> ThisType;
    typedef AdaptiveDiscreteFunctionTraits<
        DiscreteFunctionSpaceType, DofManagerImp> Traits;

    enum { dimRange = DiscreteFunctionSpaceType::DimRange };


    //! these are the types for the derived classes
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;
    typedef typename Traits::DofType DofType;

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
                       RangeType & ret);

    template <class EntityType, class QuadratureType>
    void evaluateLocal(EntityType& en,
                       QuadratureType& quad,
                       int quadPoint,
                       RangeType & ret);

    template <class EntityType>
    void jacobianLocal(EntityType& en,
                       const DomainType& x,
                       JacobianRangeType& ret);

    template <class EntityType, class QuadratureType>
    void jacobianLocal(EntityType& en,
                       QuadratureType& quad,
                       int quadPoint,
                       JacobianRangeType& ret);

    //- Additional methods for specialisation
    void assign(const DofVectorType& dofs);

    int numberOfBaseFunctions() const;

  private:
    const DiscreteFunctionSpaceType& spc_;
    DofStorageType& dofVec_;

    mutable std::vector<RangeFieldType *> values_;

    mutable RangeType& tmp_;
    mutable JacobianRangeType& tmpGrad_;
  }; // end class AdaptiveLocalFunction (specialised for CombinedSpace)

} // end namespace Dune

#endif
