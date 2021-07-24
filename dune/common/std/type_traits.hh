// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
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

  // to_false_type
  // -------------

  /** \class to_false_type
   *
   *  \brief template mapping a type to <tt>std::false_type</tt>
   *  \deprecated Use Dune::AlwaysFalse (from dune/common/typetraits.hh) instead
   *  \tparam T Some type
   *
   *  Suppose you have a template class. You want to document the required
   *  members of this class in the non-specialized template, but you know that
   *  actually instantiating the non-specialized template is an error. You can
   *  try something like this:
   *  \code
   *  template<typename T>
   *  struct Traits
   *  {
   *    static_assert(false,
   *                  "Instanciating this non-specialized template is an "
   *                  "error. You should use one of the specializations "
   *                  "instead.");
   *    //! The type used to frobnicate T
   *    typedef void FrobnicateType;
   *  };
   *  \endcode
   *  This will trigger static_assert() as soon as the compiler reads the
   *  definition for the Traits template, since it knows that "false" can never
   *  become true, no matter what the template parameters of Traits are. As a
   *  workaround you can use to_false_type: replace <tt>false</tt> by
   *  <tt>to_false_type<T>::value</tt>, like this:
   *  \code
   *  template<typename T>
   *  struct Traits
   *  {
   *    static_assert(Std::to_false_type<T>::value,
   *                  "Instanciating this non-specialized template is an "
   *                  "error. You should use one of the specializations "
   *                  "instead.");
   *    //! The type used to frobnicate T
   *    typedef void FrobnicateType;
   *  };
   *  \endcode
   *  Since there might be an specialization of to_false_type for template
   *  parameter T, the compiler cannot trigger static_assert() until the type
   *  of T is known, that is, until Traits<T> is instantiated.
   *
   * \ingroup CxxUtilities
   */
  template< typename T >
  struct [[deprecated("Will be removed after release 2.8. Use Dune::AlwaysFalse (from dune/common/typetraits.hh)")]] to_false_type : public std::false_type {};



  // to_true_type
  // ------------

  /** \class to_true_type
   *
   *  \brief template mapping a type to <tt>std::true_type</tt>
   *  \deprecated Use Dune::AlwaysFalse (from dune/common/typetraits.hh) instead
   *  \tparam T Some type
   *
   *  \note This class exists mostly for consistency with to_false_type.
   *
   * \ingroup CxxUtilities
   */
  template< typename T >
  struct [[deprecated("Will be removed after release 2.8. Use Dune::AlwaysTrue (from dune/common/typetraits.hh)")]] to_true_type : public std::true_type {};


  /// A helper alias template std::bool_constant imported into the namespace Dune::Std
  /// \deprecated Use the `std::bool_constant` directly.
  using std::bool_constant;


  namespace Impl {

    // If R is void we only need to check if F can be called
    // with given Args... list. If this is not possible
    // result_of_t is not defined and this overload is disabled.
    template<class R, class F, class... Args,
      std::enable_if_t<
        std::is_same<std::void_t<std::result_of_t<F(Args...)>>, R>::value
      , int> = 0>
    std::true_type is_callable_helper(PriorityTag<2>)
    { return {}; }

    // Check if result of F(Args...) can be converted to R.
    // If F cannot even be called with given Args... then
    // result_of_t is not defined and this overload is disabled.
    template<class R, class F, class... Args,
      std::enable_if_t<
        std::is_convertible<std::result_of_t<F(Args...)>, R>::value
      , int> = 0>
    std::true_type is_callable_helper(PriorityTag<1>)
    { return {}; }

    // If none of the above matches, F can either not be called
    // with given Args..., or the result cannot be converted to
    // void, or R is not void.
    template<class R, class F, class... Args>
    std::false_type is_callable_helper(PriorityTag<0>)
    { return {}; }
  }

  /**
   * \brief Traits class to check if function is callable
   * \deprecated Use std::is_invocable from <type_traits>
   *
   * \tparam D Function descriptor
   * \tparam R Return value
   *
   * If D = F(Args...) this checks if F can be called with an
   * argument list of type Args..., and if the return value can
   * be converted to R. If R is void, any return type is accepted.
   * The result is encoded by deriving from std::integral_constant<bool, result>.
   *
   * If D is not of the form D = F(Args...) this class is not defined.
   *
   * This implements std::is_callable as proposed in N4446 for C++17.
   *
   * \ingroup CxxUtilities
   */
  template <class D, class R= void>
  struct is_callable;

  /**
   * \brief Traits class to check if function is callable
   * \deprecated Use std::is_invocable from <type_traits>
   *
   * \tparam D Function descriptor
   * \tparam R Return value
   *
   * If D = F(Args...) this checks if F can be called with an
   * argument list of type Args..., and if the return value can
   * be converted to R. If R is void, any return type is accepted.
   * The result is encoded by deriving from std::integral_constant<bool, result>.
   *
   * If D is not of the form D = F(Args...) this class is not defined.
   *
   * This implements std::is_callable as proposed in N4446 for C++17.
   *
   * \ingroup CxxUtilities
   */
  template <class F, class... Args, class R>
  struct [[deprecated("Use std::is_invocable from <type_traits>. Will be removed after release 2.8")]] is_callable< F(Args...), R> :
      decltype(Impl::is_callable_helper<R, F, Args...>(PriorityTag<42>()))
  {};


  /**
   * \brief Traits class to check if function is invocable
   * \deprecated Use std::is_invocable from <type_traits>
   *
   * \tparam F    Function to check
   * \tparam Args Function arguments to check
   *
   * This checks if F can be called with an arguments list of type Args....
   * The result is encoded by deriving from std::integral_constant<bool, result>.
   *
   * This implements std::is_invocable from C++17.
   *
   * \ingroup CxxUtilities
   */
  template <class F, class... Args>
  struct [[deprecated("Use std::is_invocable from <type_traits>. Will be removed after release 2.8")]] is_invocable :
      decltype(Impl::is_callable_helper<void, F, Args...>(PriorityTag<42>()))
  {};

  /**
   * \brief Traits class to check if function is invocable and the return type is compatible
   * \deprecated Use std::is_invocable_r from <type_traits>
   *
   * \tparam R    Desired result type
   * \tparam F    Function to check
   * \tparam Args Function arguments to check
   *
   * This checks if F can be called with an arguments list of type Args..., and
   * if the return value can be converted to R.
   * The result is encoded by deriving from std::integral_constant<bool, result>.
   *
   * This implements std::is_invocable_r from C++17.
   *
   * \ingroup CxxUtilities
   */
  template <class R, class F, class... Args>
  struct [[deprecated("Use std::is_invocable_r from <type_traits>. Will be removed after release 2.8")]] is_invocable_r :
      decltype(Impl::is_callable_helper<R, F, Args...>(PriorityTag<42>()))
  {};


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
  struct [[deprecated("Will be removed after release 2.8. Use std::conjuction instead.")]] conjunction
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

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
