// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSGRID_MYAUTOPTR_HH__
#define __DUNE_BSGRID_MYAUTOPTR_HH__

/** \brief An auto pointer class */
template <class Pointer>
class AutoPointer
{
  //! Pointer to Object
  Pointer * ptr_;

  //! number of copies that exist from this Object
  mutable int *refCount_;

public:
  //! if a copy is made, the refcount is increased
  inline AutoPointer(const AutoPointer<Pointer> & copy)
  {
    ptr_ = 0;
    refCount_ = 0;
    if(copy.ptr_)
    {
      ptr_ = copy.ptr_;
      refCount_ = copy.refCount_;
      (*refCount_)++;
    }
  }

  //! initialize the member variables
  AutoPointer() : ptr_ (0) , refCount_ (0) {}

  // store object pointer and create refCount
  void store (Pointer * ptr)
  {
    assert(ptr_ == 0);
    ptr_ = ptr;
    if(ptr_)
    {
      int * tmp = new int;
      refCount_ = tmp;
      (*refCount_) = 1;
    }
  }

  //! set Stack free, if no more refences exist
  ~AutoPointer()
  {
    if(refCount_ && ptr_)
    {
      (*refCount_)--;
      if((*refCount_) <= 0)
      {
        if(ptr_) delete  ptr_;
        if(refCount_) delete refCount_;
      }
    }
  }

  //! return object reference
  Pointer & operator * () const
  {
    assert( ptr_ != 0);
    return *ptr_;
  }

  //! return object pointer
  Pointer * operator -> () const
  {
    assert( ptr_ != 0);
    return ptr_;
  }

private:
  //! if copy is made than one more Reference exists
  AutoPointer<Pointer> & operator = (const AutoPointer<Pointer> & copy)
  {
    assert(false);
    return (*this);
  }
};



#endif
