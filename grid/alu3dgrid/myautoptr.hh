// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MYAUTOPTR_HH
#define DUNE_MYAUTOPTR_HH

namespace ALUGridSpace {

  /** \brief An auto pointer class */
  template <class Pointer>
  class AutoPointer
  {
    //! Pointer to Object
    Pointer * ptr_;

    //! number of copies that exist from this Object
    mutable int *refCount_;

    //! true if we own the pointer
    mutable bool owner_;

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
        // now we own the pointer
        owner_ = true;
        copy.owner_ = false;
      }
    }

    //! initialize the member variables
    AutoPointer() : ptr_ (0) , refCount_ (0) , owner_(false) {}

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
        owner_ = true;
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
          if(ptr_) delete ptr_;
          if(refCount_) delete refCount_;
          owner_ = false;
        }
      }
    }

    //! return object reference
    Pointer & operator * () const
    {
      assert(ptr_ != 0);
      assert(owner_);
      return *ptr_;
    }

    //! return object pointer
    Pointer * operator -> () const
    {
      assert(ptr_ != 0);
      assert(owner_);
      return ptr_;
    }

  private:
    //! if copy is made than one more Reference exists
    AutoPointer<Pointer> & operator = (const AutoPointer<Pointer> & copy)
    {
      assert(false);
      return (*this);
    }
  }; // end class AutoPointer

} // end namespace ALU3dGridSpace
#endif
