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
    }

    //! return reference to dof
    const DofImp& operator *() const
    {
      return asImp().operator *();
    }

    //! return global dof number of dof
    int index () const {  return asImp().index(); }

    //! go to next dof
    DofIteratorType& operator++ ()
    {
      return asImp().operator ++ ();
    }

    //! compare with other GlobalDofIterators
    bool operator == (const DofIteratorType& I) const
    {
      return asImp().operator == (I);
    }

    //! compare with other GlobalDofIterators
    bool operator != (const DofIteratorType& I) const
    {
      return asImp().operator != (I);
    }

    //! set the iterator to begin status
    void reset ()
    {
      asImp().reset();
    }

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
      for (int i=0; i<n; i++) ++asImp();
      return asImp().operator *();
    };

    const DofImp& operator [] (int n) const
    {
      asImp().reset();
      for (int i=0; i<n; i++) ++asImp();
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


  //****************************************************************
  //
  /*! \brief
     ConstDofIteratorDefault make a const interator out of a
     dof iterator. This version works for all dof iterators.
   */
  //****************************************************************
  template <class DofIteratorImp>
  class ConstDofIteratorDefault
    : public DofIteratorDefault< typename DofIteratorImp::DofType, DofIteratorImp>
  {
    typedef ConstDofIteratorDefault<DofIteratorImp> MyType;
  public:
    typedef DofIteratorImp DofIteratorType;
    typedef typename DofIteratorType::DofType DofType;

    //! Constructor
    ConstDofIteratorDefault( DofIteratorImp & it ) : it_(it) {}

    //! random access operator, for efficient implementation overload in the
    //! implementation class DofIteratorImp
    const DofType& operator [] (int n) const
    {
      return it_[n];
    }

    const DofType& operator *() const
    {
      return (*it_);
    }

    //! return global dof number of dof
    int index () const {  return it_.index();   }

    //! go to next dof
    MyType & operator++ ()
    {
      ++it_;
      return (*this);
    }

    //! compare with other GlobalDofIterators
    bool operator == (const MyType& I) const
    {
      return it_  == I.it_;
    }

    //! compare with other GlobalDofIterators
    bool operator != (const MyType& I) const
    {
      return it_ != I.it_;
    }

    //! set the iterator to begin status
    void reset ()
    {
      it_.reset();
    }

  private:
    DofIteratorImp it_;
  }; // end class DofIteratorDefault


  /** @} end documentation group */

} // end namespace Dune

#endif
