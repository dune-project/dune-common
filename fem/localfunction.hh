// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LOCALFUNCTION_HH__
#define __DUNE_LOCALFUNCTION_HH__

namespace Dune {


  /** @defgroup LocalFunction The LocalFunction Interface for Dof access

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
    typedef typename DiscreteFunctionSpaceType::RangeField RangeField;
    typedef typename DiscreteFunctionSpaceType::Domain Domain;
    typedef typename DiscreteFunctionSpaceType::Range Range;

    //! access to dof number num, all dofs of the local function
    RangeField& operator [] (int num)
    {
      return asImp().operator [] ( num );
    }

    //! return the number of local dof of this local function
    int numberOfDofs ()
    {
      return asImp().numberOfDofs ();
    };

    //! evaluate the local function on x and return ret
    template <class EntityType>
    void evaluate (EntityType &en, const Domain & x, Range & ret)
    {
      asImp().evaluate(en,x,ret);
    }

  private:
    //! Barton-Nackman trick
    LocalFunctionImp & asImp()
    {
      return static_cast<LocalFunctionImp&>(*this);
    }
    const LocalFunctionImp &asImp() const
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
    // notin'
  }; // end LocalFunctionDefault

} // end namespace Dune

#endif
