// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TYPETRAITS_HH
#define DUNE_TYPETRAITS_HH

#include <complex>
#include <type_traits>
#include <utility>

#include <dune/common/deprecated.hh>

namespace Dune
{

  namespace Impl
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
  using void_t = typename Impl::voider<Types...>::type;

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

  /**
   * @brief Enable typedef if two types are interoperable.
   *
   * (also see IsInteroperable)
   */
  template<class T1, class T2, class Type>
  struct EnableIfInterOperable
    : public std::enable_if<IsInteroperable<T1,T2>::value, Type>
  {};

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

  //! \brief Whether this type acts as a scalar in the context of
  //!        (hierarchically blocked) containers
  /**
     All types `T` for which `IsNumber<T>::value` is `true` will act as a
     scalar when used with possibly hierarchically blocked containers, such as
     `FieldMatrix`, `FieldVector`, `BCRSMatrix`, `BlockVector`,
     `MultiTypeBlockVector`, etc.  This enables earlier error reporting when
     implementing binary container-scalar operators, such as `=` or `*=`.

     By default is `true` for all arithmetic types (as per
     `std::is_arithmetic`), and for `T=std::complex<U>`, iff
     `IsNumber<U>::value` itself is `true`.

     Should be specialized to `true` for e.g. extended precision types or
     automatic differentiation types, or anything else that might sensibly be
     an element of a matrix or vector.
   */
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

  namespace Impl {

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
    : public Impl::_is_indexable<T,I>
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


  namespace Impl {

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
               Impl::_lazy<std::true_type>,
               typename std::conditional<
                 std::is_class<T>::value,
                 Impl::_check_for_index_operator,
                 Impl::_lazy<std::false_type>
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
    void ignore(T&&... /*t*/)
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
  namespace Impl {

  template<class T>
  struct IsTuple : public std::false_type
  {};

  template<class... T>
  struct IsTuple<std::tuple<T...>> : public std::true_type
  {};

  } // namespace Impl

  /**
   * \brief Check if T is a std::tuple<...>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsTuple :
    public Impl::IsTuple<T>
  {};



  // Implementation of IsTupleOrDerived
  namespace Impl {

  template<class... T, class Dummy>
  std::true_type isTupleOrDerived(const std::tuple<T...>*, Dummy)
  { return {}; }

  template<class Dummy>
  std::false_type isTupleOrDerived(const void*, Dummy)
  { return {}; }

  } // namespace Impl

  /**
   * \brief Check if T derived from a std::tuple<...>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsTupleOrDerived :
    public decltype(Impl::isTupleOrDerived(std::declval<T*>(), true))
  {};



  // Implementation of is IsIntegralConstant
  namespace Impl {

  template<class T>
  struct IsIntegralConstant : public std::false_type
  {};

  template<class T, T t>
  struct IsIntegralConstant<std::integral_constant<T, t>> : public std::true_type
  {};

  } // namespace Impl

  /**
   * \brief Check if T is an std::integral_constant<I, i>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsIntegralConstant : public Impl::IsIntegralConstant<std::decay_t<T>>
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



  namespace Impl {

  template<class T, T...>
  struct IntegerSequenceHelper;

  // Helper struct to compute the i-th entry of a std::integer_sequence
  //
  // This could also be implemented using std::get<index>(std::make_tuple(t...)).
  // However, the gcc-6 implementation of std::make_tuple increases the instantiation
  // depth by 15 levels for each argument, such that the maximal instantiation depth
  // is easily hit, especially with clang where it is set to 256.
  template<class T, T head, T... tail>
  struct IntegerSequenceHelper<T, head, tail...>
  {

    // get first entry
    static constexpr auto get(std::integral_constant<std::size_t, 0>)
    {
      return std::integral_constant<T, head>();
    }

    // call get with first entry cut off and decremented index
    template<std::size_t index,
      std::enable_if_t<(index > 0) and (index < sizeof...(tail)+1), int> = 0>
    static constexpr auto get(std::integral_constant<std::size_t, index>)
    {
      return IntegerSequenceHelper<T, tail...>::get(std::integral_constant<std::size_t, index-1>());
    }

    // use static assertion if index exceeds size
    template<std::size_t index,
      std::enable_if_t<(index >= sizeof...(tail)+1), int> = 0>
    static constexpr auto get(std::integral_constant<std::size_t, index>)
    {
      static_assert(index < sizeof...(tail)+1, "index used in IntegerSequenceEntry exceed size");
    }
  };

  } // end namespace Impl


  /**
   * \brief Get entry of std::integer_sequence
   *
   * \param seq An object of type std::integer_sequence<...>
   * \param i Index
   *
   * \return The i-th entry of the integer_sequence encoded as std::integral_constant<std::size_t, entry>.
   *
   */
  template<class T, T... t, std::size_t index>
  constexpr auto integerSequenceEntry(std::integer_sequence<T, t...> /*seq*/, std::integral_constant<std::size_t, index> i)
  {
    static_assert(index < sizeof...(t), "index used in IntegerSequenceEntry exceed size");
    return Impl::IntegerSequenceHelper<T, t...>::get(i);
  }

  template<class IntegerSequence, std::size_t index>
  struct IntegerSequenceEntry;

  /**
   * \brief Get entry of std::integer_sequence
   *
   * Computes the i-th entry of the integer_sequence. The result
   * is exported as ::value by deriving form std::integral_constant<std::size_t, entry>.
   */
  template<class T, T... t, std::size_t i>
  struct IntegerSequenceEntry<std::integer_sequence<T, t...>, i>
    : public decltype(Impl::IntegerSequenceHelper<T, t...>::get(std::integral_constant<std::size_t, i>()))
  {};



  /** @} */
}
#endif
