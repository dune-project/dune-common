// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LOCALFUNCTION_HH__
#define __DUNE_LOCALFUNCTION_HH__

namespace Dune {


  /** @defgroup LocalFunction LocalFunction
     @ingroup DiscreteFunction

     On every element from a discrete function the local funtion can be accessed.
     With the local function one has access to the dof and on the other hand to
     the base function set of this actual element. Therefore this is called a
     local function.

     @{
   */


  //****************************************************************************
  //
  //  --LocalFunctionInterface
  //
  //! The LocalFunctionInterface is the Interface to local function which
  //! form the discrete Function
  //
  //****************************************************************************
  template < class DiscreteFunctionSpaceType, class LocalFunctionImp >
  class LocalFunctionInterface
  {
  public:
    //! this are the types for the derived classes
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;

    //! access to dof number num, all dofs of the local function
    RangeFieldType& operator [] (int num)
    {
      return asImp().operator [] ( num );
    }

    //! access to dof number num, all dofs of the local function
    const RangeFieldType& operator [] (int num) const
    {
      return asImp().operator [] ( num );
    }

    //! return the number of local dof of this local function
    int numberOfDofs () const
    {
      return asImp().numberOfDofs ();
    };

    //! evaluate the local function on x and return ret
    template <class EntityType>
    void evaluateLocal(EntityType& en,
                       const DomainType& x,
                       RangeType & ret)
    {
      asImp().evaluateLocal(en,x,ret);
    }

    template <class EntityType>
    void jacobianLocal(EntityType& en,
                       const DomainType& x,
                       JacobianRangeType& ret)
    {
      asImp().jacobianLocal(en,x,ret);
    }

  private:
    //! Barton-Nackman trick
    LocalFunctionImp& asImp()
    {
      return static_cast<LocalFunctionImp&>(*this);
    }

    const LocalFunctionImp& asImp() const
    {
      return static_cast<const LocalFunctionImp&>(*this);
    }
  }; // end LocalFunctionInterface

  //************************************************************************
  //
  //  --LocalFunctionDefault
  //
  //! The Interface to the dune programmer, use this class to derive
  //! the own implementation. But only the methods declared in the interface
  //! class must be implemented.
  //!
  //************************************************************************
  template < class DiscreteFunctionSpaceType, class LocalFunctionImp >
  class LocalFunctionDefault : public LocalFunctionInterface <
                                   DiscreteFunctionSpaceType , LocalFunctionImp >
  {
  public:
    //! these are the types for the derived classes
    typedef typename DiscreteFunctionSpaceType::RangeFieldType RangeFieldType;
    typedef typename DiscreteFunctionSpaceType::DomainType DomainType;
    typedef typename DiscreteFunctionSpaceType::RangeType RangeType;
    typedef typename DiscreteFunctionSpaceType::JacobianRangeType JacobianRangeType;

  public:
    //! Constructor
    LocalFunctionDefault() : xLoc_(0.0) {}

    //! evaluate the local function on x and return ret
    template <class EntityType>
    void evaluate(EntityType& en,
                  const DomainType& x,
                  RangeType & ret)
    {
      ret = 0.0;
      xLoc_ = en.geometry().local(x);
      evaluateLocal(en,xLoc_,ret);
    }

    template <class EntityType>
    void jacobian(EntityType& en,
                  const DomainType& x,
                  JacobianRangeType& ret)
    {
      ret = 0.0;
      xLoc_ = en.geometry().local(x);
      jacobian(en,xLoc_,ret);
    }

  private:
    mutable DomainType xLoc_;

  }; // end LocalFunctionDefault

} // end namespace Dune

#endif
