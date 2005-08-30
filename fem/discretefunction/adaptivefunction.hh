// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ADAPTIVEFUNCTION_HH
#define DUNE_ADAPTIVEFUNCTION_HH

//- System includes
#include <string>

//- Dune includes
#include <dune/fem/common/discretefunction.hh>
#include <dune/fem/common/localfunction.hh>

//- Local includes
#include "adaptiveimp.hh"

namespace Dune {

  //- Forward declarations
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveDiscreteFunction;
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveLocalFunction;

  //- Class definitions
  template <class DiscreteFunctionSpaceImp>
  struct AdaptiveDiscreteFunctionTraits {
    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp> DiscreteFunctionType;
    typedef AdaptiveLocalFunction<DiscreteFunctionSpaceImp> LocalFunctionType;

  }; // end class AdaptiveDiscreteFunctionTraits

  template <class DiscreteFunctionSpaceImp>
  class AdaptiveDiscreteFunction :
    public DiscreteFunctionDefault<
        AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp> >,
    private AdaptiveFunctionImplementation<DiscreteFunctionSpaceImp>
  {
  private:
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
  }; // end class AdaptiveDiscreteFunction

  // Note: could use Traits class for Barton-Nackman instead
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveLocalFunction<DiscreteFunctionSpaceImp> :
    public LocalFunctionDefault<
        DiscreteFunctionSpaceImp,
        AdaptiveLocalFunction<DiscreteFunctionSpaceImp> >
  {
    friend class AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp>;
  public:
    //- Public typedefs and enums
    typedef AdaptiveLocalFunction<DiscreteFunctionSpaceImp> ThisType;
    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp> DiscreteFunctionSpaceType;

    //! these are the types for the derived classes
    typedef typename DiscreteFunctionSpaceType::FunctionSpaceType FunctionSpaceType;
    typedef typename DiscreteFunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;
    typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::Domain DomainType;
    typedef typename DiscreteFunctionSpaceType::Range RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRange JacobianRangeType;
    typedef RangeFieldType DofType;
    typedef const DofType ConstDofType;
    typedef typename DiscreteFunctionSpaceType::DofStorageType DofStorageType;

    enum { dimRange = FunctionSpaceType::dimRange };

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
    ConstDofType& operator[] (int num) const;

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

    //- Private methods
    DofType& dofAt(int num);
    const DofType& dofAt(int num);

    template <class EntityType>
    void init(EntityType& en);
  private:
    //- Data members
    const DiscreteFunctionSpaceType& spc_;
    DofStorageType& dofVec_;

    mutable Array < RangeFieldType * > values_;
    mutable bool init_;

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

     template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
     class AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> > :
     public LocalFunctionDefault<
     CombinedSpace<ContainedFunctionSpaceImp, N, p>,
     AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >
     >
     {
      public:
     //- Public typedefs and enums
     typedef AdaptiveLocalFunction<DiscreteFunctionSpaceImp> ThisType;
     typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;

     //! these are the types for the derived classes
     typedef typename DiscreteFunctionSpaceType::RangeField RangeFieldType;
     typedef typename DiscreteFunctionSpaceType::Domain DomainType;
     typedef typename DiscreteFunctionSpaceType::Range RangeType;
     typedef typename DiscreteFunctionSpaceType::JacobianRange JacobianRangeType;
     typedef RangeFieldType DofType;
     typedef const DofType ConstDofType;
     public:
     //- Public methods
     //- Constructors and destructors

     //! Constructor
     // * More to come
     AdaptiveLocalFunction();

     //! Copy constructor
     AdaptiveLocalFunction(const ThisType& other);

     //! Destructor
     ~AdaptiveLocalFunction();

     //- Operators
     //! Random access operator
     DofType& operator[] (int num);

     //! Cosnt random access operator
     ConstDofType& operator[] (int num) const;

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

     //- Additional methods for specialisation
     void assign(const DofVectorType& dofs);

     int numberOfBaseFunctions() const;
     private:
     }; // end class AdaptiveLocalFunction (specialised for CombinedSpace)
   */
} // end namespace Dune

#endif
