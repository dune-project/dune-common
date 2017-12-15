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
      template<class U>
      static auto test(U* u) -> decltype(u->real(), u->imag(), std::true_type());

      template<class U>
      static auto test(...) -> decltype(std::false_type());

    public:
      static const bool value = decltype(test<T>(nullptr))::value;
  };

  namespace MathOverloads {
    //necessary to find the following overloads in this namespace
    struct ADLTag {};

    /* Default implementation, only applicable if isNaN is defined for T.
       Otherwise, SFINAE prevents the overload from being considered.*/
    template<class T>
    auto isNaN(const T &t, PriorityTag<1>, ADLTag) -> decltype(isNaN(t)) {
      return isNaN(t);
    }

    /* last resort: rerouting to std::isnan and potential customizations*/
    template<class T>
    auto isNaN(const T &t, PriorityTag<0>, ADLTag) {
      using std::isnan;
      return isnan(t);
    }

  } //MathOverloads

  namespace MathImpl {
    struct isNaNImpl {
      template<class T>
      constexpr auto operator()(const T &t) const {
        return isNaN(t, PriorityTag<10>{}, MathOverloads::ADLTag{});
      }
    };
  } //MathImpl


  /* We need to make sure a call to Dune::isNaN(t) leads to a call to the
     functor defined in the MathImpl namespace above. Defining the functor
     in the Dune namespace would yield ambiguousity. Therefore we need to
     pass the Dune::isNaN() call which also allows us to simply call
     Dune::isNaN() without actually creating a Dune::isNaN functor everytime */
  template<class T>
  struct MathDummy
  {
    static constexpr T value{};
  };

  template<class T>
  constexpr T MathDummy<T>::value;

  namespace {
    constexpr auto const &isNaN = MathDummy<MathImpl::isNaNImpl>::value;
  }

  namespace MathOverloads {
    /**Overload for complex types*/
    template<class T, class = std::enable_if_t<isComplexLike<T>::value> >
    auto isNaN(const T &t, PriorityTag<2>, ADLTag) {
      using Dune::isNaN;
      return isNaN(real(t)) || isNaN(imag(t));
    }
  } //MathOverloads
}

#endif // #ifndef DUNE_MATH_HH
