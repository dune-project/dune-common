// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ADAPTIVEFUNCTION_HH
#define DUNE_ADAPTIVEFUNCTION_HH

//- System includes
#include <rpc/xdr.h>

//- Dune includes
#include "dofmanager.hh"
#include "common/discretefunction.hh"
#include "common/localfunction.hh"
#include "discretefunction/dofarrayiterator.hh"

namespace Dune {

  // Forward declarations
  template <class DiscreteFunctionSpaceImp>
  class AdaptiveDiscreteFunction;
  template <class DiscreteFunctionSpaceIm>
  class AdaptiveLocalFunction;

  template <class DiscreteFunctionSpaceImp>
  struct AdaptiveDiscreteFunctionTraits {
    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp> DiscreteFunctionType;
    typedef AdaptiveLocalFunction<DiscreteFunctionSpaceImp> LocalFunctionType;

  };

  template <class DisreteFunctionSpaceImp>
  class AdaptiveDiscreteFunction :
    public DiscreteFunctionDefault<DiscreteFunctionSpaceImp,
        AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp> > {
  public:
    //- Public typedefs and enums
    typedef AdaptiveDiscreteFunction<DiscreteFunctionSpaceImp> TypeType;
    typedef DiscreteFunctionDefault<
        DiscreteFunctionSpaceImp,
        AdaptiveDiscreteFunctionTraits<DiscreteFunctionSpaceImp>
        > BaseType;
    typedef typename BaseType::MappingType MappingType;

    typedef DiscreteFunctionSpaceImp DiscreteFunctionSpaceType;
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;

    typedef typename DofArray<RangeFieldType>::DofIteratorType DofIteratorType;
    typedef typename DofArray<
        RangeFieldType>::ConstDofIteratorType ConstDofIteratorType;

  public:
    //- Public interface
    //- Constructor and destructors
    //! Constructor
    AdaptiveDiscreteFunction(const DiscreteFunctionSpaceType& f);

    //! Copy constructor
    AdaptiveDiscreteFunction(const DiscreteFunctionType& f);

    //! Destructor
    ~AdaptiveDiscreteFunction();

    //- Operators
    //! Addition of g to discrete function
    virtual ThisType& operator+=(const MappingType& g);

    //! subtract g from discrete function
    virtual ThisType& operator-=(const MappingType &g);

    //! multiply with scalar
    virtual ThisType& operator*=(const RangeFieldType &scalar);

    //! Division by a scalar
    virtual ThisType& operator/=(const RangeFieldType &scalar);

    // * Need to add operator + and so forth

    //- Other methods
    //! Begin iterator for contained dofs
    DofIteratorType dbegin();

    //! End iterator for contained dofs
    DofIteratorType dend();

    //! Begin const iterator for contained dofs
    ConstDofIteratorType dbegin() const;

    //! End const iterator for contained dofs
    ConstDofIteratorType dend() const;

    //! Get new local function
    //! The obtained local function is uninitialised and needs to be set to
    //! a specific entity
    LocalFunctionType newLocalFunction();

    //! Set local function to an entity
    template <class EntityType>
    void localFunction(const EntityType& en, LocalFunctionType& lf);

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool write_xdr(const std::string& filename);

    //! write data of discrete function to file filename|timestep
    //! with xdr methods
    bool read_xdr(const std::string& filename);

    //! write function data to file filename|timestep in ascii Format
    bool write_ascii(const std::string& filename);

    //! read function data from file filename|timestep in ascii Format
    bool read_ascii(const std::string& filename);

  private:
    //- Private typedefs
    typedef DofArray<RangeFieldType> DofArrayType;

  private:
    //- Private methods

  private:
    //- Data members
    DofArrayType& dofVec_;
  };

  template <class DiscreteFunctionSpaceImp>
  class AdaptiveLocalFunction<DiscreteFunctionSpaceImp> : public
                                                          LocalFunctionDefault<DiscreteFunctionSpaceImp,
                                                              AdaptiveLocalFunction<DiscreteFunctionSpaceImp> > {
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
    //- Public interface
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
  private:
    //- Private typedefs

  private:
    //- Private methods

  private:
    //- Data members

  }; // end class AdaptiveLocalFunction


} // end namespace Dune

#include "adaptivefunction.cc"

#endif
