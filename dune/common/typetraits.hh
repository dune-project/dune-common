// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TYPETRAITS_HH
#define DUNE_TYPETRAITS_HH

#include <complex>
#include <type_traits>

#include <dune/common/deprecated.hh>

namespace Dune
{

  namespace detail
  {
    ///
    /**
     * @internal
     * @brief Helper to make void_t work with gcc versions prior to gcc 5.0.
     *
     * This was not a compiler bug, but an accidental omission in the C++11 standard (see N3911, CWG issue 1558).
     * It is not clearly specified what happens
     * with unused template arguments in template aliases. The developers of GCC decided to ignore them, thus making void_t equivalent to void.
     * With gcc 5.0 this was changed and the voider-hack is no longer needed.
     */
    template <class...>
    struct voider
    {
      using type = void;
    };
  }

  //! Is void for all valid input types (see N3911). The workhorse for C++11 SFINAE-techniques.
  template <class... Types>
  using void_t = typename detail::voider<Types...>::type;

  /**
   * @file
   * @brief Traits for type conversions and type information.
   * @author Markus Blatt, Christian Engwer
   */
  /** @addtogroup Common
   *
   * @{
   */

  /**
   * @brief Just an empty class
   */
  struct Empty {};

  /**
   * @brief Determines whether a type is const or volatile and provides the
   * unqualified types.
   */
  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use <type_traits> instead!") ConstantVolatileTraits
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_volatile/std::is_const instead!") {
      /** @brief True if T has a volatile specifier. */
      isVolatile=std::is_volatile<T>::value,
      /** @brief True if T has a const qualifier. */
      isConst=std::is_const<T>::value
    };

    /** @brief The unqualified type. */
    typedef DUNE_DEPRECATED_MSG("Use std::remove_const instead!") typename std::remove_cv<T>::type UnqualifiedType;
    /** @brief The const type. */
    typedef DUNE_DEPRECATED_MSG("Use std::add_const instead!") typename std::add_const<UnqualifiedType>::type ConstType;
    /** @brief The const volatile type. */
    typedef DUNE_DEPRECATED_MSG("Use std::add_cv instead!") typename std::add_cv<UnqualifiedType>::type ConstVolatileType;
  };

  /** @brief Tests whether a type is volatile. */
  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::is_volatile instead!") IsVolatile
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_volatile instead!") {
      /** @brief True if The type is volatile. */
      value=std::is_volatile<T>::value
    };
  };

  /** @brief Tests whether a type is constant. */
  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::is_const instead!") IsConst
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_const instead!") {
      /** @brief True if The type is constant. */
      value=std::is_const<T>::value
    };
  };

  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::remove_const instead!") remove_const
  {
    typedef DUNE_DEPRECATED_MSG("Use std::remove_const instead!") typename std::remove_const<T>::type type;
  };

  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::remove_reference instead!") remove_reference
  {
    typedef DUNE_DEPRECATED_MSG("Use std::remove_reference instead!") typename std::remove_reference<T>::type type;
  };

  /**
   * @brief Checks whether a type is convertible to another.
   *
   * @tparam From type you want to convert
   * @tparam To type you want to obtain
   */
  template<class From, class To>
  struct DUNE_DEPRECATED_MSG("Use std::is_convertible/std::is_same instead!") Conversion
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_convertible/std::is_same instead!") {
      /** @brief True if the conversion exists. */
      exists =  std::is_convertible<From,To>::value,
      /** @brief Whether the conversion exists in both ways. */
      isTwoWay = std::is_convertible<From,To>::value && std::is_convertible<To,From>::value,
      /** @brief True if To and From are the same type. */
      sameType = std::is_same<From,To>::value
    };
  };

  /**
   * @brief Checks whether a type is derived from another.
   *
   * @tparam Base the potential base class you want to test for
   * @tparam Derived type you want to test
   */
  template <class Base, class Derived>
  struct DUNE_DEPRECATED_MSG("Use std::is_base_of instead!") IsBaseOf
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_base_of instead!") {
      /** @brief True if Base is a base class of Derived. */
      value = std::is_base_of<Base, Derived>::value
    };
  };

  /**
   * @brief Checks whether two types are interoperable.
   *
   * Two types are interoperable if conversions in either directions
   * exists.
   */
  template<class T1, class T2>
  struct IsInteroperable
  {
    enum {
      /**
       * @brief True if either a conversion from T1 to T2 or vice versa
       * exists.
       */
      value = std::is_convertible<T1,T2>::value || std::is_convertible<T2,T1>::value
    };
  };

  template<bool B, class T = void>
  struct enable_if
  {};

  template<class T>
  struct enable_if<true,T>
  {
    typedef DUNE_DEPRECATED_MSG("Use std::enable_if instead!") T type;
  };

  /**
   * @brief Enable typedef if two types are interoperable.
   *
   * (also see IsInteroperable)
   */
  template<class T1, class T2, class Type>
  struct EnableIfInterOperable
    : public std::enable_if<IsInteroperable<T1,T2>::value, Type>
  {};

  // pull in default implementation
  template<typename T, typename U>
  struct DUNE_DEPRECATED_MSG("Use std::is_same instead!") is_same
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_same instead!") {
      value = std::is_same<T,U>::value
    };
  };

  template<bool B, typename T, typename F>
  struct DUNE_DEPRECATED_MSG("Use std::conditional instead!") conditional
  {
    typedef DUNE_DEPRECATED_MSG("Use std::conditional instead!") typename std::conditional<B,T,F>::type type;
  };

  template<typename T, T v>
  struct DUNE_DEPRECATED_MSG("Use std::integral_constant instead!") integral_constant
  {
    DUNE_DEPRECATED_MSG("Use std::integral_constant instead!")
    static constexpr T value = v;
  };

  struct DUNE_DEPRECATED_MSG("Use std::true_type instead!") true_type
  {
    enum DUNE_DEPRECATED_MSG("Use std::true_type instead!") {
      value = true
    };
  };

  struct DUNE_DEPRECATED_MSG("Use std::false_type instead!") false_type
  {
    enum DUNE_DEPRECATED_MSG("Use std::false_type instead!") {
      value = false
    };
  };

  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::is_pointer instead!") is_pointer
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_pointer instead!") {
      value = std::is_pointer<T>::value
    };
  };

  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::is_lvalue_reference instead!") is_lvalue_reference
  {
    enum DUNE_DEPRECATED_MSG("Use std::is_lvalue_reference instead!") {
      value = std::is_lvalue_reference<T>::value
    };
  };

  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::remove_pointer instead!") remove_pointer
  {
    typedef DUNE_DEPRECATED_MSG("Use std::remove_pointer instead!") typename std::remove_pointer<T>::type type;
  };

  /**
     \brief template which always yields a false value
     \tparam T Some type.  It should be a type expression involving template
               parameters of the class or function using AlwaysFalse.

     Suppose you have a template class.  You want to document the required
     members of this class in the non-specialized template, but you know that
     actually instantiating the non-specialized template is an error.  You
     can try something like this:
     \code
     template<typename T>
     struct Traits {
       static_assert(false,
                          "Instanciating this non-specialized template is an "
                          "error.  You should use one of the specializations "
                          "instead.");
       //! The type used to frobnicate T
       typedef void FrobnicateType;
     };
     \endcode
     This will trigger static_assert() as soon as the compiler reads the
     definition for the Traits template, since it knows that "false" can
     never become true, no matter what the template parameters of Traits are.
     As a workaround you can use AlwaysFalse: replace <tt>false</tt> by
     <tt>AlwaysFalse<T>::value</tt>, like this:
     \code
     template<typename T>
     struct Traits {
       static_assert(AlwaysFalse<T>::value,
                          "Instanciating this non-specialized template is an "
                          "error.  You should use one of the specializations "
                          "instead.");
       //! The type used to frobnicate T
       typedef void FrobnicateType;
     };
     \endcode
     Since there might be an specialization of AlwaysFalse for template
     parameter T, the compiler cannot trigger static_assert() until the
     type of T is known, that is, until Traits<T> is instantiated.
   */
  template<typename T>
  struct AlwaysFalse {
    //! always a false value
    static const bool value = false;
  };

  /**
     \brief template which always yields a true value
     \tparam T Some type.  It should be a type expression involving template
               parameters of the class or function using AlwaysTrue.

     \note This class exists mostly for consistency with AlwaysFalse.
   */
  template<typename T>
  struct AlwaysTrue {
    //! always a true value
    static const bool value = true;
  };

  template <typename T>
  struct IsNumber
    : public std::integral_constant<bool, std::is_arithmetic<T>::value> {
  };

  template <typename T>
  struct IsNumber<std::complex<T>>
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

  template <typename T>
  struct has_nan
      : public std::integral_constant<bool, std::is_floating_point<T>::value> {
  };

  template <typename T>
  struct has_nan<std::complex<T>>
      : public std::integral_constant<bool, std::is_floating_point<T>::value> {
  };

#if defined(DOXYGEN) or HAVE_IS_INDEXABLE_SUPPORT

#ifndef DOXYGEN

  namespace detail {

    template<typename T, typename I, typename = int>
    struct _is_indexable
      : public std::false_type
    {};

    template<typename T, typename I>
    struct _is_indexable<T,I,typename std::enable_if<(sizeof(std::declval<T>()[std::declval<I>()]) > 0),int>::type>
      : public std::true_type
    {};

  }

#endif // DOXYGEN

  //! Type trait to determine whether an instance of T has an operator[](I), i.e. whether
  //! it can be indexed with an index of type I.
  /**
   * \warning Not all compilers support testing for arbitrary index types. In particular, there
   *          are problems with GCC 4.4 and 4.5.
   */
  template<typename T, typename I = std::size_t>
  struct is_indexable
    : public detail::_is_indexable<T,I>
  {};


#else // defined(DOXYGEN) or HAVE_IS_INDEXABLE_SUPPORT


  // okay, here follows a mess of compiler bug workarounds...
  // GCC 4.4 dies if we try to subscript a simple type like int and
  // both GCC 4.4 and 4.5 don't like using arbitrary types as subscripts
  // for macros.
  // So we make sure to only ever attempt the SFINAE for operator[] for
  // class types, and to make sure the compiler doesn't become overly eager
  // we have to do some lazy evaluation tricks with nested templates and
  // stuff.
  // Let's get rid of GCC 4.4 ASAP!


  namespace detail {

    // simple wrapper template to support the lazy evaluation required
    // in _is_indexable
    template<typename T>
    struct _lazy
    {
      template<typename U>
      struct evaluate
      {
        typedef T type;
      };
    };

    // default version, gets picked if SFINAE fails
    template<typename T, typename = int>
    struct _is_indexable
      : public std::false_type
    {};

    // version for types supporting the subscript operation
    template<typename T>
    struct _is_indexable<T,decltype(std::declval<T>()[0],0)>
      : public std::true_type
    {};

    // helper struct for delaying the evaluation until we are sure
    // that T is a class (i.e. until we are outside std::conditional
    // below)
    struct _check_for_index_operator
    {

      template<typename T>
      struct evaluate
        : public _is_indexable<T>
      {};

    };

  }

  // The rationale here is as follows:
  // 1) If we have an array, we assume we can index into it. That isn't
  //    true if I isn't an integral type, but that's why we have the static assertion
  //    in the body - we could of course try and check whether I is integral, but I
  //    can't be arsed and want to provide a motivation to switch to a newer compiler...
  // 2) If we have a class, we use SFINAE to check for operator[]
  // 3) Otherwise, we assume that T does not support indexing
  //
  // In order to make sure that the compiler doesn't accidentally try the SFINAE evaluation
  // on an array or a scalar, we have to resort to lazy evaluation.
  template<typename T, typename I = std::size_t>
  struct is_indexable
    : public std::conditional<
               std::is_array<T>::value,
               detail::_lazy<std::true_type>,
               typename std::conditional<
                 std::is_class<T>::value,
                 detail::_check_for_index_operator,
                 detail::_lazy<std::false_type>
                 >::type
               >::type::template evaluate<T>::type
  {
    static_assert(std::is_same<I,std::size_t>::value,"Your compiler is broken and does not support checking for arbitrary index types");
  };


#endif // defined(DOXYGEN) or HAVE_IS_INDEXABLE_SUPPORT

  namespace Impl {
    // This function does nothing.
    // By passing expressions to this function one can avoid
    // "value computed is not used" warnings that may show up
    // in a comma expression.
    template<class...T>
    void ignore(T&&... t)
    {}
  }

  /**
     typetrait to check that a class has begin() and end() members
   */
  // default version, gets picked if SFINAE fails
  template<typename T, typename = void>
  struct is_range
    : public std::false_type
  {};

#ifndef DOXYGEN
  // version for types with begin() and end()
  template<typename T>
  struct is_range<T, decltype(Impl::ignore(
      std::declval<T>().begin(),
      std::declval<T>().end(),
      std::declval<T>().begin() != std::declval<T>().end(),
      decltype(std::declval<T>().begin()){std::declval<T>().end()},
      ++(std::declval<std::add_lvalue_reference_t<decltype(std::declval<T>().begin())>>()),
      *(std::declval<T>().begin())
      ))>
    : public std::true_type
  {};
#endif

  template <class> struct FieldTraits;

  //! Convenient access to FieldTraits<Type>::field_type.
  template <class Type>
  using field_t = typename FieldTraits<Type>::field_type;

  //! Convenient access to FieldTraits<Type>::real_type.
  template <class Type>
  using real_t = typename FieldTraits<Type>::real_type;



  // Implementation of IsTuple
  namespace Imp {

  template<class T>
  struct IsTuple : public std::false_type
  {};

  template<class... T>
  struct IsTuple<std::tuple<T...>> : public std::true_type
  {};

  } // namespace Imp

  /**
   * \brief Check if T is a std::tuple<...>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsTuple :
    public Imp::IsTuple<T>
  {};



  // Implementation of IsTupleOrDerived
  namespace Imp {

  template<class... T, class Dummy>
  std::true_type isTupleOrDerived(const std::tuple<T...>*, Dummy)
  { return {}; }

  template<class Dummy>
  std::false_type isTupleOrDerived(const void*, Dummy)
  { return {}; }

  } // namespace Imp

  /**
   * \brief Check if T derived from a std::tuple<...>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsTupleOrDerived :
    public decltype(Imp::isTupleOrDerived(std::declval<T*>(), true))
  {};



  // Implementation of is IsIntegralConstant
  namespace Imp {

  template<class T>
  struct IsIntegralConstant : public std::false_type
  {};

  template<class T, T t>
  struct IsIntegralConstant<std::integral_constant<T, t>> : public std::true_type
  {};

  } // namespace Imp

  /**
   * \brief Check if T is an std::integral_constant<I, i>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsIntegralConstant : public Imp::IsIntegralConstant<std::decay_t<T>>
  {};



  /**
   * \brief Compute size of variadic type list
   *
   * \tparam T Variadic type list
   *
   * The ::value member gives the size of the variadic type list T...
   * This should be equivalent to sizeof...(T). However, with clang
   * the latter may produce wrong results if used in template aliases
   * due to clang bug 14858 (https://llvm.org/bugs/show_bug.cgi?id=14858).
   *
   * As a workaround one can use SizeOf<T...>::value instead of sizeof...(T)
   * in template aliases for any code that should work with clang < 3.8.
   */
  template<typename... T>
  struct SizeOf
    : public std::integral_constant<std::size_t,sizeof...(T)>
  {};



  /** @} */
}
#endif
