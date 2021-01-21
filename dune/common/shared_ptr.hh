// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_SHARED_PTR_HH
#define DUNE_SHARED_PTR_HH

#include <memory>

#include <dune/common/typetraits.hh>
/**
 * @file
 * @brief This file implements several utilities related to std::shared_ptr
 * @author Markus Blatt
 */
namespace Dune
{
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
          std::shared_ptr<int> pi = stackobject_to_shared_ptr(i);
     @endcode
     2) Convert a stack-allocated object to a std::shared_ptr of a base class
     @code
          class A {};
          class B : public A {};

          ...

          B b;
          std::shared_ptr<A> pa = stackobject_to_shared_ptr<A>(b);
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
          std::shared_ptr<int> pi = stackobject_to_shared_ptr(i);
     @endcode
     The @c std::shared_ptr points to the object on the stack, but its deleter is
     set to an instance of @c null_deleter so that nothing happens when the @c
     shared_ptr is destroyed.

     @sa null_deleter
   */
  template<typename T>
  inline std::shared_ptr<T> stackobject_to_shared_ptr(T & t)
  {
    return std::shared_ptr<T>(&t, null_deleter<T>());
  }


  /**
   * \brief Capture R-value reference to shared_ptr
   *
   * This will store a copy of the passed object in
   * a shared_ptr.
   *
   * The two overloads of wrap_or_move are intended
   * to capture references and temporaries in a unique
   * way without creating copies and only moving if
   * necessary.
   *
   * Be careful: Only use this function if you are
   * aware of it's implications. You can e.g. easily
   * end up storing a reference to a temporary if
   * you use this inside of another function without
   * perfect forwarding.
   */
  template<class T>
  auto wrap_or_move(T&& t)
  {
    return std::make_shared<std::decay_t<T>>(std::forward<T>(t));
  }

  /**
   * \brief Capture L-value reference to std::shared_ptr
   *
   * This will store a pointer for the passed reference
   * in a non-owning std::shared_ptr.
   *
   * The two overloads of wrap_or_move are intended
   * to capture references and temporaries in a unique
   * way without creating copies and only moving if
   * necessary.
   *
   * Be careful: Only use this function if you are
   * aware of it's implications. You can e.g. easily
   * end up storing a reference to a temporary if
   * you use this inside of another function without
   * perfect forwarding.
   */
  template<class T>
  auto wrap_or_move(T& t)
  {
    return stackobject_to_shared_ptr(t);
  }



  /**
   * \brief Copy const L-value references into new shared_ptr
   *
   * This will invoke the copy constructor in the make_shared
   * call to create a new shared_ptr storing t
   **/
  template<class T>
  std::shared_ptr<T> copy_or_wrap_or_share(const T& t)
  {
    return std::make_shared<T>(t);
  }

  /**
   * \brief Capture L-value reference to shared_ptr
   *
   * This will store a pointer for the passed reference
   * in a non-owning shared_ptr.
   **/
  template<class T>
  std::shared_ptr<T> copy_or_wrap_or_share(T& t)
  {
    return stackobject_to_shared_ptr(t);
  }

  /**
   * \brief Capture R-value reference to shared_ptr
   *
   * This will store a copy of the passed object in
   * a shared_ptr.
   *
   * This overload captures only real R-value references.
   **/
  template<class T,
    std::enable_if_t<!std::is_lvalue_reference<T>::value, int> = 0>
  auto copy_or_wrap_or_share(T&& t)
  {
    return std::make_shared<std::decay_t<T>>(std::forward<T>(t));
  }

  /// \brief Do not allow to wrap raw pointers
  template<class T>
  std::shared_ptr<T> copy_or_wrap_or_share(T* t)
  {
    static_assert(not std::is_pointer<T*>::value,
      "Raw pointers must be wrapped into smart pointers or references to clarify ownership");
    return {};
  }

  /**
   * \brief Forward shared_ptr
   *
   * This will share the ownership with the argument t
   **/
  template<class T>
  std::shared_ptr<T> copy_or_wrap_or_share(std::shared_ptr<T> t)
  {
    return std::move(t);
  }

  /**
   * \brief move a unique_ptr into shared_ptr
   *
   * This will transform the unique_ptr into a shared_ptr, thus releasing the original ownership.
   **/
  template<class T>
  std::shared_ptr<T> copy_or_wrap_or_share(std::unique_ptr<T> t)
  {
    return std::shared_ptr<T>(std::move(t));
  }
}
#endif
