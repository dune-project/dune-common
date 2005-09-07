// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_DISCRETEFUNCTION_HH
#define DUNE_DISCRETEFUNCTION_HH

#include <string>

#include <dune/grid/common/grid.hh>
#include <dune/common/function.hh>
#include <dune/common/functionspace.hh>
#include "discretefunctionspace.hh"
#include "dofiterator.hh"

namespace Dune {


  /** @defgroup DiscreteFunction DiscreteFunction
      @ingroup FunctionCommon
      The DiscreteFunction is responsible for the dof storage. This can be
      done in various ways an is left to the user. The user has to derive his
      own implementation from the DiscreteFunctionDefault class. If some of
      the implementations in the default class are for ineffecient for the
      dof storage in the derived class these functions can be overloaded.

      @{
   */

  //************************************************************************
  //
  //  --DiscreteFunctionInterface
  //
  //! This is the minimal interface of a discrete function which has to be
  //! implemented. It contains a local function and a dof iterator which can
  //! iterate over all dofs of one level. Via the method access the local
  //! dofs and basis functions can be accessed for a given entity.
  //!
  //************************************************************************
  template<class DiscreteFunctionTraits>
  class DiscreteFunctionInterface :
    public Function<typename DiscreteFunctionTraits::DiscreteFunctionSpaceType,
        DiscreteFunctionInterface<DiscreteFunctionTraits> >
  {
  public:
    //- Typedefs and enums

    //! types that we sometimes need outside
    typedef Function<
        typename DiscreteFunctionTraits::DiscreteFunctionSpaceType,
        DiscreteFunctionInterface<DiscreteFunctionTraits>
        > FunctionType;

    typedef typename DiscreteFunctionTraits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;

    //! Domain vector
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    //! Range vector
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;

    //! Domain field type (usually a float type)
    typedef typename DiscreteFunctionSpaceType::DomainFieldType DomainFieldType;
    //! Range field type (usually a float type)
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;

    //! Type of the underlying grid
    typedef typename DiscreteFunctionSpaceType::GridType GridType;

    //! Type of the discrete function implementation
    typedef typename DiscreteFunctionTraits::DiscreteFunctionType DiscreteFunctionType;

    //! Type of the local function implementation
    typedef typename DiscreteFunctionTraits::LocalFunctionType LocalFunctionType;

    //! Type of the dof iterator used in the discrete function implementation
    typedef typename DiscreteFunctionTraits::DofIteratorType DofIteratorType;

    //! Type of the constantdof iterator used in the discrete function implementation
    typedef typename DiscreteFunctionTraits::ConstDofIteratorType ConstDofIteratorType;

  public:
    //- Public Methods

    //! Constructor
    //! Needs to be called in derived classes
    DiscreteFunctionInterface (const DiscreteFunctionSpaceType& f)
      : FunctionType ( f ) {}

    //! Name of the discrete function
    std::string name() const {
      return asImp().name();
    }

    //! the implementation of an iterator to iterate efficient
    //! over all dofs of a discrete function
    DofIteratorType dbegin ()
    {
      return asImp().dbegin ();
    }

    //! the implementation of an iterator to iterate efficient
    //! over all dofs of a discrete function
    DofIteratorType dend ()
    {
      return asImp().dend ();
    }


    //! const version of dbegin
    ConstDofIteratorType dbegin () const
    {
      return asImp().dbegin ();
    };

    //! const version of dend
    ConstDofIteratorType dend () const
    {
      return asImp().dend ();
    };

    //! Get new local function
    //! The local function is uninitialised and needs to be set to a specific
    //! entity
    LocalFunctionType newLocalFunction() {
      return asImp().newLocalFunction();
    }

    //! Set local function to an entity
    template<class EntityType>
    void localFunction(const EntityType& en, LocalFunctionType& lf) {
      asImp().localFunction(en, lf);
    }

  private:
    // Barton-Nackman trick
    DiscreteFunctionType& asImp()
    {
      return static_cast<DiscreteFunctionType&>(*this);
    }

    //! const version of asImp
    const DiscreteFunctionType &asImp() const
    {
      return static_cast<const DiscreteFunctionType&>(*this);
    }
  };

  //*************************************************************************
  //
  //  --DiscreteFunctionDefault
  //
  //! Default implementation of the discrete function. This class provides
  //! is responsible for the dof storage. Different implementations of the
  //! discrete function use different dof storage.
  //! The default implementation provides +=, -= ans so on operators and
  //! a DofIterator access, which can run over all dofs in an efficient way.
  //! Furthermore with an entity you can access an local function to evaluate
  //! the discrete function by multiplying the dofs and the basefunctions.
  //!
  //*************************************************************************
  template <class DiscreteFunctionTraits>
  class DiscreteFunctionDefault :
    public DiscreteFunctionInterface<DiscreteFunctionTraits>
  {

    typedef DiscreteFunctionInterface<
        DiscreteFunctionTraits
        > DiscreteFunctionInterfaceType;

    typedef DiscreteFunctionDefault<
        DiscreteFunctionTraits
        > DiscreteFunctionDefaultType;

    enum { myId_ = 0 };

  public:
    typedef typename DiscreteFunctionTraits::DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    //! Type of domain vector
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    //! Type of range vector
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;

    //! Type of domain field (usually a float type)
    typedef typename DiscreteFunctionSpaceType::DomainFieldType DomainFieldType;
    //! Type of range field (usually a float type)
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;

    typedef Mapping<DomainFieldType, RangeFieldType,
        DomainType, RangeType> MappingType;

    //! Type of the discrete function (Barton-Nackman parameter)
    typedef typename DiscreteFunctionTraits::DiscreteFunctionType DiscreteFunctionType;

    //! Type of the local function
    typedef typename DiscreteFunctionTraits::LocalFunctionType LocalFunctionType;
    //! Type of the dof iterator
    typedef typename DiscreteFunctionTraits::DofIteratorType DofIteratorType;

    //! Type of the const dof iterator
    typedef typename DiscreteFunctionTraits::ConstDofIteratorType ConstDofIteratorType;

  public:
    //- Methods
    //! pass the function space to the interface class
    DiscreteFunctionDefault (const DiscreteFunctionSpaceType & f ) :
      DiscreteFunctionInterfaceType ( f ) {}

    //! Continuous data
    bool continuous() const {
      return this->functionSpace_.continuous();
    }

    //! Set all elements to zero
    void clear();

    //! daxpy operation
    void addScaled(const DiscreteFunctionType& g, const RangeFieldType& c);

    //! Evaluate a scalar product of the dofs of two DiscreteFunctions
    //! on the top level of the underlying grid
    RangeFieldType scalarProductDofs(const DiscreteFunctionType& g) const;

    //! Assignment
    virtual DiscreteFunctionDefaultType& assign(const MappingType& g);

    //! Addition of g to discrete function
    virtual DiscreteFunctionDefaultType& operator+=(const MappingType& g);

    //! subtract g from discrete function
    virtual DiscreteFunctionDefaultType& operator -= (const MappingType &g);

    //! multiply with scalar
    virtual DiscreteFunctionDefaultType&
    operator *=(const RangeFieldType &scalar);

    //! Division by a scalar
    virtual DiscreteFunctionDefaultType& operator /=
      (const RangeFieldType &scalar);

    //! add scalar * g to discrete function
    DiscreteFunctionType&
    add(const DiscreteFunctionType &g , RangeFieldType scalar );

    //! evaluate Function (which just dies because there is no meaningful implementation)
    void eval(const DomainType & arg, RangeType & dest) const {
      // Die a horrible death! Never call that one...
      assert(false);
    }

    //! evaluate function and derivatives (just dies)
    template <int derivation>
    void evaluate  ( const FieldVector<deriType, derivation> &diffVariable,
                     const DomainType& arg, RangeType & dest) const {
      // Die a horrible death! Never call that one...
      assert(false);
    }

  private:
    // Barton-Nackman trick
    DiscreteFunctionType &asImp()
    {
      return static_cast<DiscreteFunctionType&>(*this);
    }
    const DiscreteFunctionType &asImp() const
    {
      return static_cast<const DiscreteFunctionType&>(*this);
    }

  }; // end class DiscreteFunctionDefault

  /** @} end documentation group */

} // end namespace Dune

#include "discretefunction.cc"

#endif
