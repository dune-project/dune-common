// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FLOAT128_HH
#define DUNE_FLOAT128_HH

#if HAVE_QUADMATH
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib> // abs
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>
#include <quadmath.h>

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
    /// Wrapper for quad-precision type __float128
    class Float128
    {
      __float128 value_ = 0.q;

    public:
      Float128() = default;
      Float128(const __float128& value)
        : value_(value)
      {}

      // constructor from any floating-point or integer type
      template <class T,
        std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
      Float128(const T& number)
        : value_(number)
      {}

      // accessors
      operator __float128() const { return value_; }

      __float128 const& value() const { return value_; }
      __float128&       value()       { return value_; }

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
      Float128& operator++() { ++value_; return *this; }
      Float128& operator--() { --value_; return *this; }

      Float128 operator++(int) { Float128 tmp{*this}; ++value_; return tmp; }
      Float128 operator--(int) { Float128 tmp{*this}; --value_; return tmp; }

      // unary operators
      Float128 operator+() const { return Float128{+value_}; }
      Float128 operator-() const { return Float128{-value_}; }

      // assignment operators
#define DUNE_ASSIGN_OP(OP)                                              \
      Float128& operator OP(const Float128& u)                          \
      {                                                                 \
        value_ OP u.value();                                            \
        return *this;                                                   \
      }                                                                 \
      static_assert(true, "Require semicolon to unconfuse editors")

      DUNE_ASSIGN_OP(+=);
      DUNE_ASSIGN_OP(-=);

      DUNE_ASSIGN_OP(*=);
      DUNE_ASSIGN_OP(/=);
#undef DUNE_ASSIGN_OP
    };

    // binary operators
#define DUNE_BINARY_OP(OP)                                              \
    Float128 operator OP(const Float128& t,                             \
                         const Float128& u)                             \
    {                                                                   \
      return Float128{__float128(t) OP __float128(u)};                  \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    Float128 operator OP(const T& t,                                    \
                         const Float128& u)                             \
    {                                                                   \
      return Float128{__float128(t) OP __float128(u)};                  \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    Float128 operator OP(const Float128& t,                             \
                         const U& u)                                    \
    {                                                                   \
      return Float128{__float128(t) OP __float128(u)};                  \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_BINARY_OP(+);
    DUNE_BINARY_OP(-);
    DUNE_BINARY_OP(*);
    DUNE_BINARY_OP(/);
#undef DUNE_BINARY_OP

#define DUNE_BINARY_BOOL_OP(OP)                                         \
    bool operator OP(const Float128& t,                                 \
                     const Float128& u)                                 \
    {                                                                   \
      return __float128(t) OP __float128(u);                            \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    bool operator OP(const T& t,                                        \
                     const Float128& u)                                 \
    {                                                                   \
      return __float128(t) OP __float128(u);                            \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    bool operator OP(const Float128& t,                                 \
                     const U& u)                                        \
    {                                                                   \
      return __float128(t) OP __float128(u);                            \
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
#define DUNE_UNARY_FUNC(name,func)                                      \
    Float128 name(const Float128& u)                                    \
    {                                                                   \
      return Float128{func(__float128(u))};                             \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

#define DUNE_CUSTOM_UNARY_FUNC(type,name,func)                          \
    type name(const Float128& u)                                        \
    {                                                                   \
      return (type)(func(__float128(u)));                               \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

#define DUNE_BINARY_FUNC(name,func)                                     \
    Float128 name(const Float128& t, const Float128& u)                 \
    {                                                                   \
      return Float128{func(__float128(t), __float128(u))};              \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

#define DUNE_TERTIARY_FUNC(name,func)                                   \
    Float128 name(const Float128&t, const Float128& u, const Float128& v)  \
    {                                                                   \
      return Float128{func(__float128(t),__float128(u),__float128(v))}; \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_UNARY_FUNC(abs, fabsq);
    DUNE_UNARY_FUNC(acos, acosq);
    DUNE_UNARY_FUNC(acosh, acoshq);
    DUNE_UNARY_FUNC(asin, asinq);
    DUNE_UNARY_FUNC(asinh, asinhq);
    DUNE_UNARY_FUNC(atan, atanq);
    DUNE_BINARY_FUNC(atan2, atan2q);
    DUNE_UNARY_FUNC(atanh, atanhq);
    DUNE_UNARY_FUNC(cbrt, cbrtq);
    DUNE_UNARY_FUNC(ceil, ceilq);
    DUNE_BINARY_FUNC(copysign, copysignq);
    DUNE_UNARY_FUNC(cos, cosq);
    DUNE_UNARY_FUNC(cosh, coshq);
    DUNE_UNARY_FUNC(erf, erfq);
    DUNE_UNARY_FUNC(erfc, erfcq);
    DUNE_UNARY_FUNC(exp, expq);
    DUNE_UNARY_FUNC(expm1, expm1q);
    DUNE_UNARY_FUNC(fabs, fabsq);
    DUNE_BINARY_FUNC(fdim, fdimq);
    DUNE_UNARY_FUNC(floor, floorq);
    DUNE_TERTIARY_FUNC(fma, fmaq);
    DUNE_BINARY_FUNC(fmax, fmaxq);
    DUNE_BINARY_FUNC(fmin, fminq);
    DUNE_BINARY_FUNC(fmod, fmodq);
    DUNE_BINARY_FUNC(hypot, hypotq);
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
    DUNE_BINARY_FUNC(pow, powq);
    DUNE_BINARY_FUNC(remainder, remainderq);
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
#undef DUNE_BINARY_FUNC
#undef DUNE_TERTIARY_FUNC
#undef DUNE_CUSTOM_UNARY_FUNC

    Float128 frexp(const Float128& u, int* p)
    {
      return Float128{frexpq(__float128(u),p)};
    }

    Float128 ldexp(const Float128& u, int p)
    {
      return Float128{ldexpq(__float128(u),p)};
    }

    Float128 nan(const char* arg)
    {
      return Float128{nanq(arg)};
    }

    Float128 remquo(const Float128& t, const Float128& u, int* quo)
    {
      return Float128{remquoq(__float128(t),__float128(u),quo)};
    }

    Float128 scalbln(const Float128& u, long int exp)
    {
      return Float128{scalblnq(__float128(u),exp)};
    }

    Float128 scalbn(const Float128& u, int exp)
    {
      return Float128{scalbnq(__float128(u),exp)};
    }

  } // end namespace Impl

  template <>
  struct IsNumber<Impl::Float128>
      : public std::true_type {};

} // namespace Dune

namespace std
{
#ifndef NO_STD_NUMERIC_LIMITS_SPECIALIZATION
  template <>
  class numeric_limits<Dune::Impl::Float128>
  {
    using Float128 = Dune::Impl::Float128;

  public:
    static const bool is_specialized = true;
    static Float128 min() { return FLT128_MIN; }
    static Float128 max() { return FLT128_MAX; }
    static Float128 lowest() { return -FLT128_MAX; }
    static const int digits = FLT128_MANT_DIG;
    static const int digits10 = FLT128_DIG;
    static const int max_digits10 = 0;
    static const bool is_signed = true;
    static const bool is_integer = false;
    static const bool is_exact = true;
    static const int radix = 2;
    static Float128 epsilon() { return FLT128_EPSILON; }
    static Float128 round_error() { return __float128{0.5}; }
    static const int min_exponent = FLT128_MIN_EXP;
    static const int min_exponent10 = FLT128_MIN_10_EXP;
    static const int max_exponent = FLT128_MAX_EXP;
    static const int max_exponent10 = FLT128_MAX_10_EXP;
    static const bool has_infinity = false;
    static const bool has_quiet_NaN = true;
    static const bool has_signaling_NaN = false;
    static const float_denorm_style has_denorm = denorm_present;
    static const bool has_denorm_loss = false;
    static Float128 infinity() { return __float128{}; }
    static Float128 quiet_NaN() { return nanq(""); }
    static Float128 signaling_NaN() { return __float128{}; }
    static Float128 denorm_min() { return FLT128_DENORM_MIN; }
    static const bool is_iec559 = false;
    static const bool is_bounded = false;
    static const bool is_modulo = false;
    static const bool traps = false;
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero;
  };
#endif
} // end namespace std


#endif // HAVE_QUADMATH
#endif // DUNE_FLOAT128_HH
