// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_SHARED_PTR_HH
#define DUNE_SHARED_PTR_HH

#include <memory>

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
  // pull in default implementations
  using std::shared_ptr;
  using std::make_shared;

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
     @brief Create a shared_ptr for a stack-allocated object
     @relatesalso null_deleter
     @code
          #include <dune/common/shared_ptr.hh>
     @endcode

     Usage:
     @code
          int i = 10;
          shared_ptr<int> pi = stackobject_to_shared_ptr(i);
     @endcode
     The @c shared_ptr points to the object on the stack, but its deleter is
     set to an instance of @c null_deleter so that nothing happens when the @c
     shared_ptr is destroyed.

     @sa shared_ptr, null_deleter
   */
  template<typename T>
  inline shared_ptr<T> stackobject_to_shared_ptr(T & t)
  {
    return shared_ptr<T>(&t, null_deleter<T>());
  }

  /**
     @brief Create a shared_ptr to a base class for a stack-allocated object
     @relatesalso null_deleter
     @code
          #include <dune/common/shared_ptr.hh>
     @endcode

     Usage:
     @code
          class A {};
          class B : public A {};

          ...

          B b;
          shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b);
     @endcode
     The @c shared_ptr points to the object on the stack, but its deleter is
     set to an instance of @c null_deleter so that nothing happens when the @c
     shared_ptr is destroyed.

     @sa shared_ptr, null_deleter
   */
  template<typename T, typename T2>
  inline shared_ptr<T2> stackobject_to_shared_ptr(T & t)
  {
    return shared_ptr<T2>(dynamic_cast<T2*>(&t), null_deleter<T2>());
  }

}
#endif
