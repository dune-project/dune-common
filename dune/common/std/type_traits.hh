// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
#define DUNE_COMMON_STD_TYPE_TRAITS_HH

#include <type_traits>
#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>

namespace Dune
{

namespace Std
{

  // to_false_type
  // -------------

  /** \class to_false_type
   *
   *  \brief template mapping a type to <tt>std::false_type</tt>
   *
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
   */
  template< typename T >
  struct to_false_type : public std::false_type {};



  // to_true_type
  // ------------

  /** \class to_true_type
   *
   *  \brief template mapping a type to <tt>std::true_type</tt>
   *
   *  \tparam T Some type
   *
   *  \note This class exists mostly for consistency with to_false_type.
   */
  template< typename T >
  struct to_true_type : public std::true_type {};


#if __cpp_lib_bool_constant

    using std::bool_constant;

#elif __cpp_lib_experimental_bool_constant

    using std::experimental::bool_constant;

#else

    /**
     *  \brief A template alias for std::integral_constant<bool, value>
     *
     *  \tparam value Boolean value to encode as std::integral_constant<bool, value>
     */
    template <bool value>
    using bool_constant = std::integral_constant<bool, value>;

#endif


  namespace Imp {

    // If R is void we only need to check if F can be called
    // with given Args... list. If this is not possible
    // result_of_t is not defined and this overload is disabled.
    template<class R, class F, class... Args,
      std::enable_if_t<
        std::is_same<void_t<std::result_of_t<F(Args...)>>, R>::value
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
   */
  template <class D, class R= void>
  struct is_callable;

  /**
   * \brief Traits class to check if function is callable
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
   */
  template <class F, class... Args, class R>
  struct is_callable< F(Args...), R> :
      decltype(Imp::is_callable_helper<R, F, Args...>(PriorityTag<42>()))
  {};








} // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
