// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFITERATOR_HH__
#define __DUNE_DOFITERATOR_HH__

namespace Dune {

  //************************************************************************
  //
  //  --DofIteratorInterface
  //
  //! Interface for GlobalDofIterator
  //! The functionality defined here must be implemented by the user, i.e
  //! in his GlobalDofIteratorImp Class.
  //
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

    //! go to next i steps
    DofIteratorType& operator++ (int i)
    {
      return asImp().operator ++ (i);
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
  //  --GlobalDofIteratorDefault
  //
  //! Default implementation for some extra functionality for
  //! GlobalDofIterator. At the moment there is no extra functionality but
  //! perhaps later.
  //
  //************************************************************************
  template <class DofImp, class DofIteratorImp>
  class DofIteratorDefault
    : public DofIteratorInterface < DofImp , DofIteratorImp >
  {
  public:
    //! random access operator, for efficient implementation overload in the
    //! implementation class DofIteratorImp
    DofImp& operator [] (int i)
    {
      asImp().reset();
      asImp().operator ++ (i);
      return asImp().operator *();
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
  }; // end class DofIteratorDefault


#if 0
  //**************************************************************************
  //
  //  --LocalDofIteratorArray
  //
  //! The DofStorage class holds the memory with the dofs of a grid function.
  //! The DofStorage class must provide a iterator for global walk through
  //! all dofs and a local walk through all local dofs of a element.
  //! Furthermore there should be a random access to global dof number n and
  //! to local dof number l. Perhaps there should be a mapToGlobal method?
  //
  //**************************************************************************
  template <class DofEntityType >
  class LocalDofIteratorImpDefault : public
                                     LocalDofIteratorDefault < typename DofEntityType::Traits::DofImp ,
                                         LocalDofIteratorImpDefault < DofEntityType >  >
  {
    // just for readability
    typedef LocalDofIteratorImpDefault < DofEntityType >
    LocalDofIteratorImpDefaultType;
    //! the dof type
    typedef typename Traits::DofImp DofImp;

  public:
    //! Constructor getting the array that holds the dofs
    LocalDofIteratorImpDefault ( DofEntityType & e , int count )
      : dofEntity_ ( e ) , count_ ( count ) {};

    //! return local dof
    DofImp& operator *()
    {
      return dofEntity_ [ count_ ];
    };

    //! go next dof
    LocalDofIteratorImp& operator++ ()
    {
      count_++;
      return (*this);
    };

    //! go next i steps
    LocalDofIteratorImp& operator++ (int i)
    {
      count_ += i;
      return (*this);
    };

    //! comparison
    bool operator == (const LocalDofIteratorImpDefaultType & I )
    {
      return count_ == I.count_;
    }

    //! comparison
    bool operator != (const LocalDofIteratorImpDefaultType & I )
    {
      return count_ != I.count_;
    }

    //! return local dof number
    int index ()
    {
      return count_;
    };

  private:
    //! local dof number
    int count_;

    //! array storing the dofs
    DofEntityType & dofEntity_;
  };
#endif

  //**************************************************************************
  //
  //  --DofIteratorDefaultImp
  //
  //! The GlobalDofIteratorArray can esayly walk through global dof, because
  //! all dofs are stored in an array. The GlobalDofIteratorArray is more or
  //! less an array iterator.
  //
  //**************************************************************************
  template < class LocalFunctionIteratorImp >
  class DofIteratorDefaultImp : public
                                DofIteratorDefault < typename LocalFunctionIteratorImp::RangeField ,
                                    DofIteratorDefaultImp < LocalFunctionIteratorImp >  >
  {
    typedef DofIteratorDefaultImp < LocalFunctionIteratorImp >
    DofIteratorDefaultImpType;

    typedef typename LocalFunctionIteratorImp::RangeField RangeField;


  public:
    enum { AmITheDefaultGlobalDofIterator = true };

    DofIteratorDefaultImp ( const LocalFunctionIteratorImp & lf )
      :  lfIt_ ( lf ) {};

    RangeField & operator *()
    {
      return (*lfIt_)[0];
    };

    DofIteratorDefaultImpType & operator++ ()
    {
      ++lfIt_;
      return (*this);
    };

    DofIteratorDefaultImpType & operator++ (int i)
    {
      ++lfIt_(i);
      return (*this);
    };

    bool operator == (const DofIteratorDefaultImpType & I )
    {
      return lfIt_  == I.lfIt_;
    }

    bool operator != (const DofIteratorDefaultImpType & I )
    {
      return lfIt_ != I.lfIt_;
    }

  private:
    //! the LocalFunctionIterator to iterate over all LocalFunctions and
    //! then over all local Dofs
    LocalFunctionIteratorImp lfIt_;
  };

} // end namespace Dune

#endif
