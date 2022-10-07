// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
#define DUNE_COMMON_STD_TYPE_TRAITS_HH

#include <type_traits>
#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>

#if __has_include(<experimental/type_traits>)
#include <experimental/type_traits>
#endif

namespace Dune
{

//! Namespace for features backported from new C++ standards
/**
 * The namespace Dune::Std contains library features of new C++ standards and
 * technical specifications backported to older compilers. Most features are
 * detected and pulled into this namespace from the standard library if your
 * compiler has native support. If it doesn't, we provide a fallback implementation
 * on a best-effort basis.
 *
 * \ingroup CxxUtilities
 */
namespace Std
{

  /// A helper alias template std::bool_constant imported into the namespace Dune::Std
  /// \deprecated Use the `std::bool_constant` directly.
  using std::bool_constant;

#if DUNE_HAVE_CXX_EXPERIMENTAL_IS_DETECTED

  using std::experimental::nonesuch;
  using std::experimental::detected_or;
  using std::experimental::is_detected;
  using std::experimental::detected_t;
  using std::experimental::is_detected_v;
  using std::experimental::detected_or_t;
  using std::experimental::is_detected_exact;
  using std::experimental::is_detected_exact_v;
  using std::experimental::is_detected_convertible;
  using std::experimental::is_detected_convertible_v;

#else // DUNE_HAVE_CXX_EXPERIMENTAL_IS_DETECTED

  // fallback version of std::experimental::is_detected et al., heavily scribbled
  // from cppreference.com (but there is actually not much implementation to the thing)

#ifndef DOXYGEN

  namespace Impl {

    // default version of detector, this gets matched on failure
    template<typename Default, typename Void, template<typename...> class Op, typename... Args>
    struct detector
    {
      using value_t = std::false_type;
      using type = Default;
    };

    // specialization of detector that matches if Op<Args...> can be instantiated
    template<typename Default, template<typename...> class Op, typename... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
    {
      using value_t = std::true_type;
      using type = Op<Args...>;
    };

  }

#endif // DOXYGEN

  //! Type representing a lookup failure by std::detected_or and friends.
  /**
   * This type cannot be constructed, destroyed or copied.
   *
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  struct nonesuch
  {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(const nonesuch&) = delete;
    void operator=(const nonesuch&) = delete;
  };

  //! Detects whether `Op<Args...>` is valid and makes the result available.
  /**
   * This alias template is an alias for an unspecified class type with two
   * nested `typedefs` `value_t` and `type`. It can be used to detect whether
   * the meta function call `Op<Args...>` is valid and access the result of
   * the call by inspecting the returned type, which is defined as follows:
   *
   * * If `Op<Args...>` can be instantiated, `value_t` is an alias for `std::true_type`
   *   and `type` is an alias for `Op<Args...>`.
   * * If `Op<Args...>` is invalid, `value_t` is an alias for `std::false_type`
   *   and `type` is an alias for `Default`.
   *
   * This can be used to safely extract a nested `typedef` from a type `T` that
   * might not define the `typedef`:
     \code
       struct A { using size_type = int ; };
       struct B;

       template<typename T>
       using SizeType = typename T::size_type;

       // this extracts the nested typedef for int
       using st_a = typename detected_or<std::size_t,SizeType,A>::type;
       // as there is no nested typedef in B, this yields std::size_t
       using st_b = typename detected_or<std::size_t,SizeType,B>::type;
     \endcode
   *
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<typename Default, template<typename...> class Op, typename... Args>
  using detected_or = Impl::detector<Default,void,Op,Args...>;

  //! Detects whether `Op<Args...>` is valid.
  /**
   * This alias template checks whether `Op<Args...>` can be instantiated. It is
   * equivalent to `typename detected_or<nonesuch,Op,Args...>::value_t`.
   *
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<template<typename...> class Op, typename... Args>
  using is_detected = typename detected_or<nonesuch,Op,Args...>::value_t;

#ifdef __cpp_variable_templates
  //! Detects whether `Op<Args...>` is valid and makes the result available as a value.
  /**
   * This constexpr variable checks whether `Op<Args...>` can be instantiated. It is
   * equivalent to `is_detected<Op,Args...>::value`.
   *
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<template<typename...> class Op, typename... Args>
  constexpr bool is_detected_v = is_detected<Op,Args...>::value;
#endif // __cpp_variable_templates

  //! Returns `Op<Args...>` if that is valid; otherwise returns nonesuch.
  /**
   * This alias template can be used to instantiate `Op<Args...>` in a context that is
   * not SFINAE-safe by appropriately wrapping the instantiation. If instantiation fails,
   * the marker type nonesuch is returned instead.
   *
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<template<typename...> class Op, typename... Args>
  using detected_t = typename detected_or<nonesuch,Op,Args...>::type;


  //! Returns `Op<Args...>` if that is valid; otherwise returns the fallback type `Default`.
  /**
   * This alias template can be used to instantiate `Op<Args...>` in a context that is
   * not SFINAE-safe by appropriately wrapping the instantiation and automatically falling back
   * to `Default` if instantiation fails.
   *
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<typename Default, template<typename...> class Op, typename... Args>
  using detected_or_t = typename detected_or<Default,Op,Args...>::type;

  //! Checks whether `Op<Args...>` is `Expected` without causing an error if `Op<Args...>` is invalid.
  /**
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<typename Expected, template<typename...> class Op, typename... Args>
  using is_detected_exact = std::is_same<Expected,detected_t<Op,Args...>>;

#ifdef __cpp_variable_templates
  //! Convenient access to the result value of is_detected_exact.
  /**
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<typename Expected, template<typename...> class Op, typename... Args>
  constexpr bool is_detected_exact_v = is_detected_exact<Expected,Op,Args...>::value;
#endif // __cpp_variable_templates

  //! Checks whether `Op<Args...>` is convertible to `Target` without causing an error if `Op<Args...>` is invalid.
  /**
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<typename Target, template<typename...> class Op, typename... Args>
  using is_detected_convertible = std::is_convertible<Target,detected_t<Op,Args...>>;

#ifdef __cpp_variable_templates
  //! Convenient access to the result value of is_detected_convertible.
  /**
   * \note This functionality is part of the C++ library fundamentals TS v2 and might
   *       or might not became part of C++2a.
   *
   * \ingroup CxxUtilities
   */
  template<typename Target, template<typename...> class Op, typename... Args>
  constexpr bool is_detected_convertible_v = is_detected_convertible<Target,Op,Args...>::value;
#endif // __cpp_variable_templates

#endif // DUNE_HAVE_CXX_EXPERIMENTAL_IS_DETECTED



  // conjunction
  // -----------

  /**
   * \brief forms the logical conjunction of the type traits B...
   *
   * \note This functionality is part of the C++17 standard.
   *
   * \ingroup CxxUtilities
   **/
  template< class... B >
  struct [[deprecated("Will be removed after release 2.8. Use std::conjunction instead.")]] conjunction
    : std::conjunction<B...>
  {};


  // disjunction
  // -----------

  /**
   * \brief forms the logical disjunction of the type traits B...
   *
   * \note This functionality is part of the C++17 standard.
   *
   * \ingroup CxxUtilities
   **/
  template< class... B >
  struct [[deprecated("Will be removed after release 2.8. Use std::disjunction instead.")]] disjunction
    : std::disjunction<B...>
  {};


  // negation
  // --------

  /**
   * \brief forms the logical negation of the type traits B...
   *
   * \note This functionality is part of the C++17 standard.
   *
   * \ingroup CxxUtilities
   **/
  template<class B>
  struct [[deprecated("Will be removed after release 2.8. Use std::negation instead.")]] negation
    : std::negation<B>
  {};

} // namespace Std


namespace detail
{
  template <class Type>
  [[deprecated("Type extraction of `TargetType` has failed. Inspect the code calling `detected_or_fallback_t` for getting the source of this warning!")]]
  Type warningIfNotDefined(const Std::nonesuch*);

  template <class Type, class T>
  Type warningIfNotDefined(const T*);
}

//! This type will be either TargetType<Args...> if it exists, or the Fallback<Args...> type.
template <template<typename...> class Fallback,
          template<typename...> class TargetType, typename... Args>
using detected_or_fallback_t = Std::detected_or_t<decltype(
  detail::warningIfNotDefined<Std::detected_t<Fallback, Args...> >(std::declval<const Std::detected_t<TargetType, Args...>*>())),
  TargetType, Args...>;


} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
