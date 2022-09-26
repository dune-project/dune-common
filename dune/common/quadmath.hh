// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_QUADMATH_HH
#define DUNE_QUADMATH_HH

#if HAVE_QUADMATH
#include <quadmath.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib> // abs
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>

#include <dune/common/exceptions.hh>
#include <dune/common/typetraits.hh>

namespace Dune
{
  namespace Impl
  {
    // forward declaration
    class Float128;

  } // end namespace Impl

  using Impl::Float128;

  // The purpose of this namespace is to move the `<cmath>` function overloads
  // out of namespace `Dune`, see AlignedNumber in debugalign.hh.
  namespace Impl
  {
    using float128_t = __float128;

    /// Wrapper for quad-precision type __float128
    class Float128
    {
      float128_t value_ = 0.0q;

    public:
      constexpr Float128() = default;
      constexpr Float128(const float128_t& value) noexcept
        : value_(value)
      {}

      // constructor from any floating-point or integer type
      template <class T,
        std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
      constexpr Float128(const T& value) noexcept
        : value_(value)
      {}

      // constructor from pointer to null-terminated byte string
      explicit Float128(const char* str) noexcept
        : value_(strtoflt128(str, NULL))
      {}

      // accessors
      constexpr operator float128_t() const noexcept { return value_; }

      constexpr float128_t const& value() const noexcept { return value_; }
      constexpr float128_t&       value() noexcept       { return value_; }

      // I/O
      template<class CharT, class Traits>
      friend std::basic_istream<CharT, Traits>&
      operator>>(std::basic_istream<CharT, Traits>& in, Float128& x)
      {
        std::string buf;
        buf.reserve(128);
        in >> buf;
        x.value() = strtoflt128(buf.c_str(), NULL);
        return in;
      }

      template<class CharT, class Traits>
      friend std::basic_ostream<CharT, Traits>&
      operator<<(std::basic_ostream<CharT, Traits>& out, const Float128& x)
      {
        const std::size_t bufSize = 128;
        CharT buf[128];

        std::string format = "%." + std::to_string(out.precision()) + "Q" +
                              ((out.flags() | std::ios_base::scientific) ? "e" : "f");
        const int numChars = quadmath_snprintf(buf, bufSize, format.c_str(), x.value());
        if (std::size_t(numChars) >= bufSize) {
          DUNE_THROW(Dune::RangeError, "Failed to print Float128 value: buffer overflow");
        }
        out << buf;
        return out;
      }

      // Increment, decrement
      constexpr Float128& operator++() noexcept { ++value_; return *this; }
      constexpr Float128& operator--() noexcept { --value_; return *this; }

      constexpr Float128 operator++(int) noexcept { Float128 tmp{*this}; ++value_; return tmp; }
      constexpr Float128 operator--(int) noexcept { Float128 tmp{*this}; --value_; return tmp; }

      // unary operators
      constexpr Float128 operator+() const noexcept { return Float128{+value_}; }
      constexpr Float128 operator-() const noexcept { return Float128{-value_}; }

      // assignment operators
#define DUNE_ASSIGN_OP(OP)                                              \
      constexpr Float128& operator OP(const Float128& u) noexcept       \
      {                                                                 \
        value_ OP float128_t(u);                                        \
        return *this;                                                   \
      }                                                                 \
      static_assert(true, "Require semicolon to unconfuse editors")

      DUNE_ASSIGN_OP(+=);
      DUNE_ASSIGN_OP(-=);

      DUNE_ASSIGN_OP(*=);
      DUNE_ASSIGN_OP(/=);

#undef DUNE_ASSIGN_OP

    }; // end class Float128

    // binary operators:
    // For symmetry provide overloads with arithmetic types
    // in the first or second argument.
#define DUNE_BINARY_OP(OP)                                              \
    constexpr Float128 operator OP(const Float128& t,                   \
                                   const Float128& u) noexcept          \
    {                                                                   \
      return Float128{float128_t(t) OP float128_t(u)};                  \
    }                                                                   \
    constexpr Float128 operator OP(const float128_t& t,                 \
                                   const Float128& u) noexcept          \
    {                                                                   \
      return Float128{t OP float128_t(u)};                              \
    }                                                                   \
    constexpr Float128 operator OP(const Float128& t,                   \
                                   const float128_t& u) noexcept        \
    {                                                                   \
      return Float128{float128_t(t) OP u};                              \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    constexpr Float128 operator OP(const T& t,                          \
                                   const Float128& u) noexcept          \
    {                                                                   \
      return Float128{float128_t(t) OP float128_t(u)};                  \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    constexpr Float128 operator OP(const Float128& t,                   \
                                   const U& u) noexcept                 \
    {                                                                   \
      return Float128{float128_t(t) OP float128_t(u)};                  \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_BINARY_OP(+);
    DUNE_BINARY_OP(-);
    DUNE_BINARY_OP(*);
    DUNE_BINARY_OP(/);

#undef DUNE_BINARY_OP

    // logical operators:
    // For symmetry provide overloads with arithmetic types
    // in the first or second argument.
#define DUNE_BINARY_BOOL_OP(OP)                                         \
    constexpr bool operator OP(const Float128& t,                       \
                               const Float128& u) noexcept              \
    {                                                                   \
      return float128_t(t) OP float128_t(u);                            \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    constexpr bool operator OP(const T& t,                              \
                               const Float128& u) noexcept              \
    {                                                                   \
      return float128_t(t) OP float128_t(u);                            \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    constexpr bool operator OP(const Float128& t,                       \
                               const U& u) noexcept                     \
    {                                                                   \
      return float128_t(t) OP float128_t(u);                            \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_BINARY_BOOL_OP(==);
    DUNE_BINARY_BOOL_OP(!=);
    DUNE_BINARY_BOOL_OP(<);
    DUNE_BINARY_BOOL_OP(>);
    DUNE_BINARY_BOOL_OP(<=);
    DUNE_BINARY_BOOL_OP(>=);

#undef DUNE_BINARY_BOOL_OP

    // Overloads for the cmath functions

    // function with name `name` redirects to quadmath function `func`
#define DUNE_UNARY_FUNC(name,func)                                         \
    inline Float128 name(const Float128& u) noexcept                       \
    {                                                                      \
      return Float128{func (float128_t(u))};                               \
    }                                                                      \
    static_assert(true, "Require semicolon to unconfuse editors")

    // like DUNE_UNARY_FUNC but with custom return type
#define DUNE_CUSTOM_UNARY_FUNC(type,name,func)                             \
    inline type name(const Float128& u) noexcept                           \
    {                                                                      \
      return (type)(func (float128_t(u)));                                 \
    }                                                                      \
    static_assert(true, "Require semicolon to unconfuse editors")

    // redirects to quadmath function with two arguments
#define DUNE_BINARY_FUNC(name,func)                                        \
    inline Float128 name(const Float128& t,                                \
                         const Float128& u) noexcept                       \
    {                                                                      \
      return Float128{func (float128_t(t), float128_t(u))};                \
    }                                                                      \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_UNARY_FUNC(abs, fabsq);
    DUNE_UNARY_FUNC(acos, acosq);
    DUNE_UNARY_FUNC(acosh, acoshq);
    DUNE_UNARY_FUNC(asin, asinq);
    DUNE_UNARY_FUNC(asinh, asinhq);
    DUNE_UNARY_FUNC(atan, atanq);
    DUNE_UNARY_FUNC(atanh, atanhq);
    DUNE_UNARY_FUNC(cbrt, cbrtq);
    DUNE_UNARY_FUNC(ceil, ceilq);
    DUNE_UNARY_FUNC(cos, cosq);
    DUNE_UNARY_FUNC(cosh, coshq);
    DUNE_UNARY_FUNC(erf, erfq);
    DUNE_UNARY_FUNC(erfc, erfcq);
    DUNE_UNARY_FUNC(exp, expq);
    DUNE_UNARY_FUNC(expm1, expm1q);
    DUNE_UNARY_FUNC(fabs, fabsq);
    DUNE_UNARY_FUNC(floor, floorq);
    DUNE_CUSTOM_UNARY_FUNC(int, ilogb, ilogbq);
    DUNE_UNARY_FUNC(lgamma, lgammaq);
    DUNE_CUSTOM_UNARY_FUNC(long long int, llrint, llrintq);
    DUNE_CUSTOM_UNARY_FUNC(long long int, llround, llroundq);
    DUNE_UNARY_FUNC(log, logq);
    DUNE_UNARY_FUNC(log10, log10q);
    DUNE_UNARY_FUNC(log1p, log1pq);
    DUNE_UNARY_FUNC(log2, log2q);
    // DUNE_UNARY_FUNC(logb, logbq); // not available in gcc5
    DUNE_CUSTOM_UNARY_FUNC(long int, lrint, lrintq);
    DUNE_CUSTOM_UNARY_FUNC(long int, lround, lroundq);
    DUNE_UNARY_FUNC(nearbyint, nearbyintq);
    DUNE_BINARY_FUNC(nextafter, nextafterq);
    DUNE_BINARY_FUNC(pow, powq); // overload for integer argument see below
    DUNE_UNARY_FUNC(rint, rintq);
    DUNE_UNARY_FUNC(round, roundq);
    DUNE_UNARY_FUNC(sin, sinq);
    DUNE_UNARY_FUNC(sinh, sinhq);
    DUNE_UNARY_FUNC(sqrt, sqrtq);
    DUNE_UNARY_FUNC(tan, tanq);
    DUNE_UNARY_FUNC(tanh, tanhq);
    DUNE_UNARY_FUNC(tgamma, tgammaq);
    DUNE_UNARY_FUNC(trunc, truncq);

    DUNE_CUSTOM_UNARY_FUNC(bool, isfinite, finiteq);
    DUNE_CUSTOM_UNARY_FUNC(bool, isinf, isinfq);
    DUNE_CUSTOM_UNARY_FUNC(bool, isnan, isnanq);
    DUNE_CUSTOM_UNARY_FUNC(bool, signbit, signbitq);

#undef DUNE_UNARY_FUNC
#undef DUNE_CUSTOM_UNARY_FUNC
#undef DUNE_BINARY_FUNC

    // like DUNE_BINARY_FUNC but provide overloads with arithmetic
    // types in the first or second argument.
#define DUNE_BINARY_ARITHMETIC_FUNC(name,func)                          \
    inline Float128 name(const Float128& t,                             \
                         const Float128& u) noexcept                    \
    {                                                                   \
      return Float128{func (float128_t(t), float128_t(u))};             \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    inline Float128 name(const T& t,                                    \
                         const Float128& u) noexcept                    \
    {                                                                   \
      return Float128{func (float128_t(t), float128_t(u))};             \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    inline Float128 name(const Float128& t,                             \
                         const U& u) noexcept                           \
    {                                                                   \
      return Float128{func (float128_t(t), float128_t(u))};             \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_BINARY_ARITHMETIC_FUNC(atan2,atan2q);
    DUNE_BINARY_ARITHMETIC_FUNC(copysign,copysignq);
    DUNE_BINARY_ARITHMETIC_FUNC(fdim,fdimq);
    DUNE_BINARY_ARITHMETIC_FUNC(fmax,fmaxq);
    DUNE_BINARY_ARITHMETIC_FUNC(fmin,fminq);
    DUNE_BINARY_ARITHMETIC_FUNC(fmod,fmodq);
    DUNE_BINARY_ARITHMETIC_FUNC(hypot,hypotq);
    DUNE_BINARY_ARITHMETIC_FUNC(remainder,remainderq);

#undef DUNE_BINARY_ARITHMETIC_FUNC

    // some more cmath functions with special signature

    inline Float128 fma(const Float128& t, const Float128& u, const Float128& v)
    {
      return Float128{fmaq(float128_t(t),float128_t(u),float128_t(v))};
    }

    inline Float128 frexp(const Float128& u, int* p)
    {
      return Float128{frexpq(float128_t(u), p)};
    }

    inline Float128 ldexp(const Float128& u, int p)
    {
      return Float128{ldexpq(float128_t(u), p)};
    }

    inline Float128 remquo(const Float128& t, const Float128& u, int* quo)
    {
      return Float128{remquoq(float128_t(t), float128_t(u), quo)};
    }

    inline Float128 scalbln(const Float128& u, long int e)
    {
      return Float128{scalblnq(float128_t(u), e)};
    }

    inline Float128 scalbn(const Float128& u, int e)
    {
      return Float128{scalbnq(float128_t(u), e)};
    }

    /// \brief Overload of `pow` function for integer exponents.
    // NOTE: This is much faster than a pow(x, Float128(p)) call
    // NOTE: This is a modified version of boost::math::cstdfloat::detail::pown
    //   (adapted to the type Float128) that is part of the Boost 1.65 Math toolkit 2.8.0
    //   and is implemented by Christopher Kormanyos, John Maddock, and Paul A. Bristow,
    //   distributed under the Boost Software License, Version 1.0
    //   (See http://www.boost.org/LICENSE_1_0.txt)
    template <class Int,
      std::enable_if_t<std::is_integral<Int>::value, int> = 0>
    inline Float128 pow(const Float128& x, const Int p)
    {
      static const Float128 max_value = FLT128_MAX;
      static const Float128 min_value = FLT128_MIN;
      static const Float128 inf_value = float128_t{1} / float128_t{0};

      const bool isneg = (x < 0);
      const bool isnan = (x != x);
      const bool isinf = (isneg ? bool(-x > max_value) : bool(+x > max_value));

      if (isnan) { return x; }
      if (isinf) { return Float128{nanq("")}; }

      const Float128 abs_x = (isneg ? -x : x);
      if (p < Int(0)) {
        if (abs_x < min_value)
          return (isneg ? -inf_value : +inf_value);
        else
          return Float128(1) / pow(x, Int(-p));
      }

      if (p == Int(0)) { return Float128(1); }
      if (p == Int(1)) { return x; }
      if (abs_x > max_value)
        return (isneg ? -inf_value : +inf_value);

      if (p == Int(2)) { return  (x * x); }
      if (p == Int(3)) { return ((x * x) * x); }
      if (p == Int(4)) { const Float128 x2 = (x * x); return (x2 * x2); }

      Float128 result = ((p % Int(2)) != Int(0)) ? x : Float128(1);
      Float128 xn     = x;  // binary powers of x

      Int p2 = p;
      while (Int(p2 /= 2) != Int(0)) {
        xn *= xn;  // Square xn for each binary power

        const bool has_binary_power = (Int(p2 % Int(2)) != Int(0));
        if (has_binary_power)
          result *= xn;
      }

      return result;
    }


  } // end namespace Impl

  template <>
  struct IsNumber<Impl::Float128>
      : public std::true_type {};

} // end namespace Dune

namespace std
{
#ifndef NO_STD_NUMERIC_LIMITS_SPECIALIZATION
  template <>
  class numeric_limits<Dune::Impl::Float128>
  {
    using Float128 = Dune::Impl::Float128;
    using float128_t = Dune::Impl::float128_t;

  public:
    static constexpr bool is_specialized = true;
    static constexpr Float128 min() noexcept { return FLT128_MIN; }
    static constexpr Float128 max() noexcept { return FLT128_MAX; }
    static constexpr Float128 lowest() noexcept { return -FLT128_MAX; }
    static constexpr int digits = FLT128_MANT_DIG;
    static constexpr int digits10 = 34;
    static constexpr int max_digits10 = 36;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr int radix = 2;
    static constexpr Float128 epsilon() noexcept { return FLT128_EPSILON; }
    static constexpr Float128 round_error() noexcept { return float128_t{0.5}; }
    static constexpr int min_exponent = FLT128_MIN_EXP;
    static constexpr int min_exponent10 = FLT128_MIN_10_EXP;
    static constexpr int max_exponent = FLT128_MAX_EXP;
    static constexpr int max_exponent10 = FLT128_MAX_10_EXP;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_present;
    static constexpr bool has_denorm_loss = false;
    static constexpr Float128 infinity() noexcept { return float128_t{1}/float128_t{0}; }
    static Float128 quiet_NaN() noexcept { return nanq(""); }
    static constexpr Float128 signaling_NaN() noexcept { return float128_t{}; }
    static constexpr Float128 denorm_min() noexcept { return FLT128_DENORM_MIN; }
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = false;
    static constexpr bool is_modulo = false;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = round_to_nearest;
  };
#endif
} // end namespace std

#endif // HAVE_QUADMATH
#endif // DUNE_QUADMATH_HH
