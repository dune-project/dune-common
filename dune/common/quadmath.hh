// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_QUADMATH_HH
#define DUNE_QUADMATH_HH

#if HAVE_QUADMATH
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib> // abs
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>

#include <dune/common/typetraits.hh>

#if HAVE_INTEL_QUAD
  // For intel quad library the prototypes must be defined manually
  extern "C" {
    _Quad __ldexpq(_Quad, int);
    _Quad __frexpq(_Quad, int*);
    _Quad __fabsq(_Quad);
    _Quad __floorq(_Quad);
    _Quad __ceilq(_Quad);
    _Quad __sqrtq(_Quad);
    _Quad __truncq(_Quad);
    _Quad __expq(_Quad);
    _Quad __powq(_Quad, _Quad);
    _Quad __logq(_Quad);
    _Quad __log10q(_Quad);
    _Quad __sinq(_Quad);
    _Quad __cosq(_Quad);
    _Quad __tanq(_Quad);
    _Quad __asinq(_Quad);
    _Quad __acosq(_Quad);
    _Quad __atanq(_Quad);
    _Quad __sinhq(_Quad);
    _Quad __coshq(_Quad);
    _Quad __tanhq(_Quad);
    _Quad __fmodq(_Quad, _Quad);
    _Quad __atan2q(_Quad, _Quad);
    // TODO: fill in all the missing functions with replacements
  }
  #define DUNE_ADD_QUAD_PREFIX(func) __ # func
#else
  #include <quadmath.h>
  #define DUNE_ADD_QUAD_PREFIX(func) func
#endif

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
  #if HAVE_INTEL_QUAD
    using float128_t = _Quad
  #else
    using float128_t = __float128;
  #endif

    /// Wrapper for quad-precision type __float128
    class Float128
    {
      float128_t value_ = 0.0q;

    public:
      constexpr Float128() = default;
      constexpr Float128(const float128_t& value)
        : value_(value)
      {}

      // constructor from any floating-point or integer type
      template <class T,
        std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
      constexpr Float128(const T& number)
        : value_(number)
      {}

      // accessors
      constexpr operator float128_t() const { return value_; }

      constexpr float128_t const& value() const { return value_; }
      constexpr float128_t&       value()       { return value_; }

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
      constexpr Float128& operator++() { ++value_; return *this; }
      constexpr Float128& operator--() { --value_; return *this; }

      constexpr Float128 operator++(int) { Float128 tmp{*this}; ++value_; return tmp; }
      constexpr Float128 operator--(int) { Float128 tmp{*this}; --value_; return tmp; }

      // unary operators
      constexpr Float128 operator+() const { return Float128{+value_}; }
      constexpr Float128 operator-() const { return Float128{-value_}; }

      // assignment operators
#define DUNE_ASSIGN_OP(OP)                                              \
      constexpr Float128& operator OP(const Float128& u)                \
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

    // binary operators
#define DUNE_BINARY_OP(OP)                                              \
    constexpr Float128 operator OP(const Float128& t,                   \
                                   const Float128& u)                   \
    {                                                                   \
      return Float128{float128_t(t) OP float128_t(u)};                  \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    constexpr Float128 operator OP(const T& t,                          \
                                   const Float128& u)                   \
    {                                                                   \
      return Float128{float128_t(t) OP float128_t(u)};                  \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    constexpr Float128 operator OP(const Float128& t,                   \
                                   const U& u)                          \
    {                                                                   \
      return Float128{float128_t(t) OP float128_t(u)};                  \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_BINARY_OP(+);
    DUNE_BINARY_OP(-);
    DUNE_BINARY_OP(*);
    DUNE_BINARY_OP(/);

#undef DUNE_BINARY_OP

#define DUNE_BINARY_BOOL_OP(OP)                                         \
    constexpr bool operator OP(const Float128& t,                       \
                               const Float128& u)                       \
    {                                                                   \
      return float128_t(t) OP float128_t(u);                            \
    }                                                                   \
    template <class T,                                                  \
      std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>          \
    constexpr bool operator OP(const T& t,                              \
                               const Float128& u)                       \
    {                                                                   \
      return float128_t(t) OP float128_t(u);                            \
    }                                                                   \
    template <class U,                                                  \
      std::enable_if_t<std::is_arithmetic<U>::value, int> = 0>          \
    constexpr bool operator OP(const Float128& t,                       \
                               const U& u)                              \
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
#define DUNE_UNARY_FUNC(name,func)                                      \
    Float128 name(const Float128& u)                                    \
    {                                                                   \
      return { DUNE_ADD_QUAD_PREFIX(func) (float128_t(u))};             \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

#define DUNE_CUSTOM_UNARY_FUNC(type,name,func)                          \
    type name(const Float128& u)                                        \
    {                                                                   \
      return (type)( DUNE_ADD_QUAD_PREFIX(func) (float128_t(u)));       \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

#define DUNE_BINARY_FUNC(name,func)                                     \
    Float128 name(const Float128& t, const Float128& u)                 \
    {                                                                   \
      return { DUNE_ADD_QUAD_PREFIX(func) (float128_t(t), float128_t(u))}; \
    }                                                                   \
    static_assert(true, "Require semicolon to unconfuse editors")

#define DUNE_TERTIARY_FUNC(name,func)                                   \
    Float128 name(const Float128&t, const Float128& u, const Float128& v)  \
    {                                                                   \
      return { DUNE_ADD_QUAD_PREFIX(func) (float128_t(t),float128_t(u),float128_t(v))}; \
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
      return { DUNE_ADD_QUAD_PREFIX(frexpq) (float128_t(u),p)};
    }

    Float128 ldexp(const Float128& u, int p)
    {
      return { DUNE_ADD_QUAD_PREFIX(ldexpq) (float128_t(u),p)};
    }

    Float128 nan(const char* arg)
    {
      return { DUNE_ADD_QUAD_PREFIX(nanq) (arg)};
    }

    Float128 remquo(const Float128& t, const Float128& u, int* quo)
    {
      return { DUNE_ADD_QUAD_PREFIX(remquoq) (float128_t(t),float128_t(u),quo)};
    }

    Float128 scalbln(const Float128& u, long int exp)
    {
      return { DUNE_ADD_QUAD_PREFIX(scalblnq) (float128_t(u),exp)};
    }

    Float128 scalbn(const Float128& u, int exp)
    {
      return { DUNE_ADD_QUAD_PREFIX(scalbnq) (float128_t(u),exp)};
    }

    template <class Scalar>
    Float128 max(const Float128& a, const Scalar& b)
    {
        return std::max(a, Float128(b));
    }

    template <class Scalar>
    Float128 max(const Scalar& a, const Float128& b)
    {
        return std::max(Float128(a), b);
    }

    template <class Scalar>
    Float128 min(const Float128& a, const Scalar& b)
    {
        return std::min(a, Float128(b));
    }

    template <class Scalar>
    Float128 min(const Scalar& a, const Float128& b)
    {
        return std::min(Float128(a), b);
    }

    template <class Scalar>
    Float128 pow(const Float128& base, const Scalar& exp)
    {
        return powq(base, Float128(exp));
    }
  } // end namespace Impl

  template <>
  struct IsNumber<Impl::Float128>
      : public std::true_type {};

} // namespace Dune

namespace std
{
#ifndef NO_STD_NUMERIC_LIMITS_SPECIALIZATION
#ifndef HAVE_INTEL_QUAD
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
    static constexpr int digits10 = FLT128_DIG;
    static constexpr int max_digits10 = 0;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = true;
    static constexpr int radix = 2;
    static constexpr Float128 epsilon() noexcept { return FLT128_EPSILON; }
    static constexpr Float128 round_error() noexcept { return float128_t{0.5}; }
    static constexpr int min_exponent = FLT128_MIN_EXP;
    static constexpr int min_exponent10 = FLT128_MIN_10_EXP;
    static constexpr int max_exponent = FLT128_MAX_EXP;
    static constexpr int max_exponent10 = FLT128_MAX_10_EXP;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_present;
    static constexpr bool has_denorm_loss = false;
    static constexpr Float128 infinity() noexcept { return float128_t{}; }
    static Float128 quiet_NaN() noexcept { return nanq(""); }
    static constexpr Float128 signaling_NaN() noexcept { return float128_t{}; }
    static constexpr Float128 denorm_min() noexcept { return FLT128_DENORM_MIN; }
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = false;
    static constexpr bool is_modulo = false;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = round_toward_zero;
  };
#endif
#endif
} // end namespace std


#endif // HAVE_QUADMATH
#endif // DUNE_QUADMATH_HH
