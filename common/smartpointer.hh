// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_SMARTPOINTER_HH
#define DUNE_SMARTPOINTER_HH
#include <iostream>

/**
 * @file
 * @brief This file implements the class SmartPointer which is a reference counting
 * pointer.
 * @author Markus Blatt
 */
namespace Dune
{
  /** @addtogroup Common
   *
   * @{
   */
  /**
   * @brief A reference counting smart pointer.
   *
   * It is designed such that it is usable within a std::vector.
   * The contained object is destroyed only if there are no more
   * references to it.
   */
  template<class T>
  class SmartPointer
  {
  public:
    /**
     * @brief The data type we are a pointer for.
     *
     * This has to have a parameterless constructor.
     */
    typedef T MemberType;

    /**
     * @brief Constructs a new smart pointer and allocates the referenced Object.
     */
    inline SmartPointer();

    /**
     * @brief Constructs a new smart pointer from a preallocated Object.
     *
     * note: the object must be allocated on the heap and after handing the pointer to
     * SmartPointer the ownership of the pointer is also handed to the SmartPointer.
     */
    inline SmartPointer(T * pointer);

    /**
     * @brief Copy constructor.
     * @param pointer The object to copy.
     */
    inline SmartPointer(const SmartPointer<T>& pointer);

    /**
     * @brief Destructor.
     */
    inline ~SmartPointer();

    /** \brief Assignment operator */
    inline SmartPointer& operator=(const SmartPointer<T>& pointer);

    /** \brief Dereference as object */
    inline MemberType& operator*();

    /** \brief Dereference as pointer */
    inline MemberType* operator->();

    /** \brief Dereference as const object */
    inline const MemberType& operator*() const;

    /** \brief Dereference as const pointer */
    inline const MemberType* operator->() const;

    /**
     * @brief Deallocates the references object if no other
     * pointers reference it.
     */
    inline void deallocate();
    int count() const;
  private:
    /** @brief The object we reference. */
    class PointerRep
    {
      friend class SmartPointer<MemberType>;
      /** @brief The number of references. */
      int count_;
      /** @brief The representative. */
      MemberType * rep_;
      /** @brief Default Constructor. */
      PointerRep() : count_(1), rep_(new MemberType) {}
      /** @brief Constructor from existing Pointer. */
      PointerRep(MemberType * p) : count_(1), rep_(p) {}
    } *rep_;
  };

  template<class T>
  inline SmartPointer<T>::SmartPointer(T * p)
  {
    rep_ = new PointerRep(p);
  }

  template<class T>
  inline SmartPointer<T>::SmartPointer()
  {
    rep_ = new PointerRep;
  }

  template<class T>
  inline SmartPointer<T>::SmartPointer(const SmartPointer<T>& other) : rep_(other.rep_)
  {
    ++(rep_->count_);
  }

  template<class T>
  inline SmartPointer<T>& SmartPointer<T>::operator=(const SmartPointer<T>& other)
  {
    (other.rep_->count_)++;
    if(rep_!=0 && --(rep_->count_)<=0) delete rep_;
    rep_ = other.rep_;
    return *this;
  }

  template<class T>
  inline SmartPointer<T>::~SmartPointer()
  {
    if(rep_!=0 && --(rep_->count_)==0) {
      delete rep_;
      rep_=0;
    }
  }

  template<class T>
  inline T& SmartPointer<T>::operator*()
  {
    return *(rep_->rep_);
  }

  template<class T>
  inline T *SmartPointer<T>::operator->()
  {
    return rep_->rep_;
  }

  template<class T>
  inline const T& SmartPointer<T>::operator*() const
  {
    return *(rep_->rep_);
  }

  template<class T>
  inline const T *SmartPointer<T>::operator->() const
  {
    return rep_->rep_;
  }

  template<class T>
  inline int SmartPointer<T>::count() const
  {
    return rep_->count_;
  }

  template<class T>
  inline void SmartPointer<T>::deallocate()
  {
    assert(rep_!=0 && rep_->count_==1);
    delete rep_;
    rep_=0;
  }
  /** @} */
}
#endif
