// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFITERATOR_HH__
#define __DUNE_DOFITERATOR_HH__

namespace Dune {

  /** @defgroup DofIterator The dof iterator interface
     \ingroup DiscreteFunction
     The base functions are essential to describe a numericla solution.
     Here the interface of base functions and the corresponding base
     function set is presented. The user always works with the base function
     set, where all diffrent base functions for on element type are known.

     @{
   */


  //************************************************************************
  //
  //  --DofIteratorInterface
  //
  //! Interface for the DofIterator. All methods declared in this interface
  //! class must be implemented by the implementation class.
  //! The dof iterator is the interface for efficient walk trough the dofs of
  //! an discrete function.
  //!
  //***********************************************************************
  template <class DofImp, class DofIteratorImp>
  class DofIteratorInterface
  {
  public:
    typedef DofIteratorImp DofIteratorType;

    //! return reference to dof
    DofImp& operator *()
    {
      return asImp().operator *();
    };

    //! return global dof number of dof
    int index ()  {  return asImp().index();   };

    //! go to next dof
    DofIteratorType& operator++ ()
    {
      return asImp().operator ++ ();
    };

    //! compare with other GlobalDofIterators
    bool operator == (const DofIteratorType& I)
    {
      return asImp().operator == (I);
    };

    //! compare with other GlobalDofIterators
    bool operator != (const DofIteratorType& I)
    {
      return asImp().operator != (I);
    };

    //! set the iterator to begin status
    void reset ()
    {
      asImp().reset();
    };

  private:
    //! Barton-Nackman trick
    DofIteratorType &asImp()
    {
      return static_cast<DofIteratorType&>(*this);
    };
    const DofIteratorType &asImp() const
    {
      return static_cast<const DofIteratorType&>(*this);
    };
  }; // end DofIteratorInterface


  //************************************************************************
  //
  //  --DofIteratorDefault
  //
  //! Default implementation for some extra functionality for
  //! DofIterator. This class provides an default implementation for
  //! random access of the dofs.
  //!
  //************************************************************************
  template <class DofImp, class DofIteratorImp>
  class DofIteratorDefault
    : public DofIteratorInterface < DofImp , DofIteratorImp >
  {
    typedef DofIteratorImp DofIteratorType;
  public:
    //! random access operator, for efficient implementation overload in the
    //! implementation class DofIteratorImp
    DofImp& operator [] (int n)
    {
      asImp().reset();
      for (int i=0; i<n; i++)
        ++asImp();
      return asImp().operator *();
    };

  private:
    // Barton-Nackman trick
    DofIteratorType &asImp()
    {
      return static_cast<DofIteratorType&>(*this);
    };
    const DofIteratorType &asImp() const
    {
      return static_cast<const DofIteratorType&>(*this);
    };
  }; // end class DofIteratorDefault


  /** @} end documentation group */

} // end namespace Dune

#endif
