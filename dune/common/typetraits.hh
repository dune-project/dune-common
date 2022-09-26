// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_TYPETRAITS_HH
#define DUNE_TYPETRAITS_HH

#include <complex>
#include <type_traits>
#include <utility>
#include <vector>

namespace Dune
{

  namespace Impl
  {
    ///
    /**
     * @internal
     * @brief Helper to make void_t work with gcc versions prior to gcc 5.0 and some clang versions.
     */
    template <class...>
    struct voider
    {
      using type = void;
    };
  }

  //! Is void for all valid input types. The workhorse for C++11 SFINAE-techniques.
  /**
   * \ingroup CxxUtilities
   *
   * Note, since c++17 there is also `std::void_t` that should be preferred. But, due to an issue
   * in the c++ standard, see CWG issue #1980. "Equivalent but not functionally-equivalent redeclarations",
   * and a corresponding failure in some clang compilers, this tool is left here as a workaround.
   * Use it if you want to specialize multiple classes using `void_t`.
   */
  template <class... Types>
  using void_t = typename Impl::voider<Types...>::type;

  /**
   * @file
   * @brief Traits for type conversions and type information.
   * @author Markus Blatt, Christian Engwer
   */
  /** @addtogroup CxxUtilities
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
    /**
     * @brief True if either a conversion from T1 to T2 or vice versa
     * exists.
     */
    constexpr static bool value = std::is_convertible<T1,T2>::value || std::is_convertible<T2,T1>::value;
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
                          "Instantiating this non-specialized template is an "
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
                          "Instantiating this non-specialized template is an "
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
  struct AlwaysFalse : public std::false_type {};

  /**
     \brief template which always yields a true value
     \tparam T Some type.  It should be a type expression involving template
               parameters of the class or function using AlwaysTrue.

     \note This class exists mostly for consistency with AlwaysFalse.
   */
  template<typename T>
  struct AlwaysTrue : public std::true_type {};

  /**
   * \brief Check if a type is callable with ()-operator and given arguments
   * \ingroup CxxUtilities
   *
   * \tparam D Function descriptor
   * \tparam R Return value type
   *
   * If `D = F(Args...)` this checks if F can be called with an
   * argument list of type `Args...`, and if the return value can
   * be converted to R. If R is `void`, any return type is accepted.
   *
   * The result is encoded by deriving from
   * either `std::true_type` or `std::false_type`
   *
   * If D is not of the form `F(Args...)` this class is not defined.
   *
   * \note This differs from `std::invocable_r` in the way that only
   *       `FunctionObject` types are allowed here while `std::invocable_r`
   *       also accepts pointers to member functions and pointers
   *       to data members (i.e. more general `Callable` types)
   * \note See https://en.cppreference.com/w/cpp/named_req/FunctionObject
   *       for the description of the named requirement `FunctionObject`
   *       and https://en.cppreference.com/w/cpp/named_req/Callable
   *       for `Callable`.
   */
  template<typename D, typename R = void>
  struct IsCallable;

  /**
   * \brief Check if a type is callable with ()-operator and given arguments
   * \ingroup CxxUtilities
   */
  template<typename R, typename F, typename... Args>
  struct IsCallable<F(Args...), R>
  : public std::bool_constant<
      std::is_invocable_r_v<R, F, Args...>
        && !std::is_member_pointer_v<std::decay_t<F>>
    > {};

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

#ifndef DOXYGEN

  template <typename T>
  struct IsNumber<std::complex<T>>
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

#endif // DOXYGEN

  //! \brief Whether this type has a value of NaN.
  /**
   * Internally, this is just a forward to `std::is_floating_point<T>`.
   */
  template <typename T>
  struct HasNaN
      : public std::integral_constant<bool, std::is_floating_point<T>::value> {
  };

#ifndef DOXYGEN

  template <typename T>
  struct HasNaN<std::complex<T>>
      : public std::integral_constant<bool, std::is_floating_point<T>::value> {
  };

#endif // DOXYGEN

#ifndef DOXYGEN

  namespace Impl {

    template<typename T, typename I, typename = int>
    struct IsIndexable
      : public std::false_type
    {};

    template<typename T, typename I>
    struct IsIndexable<T,I,typename std::enable_if<(sizeof(std::declval<T>()[std::declval<I>()]) > 0),int>::type>
      : public std::true_type
    {};

  }

#endif // DOXYGEN

  //! Type trait to determine whether an instance of T has an operator[](I), i.e. whether it can be indexed with an index of type I.
  /**
   * \warning Not all compilers support testing for arbitrary index types. In particular, there
   *          are problems with GCC 4.4 and 4.5.
   */
  template<typename T, typename I = std::size_t>
  struct IsIndexable
    : public Impl::IsIndexable<T,I>
  {};

#ifndef DOXYGEN

  namespace Impl {
    // This function does nothing.
    // By passing expressions to this function one can avoid
    // "value computed is not used" warnings that may show up
    // in a comma expression.
    template<class...T>
    void ignore(T&&... /*t*/)
    {}
  }

#endif // DOXYGEN

  /**
     \brief typetrait to check that a class has begin() and end() members
   */
  // default version, gets picked if SFINAE fails
  template<typename T, typename = void>
  struct IsIterable
    : public std::false_type
  {};

#ifndef DOXYGEN
  // version for types with begin() and end()
  template<typename T>
  struct IsIterable<T, decltype(Impl::ignore(
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

#ifndef DOXYGEN
  // this is just a forward declaration
  template <class> struct FieldTraits;
#endif

  //! Convenient access to FieldTraits<Type>::field_type.
  template <class Type>
  using field_t = typename FieldTraits<Type>::field_type;

  //! Convenient access to FieldTraits<Type>::real_type.
  template <class Type>
  using real_t = typename FieldTraits<Type>::real_type;


#ifndef DOXYGEN

  // Implementation of IsTuple
  namespace Impl {

  template<class T>
  struct IsTuple : public std::false_type
  {};

  template<class... T>
  struct IsTuple<std::tuple<T...>> : public std::true_type
  {};

  } // namespace Impl

#endif // DOXYGEN

  /**
   * \brief Check if T is a std::tuple<...>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsTuple :
    public Impl::IsTuple<T>
  {};


#ifndef DOXYGEN

  // Implementation of IsTupleOrDerived
  namespace Impl {

  template<class... T, class Dummy>
  std::true_type isTupleOrDerived(const std::tuple<T...>*, Dummy)
  { return {}; }

  template<class Dummy>
  std::false_type isTupleOrDerived(const void*, Dummy)
  { return {}; }

  } // namespace Impl

#endif // DOXYGEN

  /**
   * \brief Check if T derived from a std::tuple<...>
   *
   * The result is exported by deriving from std::true_type or std::false_type.
   */
  template<class T>
  struct IsTupleOrDerived :
    public decltype(Impl::isTupleOrDerived(std::declval<T*>(), true))
  {};


#ifndef DOXYGEN

  // Implementation of is IsIntegralConstant
  namespace Impl {

  template<class T>
  struct IsIntegralConstant : public std::false_type
  {};

  template<class T, T t>
  struct IsIntegralConstant<std::integral_constant<T, t>> : public std::true_type
  {};

  } // namespace Impl

#endif // DOXYGEN

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


#ifndef DOXYGEN

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

#endif // DOXYGEN


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


  /**
   * \brief Get entry of std::integer_sequence
   *
   * Computes the i-th entry of the integer_sequence. The result
   * is exported as ::value by deriving form std::integral_constant<std::size_t, entry>.
   */
  template<class IntegerSequence, std::size_t index>
  struct IntegerSequenceEntry;

#ifndef DOXYGEN

  template<class T, T... t, std::size_t i>
  struct IntegerSequenceEntry<std::integer_sequence<T, t...>, i>
    : public decltype(Impl::IntegerSequenceHelper<T, t...>::get(std::integral_constant<std::size_t, i>()))
  {};

#endif // DOXYGEN

  /**
   * \brief Type free of internal references that `T` can be converted to.
   *
   * This is the specialization point for `AutonomousValue` and `autoCopy()`.
   *
   * If you need to specialize for a proxy type or similar, just specialize
   * for the plain type.  There are already specializations for
   * reference-qualified and cv-qualified types that will just forward to your
   * specialization.
   *
   * \note For all specializations, the member type `type` should be
   *       constructible from `T`.
   */
  template<class T>
  struct AutonomousValueType { using type = T; };

  //! Specialization to remove lvalue references
  template<class T>
  struct AutonomousValueType<T&> : AutonomousValueType<T> {};

  //! Specialization to remove rvalue references
  template<class T>
  struct AutonomousValueType<T&&> : AutonomousValueType<T> {};

  //! Specialization to remove const qualifiers
  template<class T>
  struct AutonomousValueType<const T> : AutonomousValueType<T> {};

  //! Specialization to remove volatile qualifiers
  template<class T>
  struct AutonomousValueType<volatile T> : AutonomousValueType<T> {};

  //! Specialization for the proxies of `vector<bool>`
  template<>
  struct AutonomousValueType<std::vector<bool>::reference>
  {
    using type = bool;
  };

  //! Specialization to remove both const and volatile qualifiers
  template<class T>
  struct AutonomousValueType<volatile const T> : AutonomousValueType<T> {};

  /**
   * \brief Type free of internal references that `T` can be converted to.
   *
   * Specialize `AutonomousValueType` to add your own mapping.  Use
   * `autoCopy()` to convert an expression of type `T` to
   * `AutonomousValue<T>`.
   *
   * This type alias determines a type that `T` can be converted to, but that
   * will be free of references to other objects that it does not manage.  In
   * practice it will act like `std::decay_t`, but in addition to removing
   * references it will also determine the types that proxies stand in for,
   * and the types that expression templates will evaluate to.
   *
   * "Free of references" means that the converted object will always be valid
   * and does not alias any other objects directly or indirectly.  The "other
   * objects that it does not manage" restriction means that the converted
   * object may still contain internal references, but they must be to
   * resources that it manages itself.  So, an `std::vector` would be an
   * autonomous value even though it contains internal references to the
   * storage for the elements since it manages that storage itself.
   *
   * \note For pointers, iterators, and the like the "value" for the purpose
   *       of `AutonomousValue` is considered to be the identity of the
   *       pointed-to object, so that object should not be cloned.  But then
   *       you should hopefully never need an autonomous value for those
   *       anyway...
   */
  template<class T>
  using AutonomousValue = typename AutonomousValueType<T>::type;

  /**
   * \brief Autonomous copy of an expression's value for use in `auto` type
   *        deduction
   *
   * This function is an unproxyfier or an expression evaluator or a fancy
   * cast to ensure an expression can be used in `auto` type deduction.  It
   * ensures two things:
   *
   *   1. The return value is a prvalue,
   *   2. the returned value is self-sufficient, or "autonomous".
   *
   * The latter means that there will be no references into other objects
   * (like containers) which are not guaranteed to be kept alive during the
   * lifetime of the returned value.
   *
   *  An example usage would be
   * ```c++
   *   std::vector<bool> bitvector{24};
   *   auto value = autoCopy(bitvector[23]);
   *   bitvector.resize(42);
   *   // value still valid
   * ```
   * Since `vector<bool>` may use proxies, `auto value = bitvector[23];` would
   * mean that the type of `value` is such a proxy.  The proxy keeps internal
   * references into the vector, and thus can be invalidated by anything that
   * modifies the vector -- such as a later call to `resize()`.  `autoCopy()`
   * lets you work around that problem by copying the value referenced by the
   * proxy and converting it to a `bool`.
   *
   * Another example would be an automatic differentiation library that lets
   * you track the operations in a computation, and later ask for derivatives.
   * Imagine that your operation involves a parameter function, and you want
   * to use that function both with plain types and with automatic
   * differentiation types.  You might write the parameter function as
   * follows:
   * ```c++
   *   template<class NumberType>
   *   auto param(NumberType v)
   *   {
   *     return 2*v;
   *   }
   * ```
   * If the automatic differentiation library is Adept, this would lead to
   * use-after-end-of-life-bugs.  The reason is that for efficiency reasons
   * Adept does not immediately evaluate the expression, but instead it
   * constructs an expression object that records the kind of expression and
   * references to the operands.  The expression object is only evaluated when
   * it is assigned to an object of some number type -- which will only happen
   * after the operands (`v` and the temporary object representing `2`) have
   * gone out of scope and been destroyed.  Basically, Adept was invented
   * before `auto` and rvalue-references were a thing.
   *
   * This can be handled with `autoCopy()`:
   * ```c++
   *   template<class NumberType>
   *   auto param(NumberType v)
   *   {
   *     return autoCopy(2*v);
   *   }
   * ```
   * Of course, `autoCopy()` needs to be taught about the expression
   * objects of Adept for this to work.
   *
   * `autoCopy()` will by default simply return the argument as a prvalue of
   * the same type with cv-qualifiers removed.  This involves one or more
   * copy/move operation, so it will only work with types that are in fact
   * copyable.  And it will incur one copy if the compiler cannot use a move,
   * such as when the type of the expression is a `std::array` or a
   * `FieldMatrix`.  (Any second copy that may semantically be necessary will
   * be elided.)
   *
   * To teach `autoCopy()` about a particular proxy type, specialize
   * `Dune::AutonomousValueType`.
   *
   * \note Do not overload `Dune::autoCopy()` directly.  It is meant to be
   *       found by unqualified or qualified lookup, not by ADL.  There is
   *       little guarantee that your overload will be declared before the
   *       definition of internal Dune functions that use `autoCopy()`.  They
   *       would need the lazy binding provided by ADL to find your overload,
   *       but they will probably use unqualified lookup.
   */
  template<class T>
  constexpr AutonomousValue<T> autoCopy(T &&v)
  {
    return v;
  }

  /** @} */
}
#endif
