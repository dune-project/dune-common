// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TYPETRAITS_HH
#define DUNE_TYPETRAITS_HH

#include <complex>
#include <type_traits>

#include <dune/common/deprecated.hh>
#include <dune/common/std/utility.hh>

namespace Dune
{

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
   * @brief Determines wether a type is const or volatile and provides the
   * unqualified types.
   */
  template<typename T>
  struct ConstantVolatileTraits
  {
    enum {
      /** @brief True if T has a volatile specifier. */
      isVolatile=std::is_volatile<T>::value,
      /** @brief True if T has a const qualifier. */
      isConst=std::is_const<T>::value
    };

    /** @brief The unqualified type. */
    typedef typename std::remove_cv<T>::type UnqualifiedType;
    /** @brief The const type. */
    typedef typename std::add_const<UnqualifiedType>::type ConstType;
    /** @brief The const volatile type. */
    typedef typename std::add_cv<UnqualifiedType>::type ConstVolatileType;
  };

  /** @brief Tests wether a type is volatile. */
  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::is_volatile instead!") IsVolatile
  {
    enum {
      /** @brief True if The type is volatile. */
      value=std::is_volatile<T>::value
    };
  };

  /** @brief Tests wether a type is constant. */
  template<typename T>
  struct DUNE_DEPRECATED_MSG("Use std::is_const instead!") IsConst
  {
    enum {
      /** @brief True if The type is constant. */
      value=std::is_const<T>::value
    };
  };

  using std::remove_const;
  using std::remove_reference;

  /**
   * @brief Checks wether a type is convertible to another.
   *
   * @tparam From type you want to convert
   * @tparam To type you want to obtain
   */
  template<class From, class To>
  class Conversion
  {
  public:
    enum {
      /** @brief True if the conversion exists. */
      exists =  std::is_convertible<From,To>::value,
      /** @brief Whether the conversion exists in both ways. */
      isTwoWay = exists && std::is_convertible<To,From>::value,
      /** @brief True if To and From are the same type. */
      sameType = std::is_same<From,To>::value
    };
  };

  /**
   * @brief Checks wether a type is derived from another.
   *
   * @tparam Base the potential base class you want to test for
   * @tparam Derived type you want to test
   */
  template <class Base, class Derived>
  class DUNE_DEPRECATED_MSG("Use std::is_base_of instead!") IsBaseOf
  {
  public:
    enum {
      /** @brief True if Base is a base class of Derived. */
      value = std::is_base_of<Base, Derived>::value
    };
  };

  /**
   * @brief Checks wether two types are interoperable.
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
      value = Conversion<T1,T2>::exists || Conversion<T2,T1>::exists
    };
  };

  using std::enable_if;

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
  using std::is_same;
  using std::conditional;
  using std::integral_constant;
  using std::true_type;
  using std::false_type;
  using std::is_pointer;
  using std::is_lvalue_reference;
  using std::remove_pointer;

  /**
     \brief template which always yields a false value
     \tparam T Some type.  It sould be a type expression involving template
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
     \tparam T Some type.  It sould be a type expression involving template
               parameters of the class or function using AlwaysTrue.

     \note This class exists mostly for consistency with AlwaysFalse.
   */
  template<typename T>
  struct AlwaysTrue {
    //! always a true value
    static const bool value = true;
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
    struct _is_indexable<T,I,typename std::enable_if<(sizeof(Std::declval<T>()[Std::declval<I>()]) > 0),int>::type>
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
    struct _is_indexable<T,decltype(Std::declval<T>()[0],0)>
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

  template <class> struct FieldTraits;

  //! Is void for all valid input types (see N3911). The workhorse for C++11 SFINAE-techniques.
  template <class... Types>
  using void_t = typename detail::voider<Types...>::type;

  //! Convenient access to FieldTraits<Type>::field_type.
  template <class Type>
  using field_t = typename FieldTraits<Type>::field_type;

  //! Convenient access to FieldTraits<Type>::real_type.
  template <class Type>
  using real_t = typename FieldTraits<Type>::real_type;

  /** @} */
}
#endif
