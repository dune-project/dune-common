// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_REFERENCE_HELPER_HH
#define DUNE_COMMON_REFERENCE_HELPER_HH

#include <type_traits>
#include <functional>



namespace Dune {

namespace Impl {

  template<class T>
  class IsReferenceWrapper : public std::false_type {};

  template<class T>
  class IsReferenceWrapper<std::reference_wrapper<T>> : public std::true_type {};

  template<class T>
  class IsReferenceWrapper<const std::reference_wrapper<T>> : public std::true_type {};

} // namespace Dune::Impl


/**
 * \brief Helper to detect if given type is a std::reference_wrapper
 *
 * \ingroup CxxUtilities
 */
template<class T>
constexpr bool IsReferenceWrapper_v = Impl::IsReferenceWrapper<T>::value;


/**
 * \brief Helper function to resolve std::reference_wrapper
 *
 * This is the overload for plain (mutable or const) l-value reference types.
 * It simply forwards the passed l-value reference.
 *
 * \ingroup CxxUtilities
 */
template<class T>
constexpr T& resolveRef(T& gf) noexcept
{
  return gf;
}


// There's no overload for non std::reference_wrapper r-values,
// because this may lead to undefined behavior whenever the
// return value is stored.
// Notice that deleting the overload is not necessary, but
// helps to document that it is missing on purpose. It also
// leads to nicer error messages.
template<class T>
const auto& resolveRef(T&& gf) = delete;


/**
 * \brief Helper function to resolve std::reference_wrapper
 *
 * This is the overload for std::reference_wrapper<T>.
 * It resolves the reference by returning the stored
 * (mutable or const) l-value reference.  It is safe
 * to call this with mutable or cost l-values
 * as well as r-values of std::reference_wrapper,
 * because the life time of the wrapped l-value reference
 * is independent of the wrapping std::reference_wrapper<T>
 * object.
 *
 * Notice that the copy created by calling this function
 * is easily elided by the compiler, since std::reference_wrapper
 * is trivially copyable.
 *
 * \ingroup CxxUtilities
 */
template<class T>
constexpr T& resolveRef(std::reference_wrapper<T> gf) noexcept
{
  return gf.get();
}



/**
 * \brief Type trait to resolve std::reference_wrapper
 *
 * This is an alias for result of resolveRef.
 * Plain types T or const T are forwarded while
 * for T=std::reference_wrapper<S> the wrapped
 * type S is returned.
 *
 * \ingroup Utility
 */
template<class T>
using ResolveRef_t = std::remove_reference_t<decltype(Dune::resolveRef(std::declval<T&>()))>;


} // namespace Dune



#endif // DUNE_COMMON_REFERENCE_HELPER_HH
