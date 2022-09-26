// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_PROXYMEMBERACCESS_HH
#define DUNE_COMMON_PROXYMEMBERACCESS_HH

/**
 * \file
 * \brief infrastructure for supporting operator->() on both references and proxies
 * \ingroup CxxUtilities
 */

#include <type_traits>
#include <utility>

namespace Dune {

  namespace Impl {

    // helper struct to store a temporary / proxy
    // for the duration of the member access
    template<typename T>
    struct member_access_proxy_holder
    {

      // only support moving the temporary into the holder object
      member_access_proxy_holder(T&& t)
        : _t(std::move(t))
      {}

      // The object is fundamentally a temporary, i.e. an rvalue,
      //
      const T* operator->() const
      {
        return &_t;
      }

      T _t;

    };

  } // end Impl namespace


#ifdef DOXYGEN

  //! Transparent support for providing member access to both lvalues and rvalues (temporary proxies).
  /**
   * If an iterator facade (like entity iterators) wants to allow the embedded implementation to
   * return either an (internally stored) reference or a temporary object and expose these two
   * behaviors to enable performance optimizations, operator->() needs special handling: If the
   * implementation returns a reference, operator->() in the facade can simply return the address
   * of the referenced object, but if the returned object is a temporary, we need to capture and
   * store it in a helper object to make sure it outlives the member access. This function transparently
   * supports both variants. It should be used like this:
   *
   * \code
   * class iterator
   * {
   *   ...
   *
   *   decltype(handle_proxy_member_access(implementation.dereference()))
   *   operator->() const
   *   {
   *     return handle_proxy_member_access(implementation.dereference());
   *   }
   *
   *   ...
   * };
   * \endcode
   *
   * \note This function exploits the special type deduction rules for unqualified rvalue references
   *       to distinguish between lvalues and rvalues and thus needs to be passed the object returned
   *       by the implementation.
   *
   * \ingroup CxxUtilities
   */
  template<typename T>
  pointer_or_proxy_holder
  handle_proxy_member_access(T&& t);

#else // DOXYGEN


  // This version matches lvalues (the C++ type deduction rules state that
  // the T&& signature deduces to a reference iff the argument is an lvalue).
  // As the argument is an lvalue, we do not have to worry about its lifetime
  // and can just return its address.
  template<typename T>
  inline typename std::enable_if<
    std::is_lvalue_reference<T>::value,
    typename std::add_pointer<
      typename std::remove_reference<
        T
        >::type
      >::type
    >::type
  handle_proxy_member_access(T&& target)
  {
    return &target;
  }

  // This version matches rvalues (the C++ type deduction rules state that
  // the T&& signature deduces to a non-reference iff the argument is an rvalue).
  // In this case, we have to capture the rvalue in a new object to make sure it
  // is kept alive for the duration of the member access. For this purpose, we move
  // it into a member_access_proxy_holder instance.
  template<typename T>
  inline typename std::enable_if<
    !std::is_lvalue_reference<T>::value,
    Impl::member_access_proxy_holder<T>
    >::type
  handle_proxy_member_access(T&& target)
  {
    return {std::forward<T>(target)};
  }

#endif // DOXYGEN

} // namespace Dune

#endif // DUNE_COMMON_PROXYMEMBERACCESS_HH
