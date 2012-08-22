// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SMARTPTR_HH
#define DUNE_SMARTPTR_HH

#include <dune/common/nullptr.hh>

namespace Dune
{

  // ReferenceCountable
  // ------------------

  struct ReferenceCountable
  {
    ReferenceCountable ()
      : refCount_( 0 )
    {}

    ReferenceCountable ( const ReferenceCountable &other )
      : refCount_( 0 )
    {}

    const ReferenceCountable &operator= ( const ReferenceCountable &other )
    {
      return *this;
    }

    void addReference ()
    {
      ++refCount_;
    }

    bool removeReference ()
    {
      return (--refCount_ == 0);
    }

  private:
    unsigned int refCount_;
  };



  // SmartPtr
  // --------

  template< class T >
  struct SmartPtr
  {
    explicit SmartPtr ( T *ptr = nullptr )
      : ptr_( ptr )
    {
      if( ptr_ )
        ptr_->addReference();
    }

    SmartPtr ( const SmartPtr &other )
      : ptr_( other.ptr_ )
    {
      if( ptr_ )
        ptr_->addReference();
    }

    ~SmartPtr ()
    {
      if( ptr_ && ptr_->removeReference() )
        delete ptr_;
    }

    const SmartPtr &operator= ( T *ptr )
    {
      if( ptr )
        ptr->addReference();
      if( ptr_ && ptr_->removeReference() )
        delete ptr_;
      ptr_ = ptr;
      return *this;
    }

    const SmartPtr &operator= ( const SmartPtr &other )
    {
      if( other.ptr_ )
        other.ptr_->addReference();
      if( ptr_ && ptr_->removeReference() )
        delete ptr_;
      ptr_ = other.ptr_;
      return *this;
    }

    operator bool () const { return ptr_; }
    operator T * () const { return ptr_; }

    T &operator* () const { return *ptr_; }
    T *operator-> () const { return ptr_; }


    bool operator== ( const SmartPtr &other ) const { return (ptr_ == other.ptr_); }
    bool operator!= ( const SmartPtr &other ) const { return (ptr_ != other.ptr_); }

  private:
    T *ptr_;
  };

} // namespace Dune

#endif // #ifndef DUNE_SMARTPTR_HH
