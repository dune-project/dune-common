// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_QUADMATH_HH
#define DUNE_COMMON_QUADMATH_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_QUADMATH
#include <cmath>
#include <iostream>
#include <string>
#include <quadmath.h>

#include <dune/common/exceptions.hh>
#include <dune/common/std/cmath.hh>

namespace Dune
{
  /// \brief An overload of output ostreams for `__float128` types.
  template<typename CharT, typename Traits>
  std::basic_ostream<CharT,Traits>& operator<< (std::basic_ostream<CharT,Traits>& out,
                                                const __float128& value)
  {
    const std::size_t bufSize = 128;
    CharT buf[128];

    std::string format = "%." + std::to_string(out.precision()) + "Q" +
                          ((out.flags() | std::ios_base::scientific) ? "e" : "f");
    const int numChars = quadmath_snprintf(buf, bufSize, format.c_str(), value);
    if (std::size_t(numChars) >= bufSize) {
      DUNE_THROW(Dune::RangeError, "Failed to print __float128 value: buffer overflow");
    }
    out << buf;
    return out;
  }

  /// \brief An overload of output streams for `std::complex<__float128>` types.
  template<typename CharT, class Traits>
  std::basic_ostream<CharT, Traits>&
  operator<<(std::basic_ostream<CharT, Traits>& os, const std::complex<__float128>& x)
  {
    std::basic_ostringstream<CharT, Traits> s;
    s.flags(os.flags());
    s.imbue(os.getloc());
    s.precision(os.precision());
    s << '(' << x.real() << ',' << x.imag() << ')';
    return os << s.str();
  }


  /// \brief An overload of input streams for `__float128` types.
  template<typename CharT, typename Traits>
  std::basic_istream<CharT,Traits>& operator>> (std::basic_istream<CharT,Traits>& in,
                                                __float128& value)
  {
    std::string buf;
    buf.reserve(128);
    in >> buf;
    value = strtoflt128(buf.c_str(), NULL);
    return in;
  }

  /// \brief An overload of input streams for `std::complex<__float128>` types.
  template<typename CharT, class Traits>
  std::basic_istream<CharT, Traits>& operator>> (std::basic_istream<CharT, Traits>& in, std::complex<__float128>& x)
  {
    __float128 re_x, im_x;
    CharT ch = CharT();
    in >> ch;
    if (ch == '(') {
      in >> re_x >> ch;
      if (ch == ',') {
        in >> im_x >> ch;
        if (ch == ')')
          x = std::complex<__float128>(re_x, im_x);
        else
          in.setstate(std::ios_base::failbit);
      }
      else if (ch == ')')
        x = re_x;
      else
        in.setstate(std::ios_base::failbit);
    }
    else if (in) {
      in.putback(ch);
      if (in >> re_x)
        x = re_x;
      else
        in.setstate(std::ios_base::failbit);
    }
    return in;
  }
}

namespace std
{
#if defined(__STRICT_ANSI__) || !defined(_GLIBCXX_USE_FLOAT128)
  // Strictly speaking, specializing std library type-traits is undefined behavior,
  // but the following type-traits are already specialized in the libstdc++ implementation
  // if some flags are set. Thus it should be save.

#ifndef NO_STD_TYPE_TRAITS_SPECIALIZATION
  template <>
  struct is_floating_point<__float128>
      : public true_type { };

  template <>
  struct is_floating_point<const __float128>
      : public true_type { };

  template <>
  struct is_floating_point<volatile __float128>
      : public true_type { };
#endif
#endif


#ifndef NO_STD_NUMERIC_LIMITS_SPECIALIZATION
  template <>
  class numeric_limits<__float128>
  {
  public:
    static const bool is_specialized = true;
    static __float128 min() { return FLT128_MIN; }
    static __float128 max() { return FLT128_MAX; }
    static __float128 lowest() { return -FLT128_MAX; }
    static const int digits = FLT128_MANT_DIG;
    static const int digits10 = FLT128_DIG;
    static const int max_digits10 = 0;
    static const bool is_signed = true;
    static const bool is_integer = false;
    static const bool is_exact = true;
    static const int radix = 2;
    static __float128 epsilon() { return FLT128_EPSILON; }
    static __float128 round_error() { return __float128{0.5}; }
    static const int min_exponent = FLT128_MIN_EXP;
    static const int min_exponent10 = FLT128_MIN_10_EXP;
    static const int max_exponent = FLT128_MAX_EXP;
    static const int max_exponent10 = FLT128_MAX_10_EXP;
    static const bool has_infinity = false;
    static const bool has_quiet_NaN = true;
    static const bool has_signaling_NaN = false;
    static const float_denorm_style has_denorm = denorm_present;
    static const bool has_denorm_loss = false;
    static __float128 infinity() { return __float128{}; }
    static __float128 quiet_NaN() { return nanq(""); }
    static __float128 signaling_NaN() { return __float128{}; }
    static __float128 denorm_min() { return FLT128_DENORM_MIN; }
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
#endif // DUNE_COMMON_QUADMATH_HH
