// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MATH_HH
#define DUNE_MATH_HH

/** \file
 * \brief Some useful basic math stuff
 */

#include <cmath>
#include <complex>

#include <dune/common/typeutilities.hh>

namespace Dune
{

  /**
     \brief Provides commonly used mathematical constants.

     a struct that is specialized for types repesenting real or complex
     numbers. I provides commonly used mathematical constants with the
     required accuary for the specified type.
   */
  template< class Field >
  struct MathematicalConstants;

  /**
     \brief Standard implementation of MathematicalConstants.

     This implementation will work with all built-in floating point
     types. It provides

   * e as std::exp(1.0)
   * pi as std::acos(-1.0)

   */
  template< class T >
  struct StandardMathematicalConstants
  {
    static const T e ()
    {
      using std::exp;
      static const T e = exp( T( 1 ) );
      return e;
    }

    static const T pi ()
    {
      using std::acos;
      static const T pi = acos( T( -1 ) );
      return pi;
    }
  };


#ifndef DOXYGEN
  // MathematicalConstants for float
  // -------------------------------

  template<>
  struct MathematicalConstants< float >
    : public StandardMathematicalConstants< float >
  {};



  // MathematicalConstants for double
  // --------------------------------

  template<>
  struct MathematicalConstants< double >
    : public StandardMathematicalConstants< double >
  {};



  // MathematicalConstants for long double
  // -------------------------------------

  template<>
  struct MathematicalConstants< long double >
    : public StandardMathematicalConstants< long double >
  {};
#endif // DOXYGEN


  //! Calculates the factorial of m at compile time
  template <int m>
  struct Factorial
  {
    //! factorial stores m!
    enum { factorial = m * Factorial<m-1>::factorial };
  };

  //! end of recursion of factorial via specialization
  template <>
  struct Factorial<0>
  {
    // 0! = 1
    enum { factorial = 1 };
  };

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


  /** Returns whether a given type behaves like std::complex<>, i.e. whether
      real() and imag() are defined*/
  template<class T>
  struct isComplexLike {
    private:
      //template<class U>
      static auto test(T* u) -> decltype(u->real(), u->imag(), std::true_type());

      //template<class U>
      static auto test(...) -> decltype(std::false_type());

    public:
      static const bool value = decltype(test(0))::value;
  };

  namespace MathOverloads {
    //necessary to find the following overloads in this namespace
    struct ADLTag {};

    /*  <1> provides a default implementation, only applicable if isNaN
            is defined for T. Otherwise, SFINAE prevents the overload
            from being considered.
        <0> provides a call to the corresponding function in the std namespace
            and potential customizations */
#define DUNE_COMMON_MATH_ISFUNCTION(function, stdfunction)  \
    template<class T>                                       \
    auto function(const T &t, PriorityTag<1>, ADLTag)       \
                  -> decltype(function(t)) {                \
      return function(t);                                   \
    }                                                       \
    template<class T>                                       \
    auto function(const T &t, PriorityTag<0>, ADLTag) {     \
      using std::stdfunction;                               \
      return stdfunction(t);                                \
    }

    DUNE_COMMON_MATH_ISFUNCTION(isNaN,isnan);
    DUNE_COMMON_MATH_ISFUNCTION(isInf,isinf);
    DUNE_COMMON_MATH_ISFUNCTION(isFinite,isfinite);
#undef DUNE_COMMON_MATH_ISFUNTION

    template<class T>
    auto isUnordered(const T &t1, const T &t2, PriorityTag<1>, ADLTag)
                  -> decltype(isUnordered(t1, t2)) {
      return isUnordered(t);
    }

    template<class T>
    auto isUnordered(const T &t1, const T &t2, PriorityTag<0>, ADLTag) {
      using std::isunordered;
      return isunordered(t);
    }

  namespace MathImpl {

#define DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(function)                     \
    struct function##Impl {                                              \
      template<class T>                                                  \
      constexpr auto operator()(const T &t) const {                      \
        return function(t, PriorityTag<10>{}, MathOverloads::ADLTag{});  \
      }                                                                  \
    };

    DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(isNaN);
    DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(isInf);
    DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR(isFinite);
#undef DUNE_COMMON_MATH_ISFUNCTION_FUNCTOR

    struct isUnorderedImpl {
      template<class T>
      constexpr auto operator()(const T &t1, const T &t2) const {
        return isUnordered(t, PriorityTag<10>{}, MathOverloads::ADLTag{});
      }
    };

  } //MathImpl


  /* We need to make sure a call to Dune::is*(t) leads to a call to the
     functor defined in the MathImpl namespace above. Defining the functor
     in the Dune namespace would yield ambiguousity. Therefore we need to
     pass the Dune::is*() call which also allows us to simply call
     Dune::is*() without actually creating a Dune::is* functor everytime */
  template<class T>
  struct MathDummy
  {
    static constexpr T value{};
  };

  namespace {
    constexpr auto const &isNaN = MathDummy<MathImpl::isNaNImpl>::value;
    constexpr auto const &isInf = MathDummy<MathImpl::isInfImpl>::value;
    constexpr auto const &isFinite = MathDummy<MathImpl::isFiniteImpl>::value;
    constexpr auto const &isUnordered = MathDummy<MathImpl::isUnorderedImpl>::value;
  }

  namespace MathOverloads {
    /**Overloads for complex types*/
    template<class T, class = std::enable_if_t<isComplexLike<T>::value> >
    auto isNaN(const T &t, PriorityTag<2>, ADLTag) {
      using Dune::isNaN;
      return isNaN(real(t)) || isNaN(imag(t));
    }

    template<class T, class = std::enable_if_t<isComplexLike<T>::value> >
    auto isInf(const T &t, PriorityTag<2>, ADLTag) {
      using Dune::isInf;
      return isInf(real(t)) || isInf(imag(t));
    }

    template<class T, class = std::enable_if_t<isComplexLike<T>::value> >
    auto isFinite(const T &t, PriorityTag<2>, ADLTag) {
      using Dune::isFinite;
      return isFinite(real(t)) && isFinite(imag(t));
    }
  } //MathOverloads
}

#endif // #ifndef DUNE_MATH_HH
