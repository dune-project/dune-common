// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: smartpointer.hh 5504 2009-04-08 13:35:31Z christi $

#ifndef DUNE_SHARED_PTR_HH
#define DUNE_SHARED_PTR_HH

#if defined SHARED_PTR_HEADER
# include SHARED_PTR_HEADER
#endif
#if defined HAVE_BOOST_SHARED_PTR_HPP
#if defined HAVE_BOOST_MAKE_SHARED_HPP
# include <boost/make_shared.hpp>
#endif
#endif

#include <dune/common/nullptr.hh>
#include <dune/common/typetraits.hh>
/**
 * @file
 * @brief This file implements the class shared_ptr (a reference counting
 * pointer), for those systems that don't have it in the standard library.
 * @author Markus Blatt
 */
namespace Dune
{
  // A shared_ptr implementation has been found if SHARED_PTR_NAMESPACE is set at all
#ifdef SHARED_PTR_NAMESPACE
  using SHARED_PTR_NAMESPACE :: shared_ptr;
#else

  /** @addtogroup Common
   *
   * @{
   */

  /** @brief The object we reference. */
  class SharedCount
  {
    template<class T1>
    friend class shared_ptr;
  protected:
    /** @brief The number of references. */
    int count_;
    /** @brief Constructor from existing Pointer. */
    SharedCount() : count_(1) {}
    /** @brief Copy constructor with type conversion. */
    SharedCount(const SharedCount& rep)
      : count_(rep.count_) {}

    /** @brief Destructor, deletes element_type* rep_. */
    virtual ~SharedCount() {};

  };
  /**
   * @brief A reference counting smart pointer.
   *
   * It is designed such that it is usable within a std::vector.
   * The contained object is destroyed only if there are no more
   * references to it.
   */
  template<class T>
  class shared_ptr
  {
    template<class T1> friend class shared_ptr;

  public:
    /**
     * @brief The data type we are a pointer for.
     *
     * This has to have a parameterless constructor.
     */
    typedef T element_type;

    /**
     * @brief Constructs a new smart pointer and allocates the referenced Object.
     */
    inline shared_ptr();

    inline shared_ptr(nullptr_t null);

    /**
     * @brief Constructs a new smart pointer from a preallocated Object.
     *
     * \param pointer Raw pointer to the shared data
     *
     * note: the object must be allocated on the heap and after handing the pointer to
     * shared_ptr the ownership of the pointer is also handed to the shared_ptr.
     */
    template<class T1>
    inline shared_ptr(T1 * pointer);


    /**
     * @brief Constructs a new smart pointer from a preallocated Object.
     *
     * \tparam Deleter This class must by copyconstructable, the copy constructor must not throw an exception
     * and it must implement void operator() (T*) const
     *
     * \param pointer Raw pointer to the shared data
     * \param deleter A copy of this deleter is stored
     *
     * note: the object must be allocated on the heap and after handing the pointer to
     * shared_ptr the ownership of the pointer is also handed to the shared_ptr.
     */
    template<class T1, class Deleter>
    inline shared_ptr(T1 * pointer, Deleter deleter);

    /**
     * @brief Copy constructor.
     * @param pointer The object to copy.
     */
    template<class T1>
    inline shared_ptr(const shared_ptr<T1>& pointer);

    /**
     * @brief Copy constructor.
     * @param pointer The object to copy.
     */
    inline shared_ptr(const shared_ptr& pointer);

    /**
     * @brief Destructor.
     */
    inline ~shared_ptr();

    /** \brief Assignment operator */
    template<class T1>
    inline shared_ptr& operator=(const shared_ptr<T1>& pointer);

    /** \brief Assignment operator */
    inline shared_ptr& operator=(const shared_ptr& pointer);

    /** \brief Dereference as object */
    inline element_type& operator*();

    /** \brief Dereference as pointer */
    inline element_type* operator->();

    /** \brief Dereference as const object */
    inline const element_type& operator*() const;

    /** \brief Dereference as const pointer */
    inline const element_type* operator->() const;

    /** \brief Access to the raw pointer, if you really want it */
    element_type* get() const {
      return rep_;
    }

    /** \brief Checks if shared_ptr manages an object, i.e. whether get() != 0. */
    operator bool() const {
      return count_ != 0 && rep_ != 0;
    }

    /** \brief Swap content of this shared_ptr and another */
    inline void swap(shared_ptr& other);

    /** \brief Decrease the reference count by one and free the memory if the
        reference count has reached 0
     */
    inline void reset();

    /** \brief Detach shared pointer and set it anew for the given pointer */
    template<class T1>
    inline void reset(T1* pointer);

    //** \brief Same as shared_ptr(pointer,deleter).swap(*this)
    template<class T1, class Deleter>
    inline void reset(T1* pointer, Deleter deleter);

    /** \brief The number of shared_ptrs pointing to the object we point to */
    int use_count() const;

  private:
    /** \brief Assignment operator */
    template<class T1>
    inline shared_ptr& assign(const shared_ptr<T1>& pointer);
    /** @brief Adds call to deleter to SharedCount. */
    template<class Deleter>
    class SharedCountImpl :
      public SharedCount
    {
      template<class T1>
      friend class shared_ptr;
      /** @brief Constructor from existing Pointer with custom deleter. */
      SharedCountImpl(T* elem,const Deleter& deleter) :
        SharedCount(),
        deleter_(deleter),
        rep_(elem)
      {}
      /** @brief Copy constructor with type conversion. */
      SharedCountImpl(const SharedCountImpl& rep)
        : SharedCount(rep), deleter_(rep.deleter_), rep_(rep.rep_) {}
      /** @brief Destructor, deletes element_type* rep_ using deleter. */
      ~SharedCountImpl()
      { deleter_(rep_); }

      // store a copy of the deleter
      Deleter deleter_;
      T* rep_;
    };

    /** \brief A default deleter that just calls delete */
    struct DefaultDeleter
    {
      void operator() (element_type* p) const
      { delete p; }
    };


    SharedCount *count_;
    T *rep_;

    // Needed for the implicit conversion to "bool"
    typedef T* *__unspecified_bool_type;

  public:
    /** \brief Implicit conversion to "bool" */
    operator __unspecified_bool_type() const     // never throws
    {
      return rep_ == 0 ? 0 : &shared_ptr::rep_;
    }

  };

  template<class T>
  template<class T1>
  inline shared_ptr<T>::shared_ptr(T1 * p)
  {
    rep_ = p;
    count_ = new SharedCountImpl<DefaultDeleter>(p, DefaultDeleter());
  }

  template<class T>
  inline shared_ptr<T>::shared_ptr(nullptr_t)
  {
    rep_   = 0;
    count_ = 0;
  }

  template<class T>
  template<class T1, class Deleter>
  inline shared_ptr<T>::shared_ptr(T1 * p, Deleter deleter)
  {
    rep_ = p;
    count_ = new SharedCountImpl<Deleter>(p, deleter);
  }

  template<class T>
  inline shared_ptr<T>::shared_ptr()
  {
    rep_ = 0;
    count_=0;
  }

  template<class T>
  template<class T1>
  inline shared_ptr<T>::shared_ptr(const shared_ptr<T1>& other)
    : count_(other.count_), rep_(other.rep_)
  {
    if (rep_)
      ++(count_->count_);
  }

  template<class T>
  inline shared_ptr<T>::shared_ptr(const shared_ptr& other)
    : count_(other.count_), rep_(other.rep_)
  {
    if (rep_)
      ++(count_->count_);
  }

  template<class T>
  template<class T1>
  inline shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T1>& other)
  {
    return assign(other);
  }

  template<class T>
  inline shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr& other)
  {
    return assign(other);
  }

  template<class T>
  template<class T1>
  inline shared_ptr<T>& shared_ptr<T>::assign(const shared_ptr<T1>& other)
  {
    if (other.count_)
      (other.count_->count_)++;

    if(rep_!=0 && --(count_->count_)<=0) {
      delete count_;
    }

    rep_ = other.rep_;
    count_ = other.count_;
    return *this;
  }

  template<class T>
  inline shared_ptr<T>::~shared_ptr()
  {
    if(rep_!=0 && --(count_->count_)==0) {
      delete count_;
      rep_=0;
    }
  }

  template<class T>
  inline T& shared_ptr<T>::operator*()
  {
    return *(rep_);
  }

  template<class T>
  inline T *shared_ptr<T>::operator->()
  {
    return rep_;
  }

  template<class T>
  inline const T& shared_ptr<T>::operator*() const
  {
    return *(rep_);
  }

  template<class T>
  inline const T *shared_ptr<T>::operator->() const
  {
    return rep_;
  }

  template<class T>
  inline int shared_ptr<T>::use_count() const
  {
    return count_->count_;
  }

  template<class T>
  inline void shared_ptr<T>::swap(shared_ptr<T>& other)
  {
    SharedCount* dummy = count_;
    count_=other.count_;
    other.count_ = dummy;
    T* tdummy=rep_;
    rep_ = other.rep_;
    other.rep_ = tdummy;
  }

  template<class T>
  inline void shared_ptr<T>::reset()
  {
    shared_ptr<T>().swap(*this);
  }

  template<class T>
  template<class T1>
  inline void shared_ptr<T>::reset(T1* pointer)
  {
    shared_ptr<T>(pointer).swap(*this);
  }

  template<class T>
  template<class T1, class Deleter>
  inline void shared_ptr<T>::reset(T1* pointer, Deleter deleter)
  {
    shared_ptr<T>(pointer, deleter).swap(*this);
  }

  /** @} */
#endif  // #ifdef SHARED_PTR_NAMESPACE


  // C++0x and Boost have a make_shared implementation, TR1 does not.
  // Unfortunately, TR1 gets picked over Boost if present.
  // Moreover, boost::make_shared() only exists for (remotely) recent versions of Boost.
#if HAVE_MAKE_SHARED
#ifdef SHARED_PTR_NAMESPACE
  using SHARED_PTR_NAMESPACE :: make_shared;
#endif
#else

  template<typename T>
  shared_ptr<T> make_shared()
  {
    return shared_ptr<T>(new T());
  }

  template<typename T, typename Arg1>
  shared_ptr<T> make_shared(const Arg1& arg1)
  {
    return shared_ptr<T>(new T(arg1));
  }

  template<typename T, typename Arg1, typename Arg2>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2)
  {
    return shared_ptr<T>(new T(arg1,arg2));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3,arg4));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4,
                            const Arg5& arg5)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3,arg4,arg5));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4,
                            const Arg5& arg5, const Arg6& arg6)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3,arg4,arg5,arg6));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6, typename Arg7>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4,
                            const Arg5& arg5, const Arg6& arg6, const Arg7& arg7)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3,arg4,arg5,arg6,arg7));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6, typename Arg7, typename Arg8>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4,
                            const Arg5& arg5, const Arg6& arg6, const Arg7& arg7, const Arg8& arg8)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8));
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4,
      typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
  shared_ptr<T> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4,
                            const Arg5& arg5, const Arg6& arg6, const Arg7& arg7, const Arg8& arg8,
                            const Arg9& arg9)
  {
    return shared_ptr<T>(new T(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9));
  }

#endif // custom make_shared

  /**
     @brief implements the Deleter concept of shared_ptr without deleting anything
     @relates shared_ptr

     If you allocate an object on the stack, but want to pass it to a class or function as a shared_ptr,
     you can use this deleter to avoid accidental deletion of the stack-allocated object.

     For convenience we provide two free functions to create a shared_ptr from a stack-allocated object
     (\see stackobject_to_shared_ptr):

     1) Convert a stack-allocated object to a shared_ptr:
     @code
          int i = 10;
          shared_ptr<int> pi = stackobject_to_shared_ptr(i);
     @endcode
     2) Convert a stack-allocated object to a shared_ptr of a base class
     @code
          class A {};
          class B : public A {};

          ...

          B b;
          shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b);
     @endcode

     @tparam T type of the stack-allocated object
   */
  template<class T>
  struct null_deleter
  {
    void operator() (T*) const {}
  };

  /**
     @brief Convert a stack-allocated object to a shared_ptr:
     @relates shared_ptr
     @code
          int i = 10;
          shared_ptr<int> pi = stackobject_to_shared_ptr(i);
     @endcode
   */
  template<typename T>
  inline shared_ptr<T> stackobject_to_shared_ptr(T & t)
  {
    return shared_ptr<T>(&t, null_deleter<T>());
  }

  /**
     @brief Convert a stack object to a shared_ptr of a base class
     @relates shared_ptr
     @code
          class A {};
          class B : public A {};

          ...

          B b;
          shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b);
     @endcode
   */
  template<typename T, typename T2>
  inline shared_ptr<T2> stackobject_to_shared_ptr(T & t)
  {
    return shared_ptr<T2>(dynamic_cast<T2*>(&t), null_deleter<T2>());
  }

}
#endif
