// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_MATH_HH
#define DUNE_MATH_HH

/** \file
 * \brief Some useful basic math stuff
 */

#include <cmath>
#include <complex>
#include <limits>
#include <type_traits>

#include <dune/common/typeutilities.hh>

namespace Dune
{

  /**
     \brief Standard implementation of MathematicalConstants.

     This implementation will work with all built-in floating point
     types. It provides

   * e as exp(1.0)
   * pi as acos(-1.0)

   */
  template< class T >
  struct StandardMathematicalConstants
  {
    /**
     * \brief Euler's number
     */
    static const T e ()
    {
      using std::exp;
      static const T e = exp( T( 1 ) );
      return e;
    }

    /**
     * \brief Archimedes' constant
     */
    static const T pi ()
    {
      using std::acos;
      static const T pi = acos( T( -1 ) );
      return pi;
    }
  };


  /**
     \brief Provides commonly used mathematical constants.

     a struct that is specialized for types representing real or complex
     numbers. It provides commonly used mathematical constants with the
     required accuracy for the specified type.
   */
  template< class Field >
  struct MathematicalConstants
    : public StandardMathematicalConstants<Field>
  {};


  /** \brief Power method for integer exponents
   *
   * \note Make sure that Base is a non-integer type when using negative exponents!
   */
  template <class Base, class Exponent>
  constexpr Base power(Base m, Exponent p)
  {
    static_assert(std::numeric_limits<Exponent>::is_integer, "Exponent must be an integer type!");

    auto result = Base(1);
    auto absp = (p<0) ? -p : p;   // This is simply abs, but std::abs is not constexpr
    for (Exponent i = Exponent(0); i<absp; i++)
      result *= m;

    if (p<0)
      result = Base(1)/result;

    return result;
  }

  //! Calculates the factorial of m at compile time
  //! \deprecated Will be removed after release 2.9
  template <int m>
  struct Factorial
  {
    //! factorial stores m!
    static constexpr int factorial = m * Factorial<m-1>::factorial;
  };

  //! end of recursion of factorial via specialization
  template <>
  struct [[deprecated("Use function factorial instead! Will be removed after Dune 2.9")]] Factorial<0>
  {
    // 0! = 1
    static constexpr int factorial = 1;
  };


  //! calculate the factorial of n as a constexpr
  // T has to be an integral type
  template<class T>
  constexpr inline static T factorial(const T& n) noexcept
  {
    static_assert(std::numeric_limits<T>::is_integer, "`factorial(n)` has to be called with an integer type.");
    T fac = 1;
    for(T k = 0; k < n; ++k)
      fac *= k+1;
    return fac;
  }

  //! calculate the factorial of n as a constexpr
  template<class T, T n>
  constexpr inline static auto factorial (std::integral_constant<T, n>) noexcept
  {
    return std::integral_constant<T, factorial(n)>{};
  }


  //! calculate the binomial coefficient n over k as a constexpr
  // T has to be an integral type
  template<class T>
  constexpr inline static T binomial (const T& n, const T& k) noexcept
  {
    static_assert(std::numeric_limits<T>::is_integer, "`binomial(n, k)` has to be called with an integer type.");

    if( k < 0 || k > n )
      return 0;

    if (2*k > n)
      return binomial(n, n-k);

    T bin = 1;
    for(auto i = n-k; i < n; ++i)
      bin *= i+1;
    return bin / factorial(k);
  }

  //! calculate the binomial coefficient n over k as a constexpr
  template<class T, T n, T k>
  constexpr inline static auto binomial (std::integral_constant<T, n>, std::integral_constant<T, k>) noexcept
  {
    return std::integral_constant<T, binomial(n, k)>{};
  }

  template<class T, T n>
  constexpr inline static auto binomial (std::integral_constant<T, n>, std::integral_constant<T, n>) noexcept
  {
    return std::integral_constant<T, (n >= 0 ? 1 : 0)>{};
  }


  //! compute conjugate complex of x
  // conjugate complex does nothing for non-complex types
  template<class K>
  inline K conjugateComplex (const K& x)
  {
    return x;
  }

#ifndef DOXYGEN
  // specialization for complex
  template<class K>
  inline std::complex<K> conjugateComplex (const std::complex<K>& c)
  {
    return std::complex<K>(c.real(),-c.imag());
  }
#endif

  //! Return the sign of the value
  template <class T>
  int sign(const T& val)
  {
    return (val < 0 ? -1 : 1);
  }


  namespace Impl {
    // Returns whether a given type behaves like std::complex<>, i.e. whether
    // real() and imag() are defined
    template<class T>
    struct isComplexLike {
      private:
        template<class U>
        static auto test(U* u) -> decltype(u->real(), u->imag(), std::true_type());

        template<class U>
        static auto test(...) -> decltype(std::false_type());

      public:
        static const bool value = decltype(test<T>(0))::value;
    };
  } // namespace Impl

  //! namespace for customization of math functions with Dune-Semantics
  /**
     You can add overloads for the Dune-semantics of math-functions in this
     namespace.  These overloads will be used by functors like `Dune::isNaN`
     to implement these functions, and will be preferred over functions found
     by ADL, or the corresponding functions from the standard (whether they
     are found by ADL or in the namespace `std`.

     PriorityTag
     ===========

     There are two predefined priorities:

     <1> provides a default implementation, only applicable if the
         camelCase-Version of the function (e.g. `isNaN`) can be found via ADL
         for an argument of type `T`.  (Otherwise the overload should not
         participate in overload resolution.)

     <0> provides a default implementation that forwards the call to the
         lower-case version of the function (e.g. `isnan`), found via ADL and
         the namespace `std`.

     Any higher priority up to 10 can be used by other overloads.
    */
  namespace MathOverloads {

    //! Tag to make sure the functions in this namespace can be found by ADL.
    struct ADLTag {};

#define DUNE_COMMON_MATH_ISFUNCTION(function, stdfunction)         \
    template<class T>                                              \
    auto function(const T &t, PriorityTag<1>, ADLTag)              \
                  -> decltype(function(t)) {                       \
      return function(t);                                          \
    }                                                              \
    template<class T>                                              \
    auto function(const T &t, PriorityTag<0>, ADLTag) {            \
      using std::stdfunction;                                      \
      return stdfunction(t);                                       \
    }                                                              \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_COMMON_MATH_ISFUNCTION(isNaN,isnan);
    DUNE_COMMON_MATH_ISFUNCTION(isInf,isinf);
    DUNE_COMMON_MATH_ISFUNCTION(isFinite,isfinite);
#undef DUNE_COMMON_MATH_ISFUNCTION

    template<class T>
    auto isUnordered(const T &t1, const T &t2, PriorityTag<1>, ADLTag)
                  -> decltype(isUnordered(t1, t2)) {
      return isUnordered(t1, t2);
    }

    template<class T>
    auto isUnordered(const T &t1, const T &t2, PriorityTag<0>, ADLTag) {
      using std::isunordered;
      return isunordered(t1, t2);
    }
  }

  namespace MathImpl {

    // NOTE: it is important that these functors have names different from the
    // names of the functions they are forwarding to.  Otherwise the
    // unqualified call would find the functor type, not a function, and ADL
    // would never be attempted.
#define DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(function)                    \
    struct function##Impl {                                              \
      template<class T>                                                  \
      constexpr auto operator()(const T &t) const {                      \
        return function(t, PriorityTag<10>{}, MathOverloads::ADLTag{});  \
      }                                                                  \
    };                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(isNaN);
    DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(isInf);
    DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(isFinite);
#undef DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR

    struct isUnorderedImpl {
      template<class T>
      constexpr auto operator()(const T &t1, const T &t2) const {
        return isUnordered(t1, t2, PriorityTag<10>{}, MathOverloads::ADLTag{});
      }
    };

  } //MathImpl


  namespace Impl {
    /* This helper has a math functor as a static constexpr member.  Doing
       this as a static member of a template struct means we can do this
       without violating the ODR or putting the definition into a separate
       compilation unit, while still still ensuring the functor is the same
       lvalue across all compilation units.
     */
    template<class T>
    struct MathDummy
    {
      static constexpr T value{};
    };

    template<class T>
    constexpr T MathDummy<T>::value;

  } //namespace Impl

  namespace {
    /* Provide the math functors directly in the `Dune` namespace.

       This actually declares a different name in each translation unit, but
       they all resolve to the same lvalue.
    */

    //! check whether the argument is NaN
    /**
     * Dune-Semantic: for multi-valued types (complex, vectors), check whether
     * *any* value is NaN.
     */
    constexpr auto const &isNaN = Impl::MathDummy<MathImpl::isNaNImpl>::value;

    //! check whether the argument is infinite or NaN
    /**
     * Dune-Semantic: for multi-valued types (complex, vectors), check whether
     * *any* value is infinite or NaN.
     */
    constexpr auto const &isInf = Impl::MathDummy<MathImpl::isInfImpl>::value;

    //! check whether the argument is finite and non-NaN
    /**
     * Dune-Semantic: for multi-valued types (complex, vectors), check whether
     * *all* values are finite and non-NaN.
     */
    constexpr auto const &isFinite = Impl::MathDummy<MathImpl::isFiniteImpl>::value;

    //! check whether the arguments are ordered
    /**
     * Dune-Semantic: for multi-valued types (complex, vectors), there is
     * never an ordering, so at the moment these types are not supported as
     * arguments.
     */
    constexpr auto const &isUnordered = Impl::MathDummy<MathImpl::isUnorderedImpl>::value;
  }

  namespace MathOverloads {
    /*Overloads for complex types*/
    template<class T, class = std::enable_if_t<Impl::isComplexLike<T>::value> >
    auto isNaN(const T &t, PriorityTag<2>, ADLTag) {
      return Dune::isNaN(real(t)) || Dune::isNaN(imag(t));
    }

    template<class T, class = std::enable_if_t<Impl::isComplexLike<T>::value> >
    auto isInf(const T &t, PriorityTag<2>, ADLTag) {
      return Dune::isInf(real(t)) || Dune::isInf(imag(t));
    }

    template<class T, class = std::enable_if_t<Impl::isComplexLike<T>::value> >
    auto isFinite(const T &t, PriorityTag<2>, ADLTag) {
      return Dune::isFinite(real(t)) && Dune::isFinite(imag(t));
    }
  } //MathOverloads
}

#endif // #ifndef DUNE_MATH_HH
