// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_BIGFLOAT_HH
#define DUNE_COMMON_BIGFLOAT_HH

/** \file
 * \brief Wrapper for the GNU MPF(R) multi-precision floating point library
 */

#if HAVE_MPFR
// The BigFloat type requires MPFR to be found and enabled. Use find_package(MPFR) and
// add_dune_mpfr_flags(target) in CMake to activate this package on your target.

#include <compare>
#include <limits>
#include <type_traits>

#include <mpreal.h>

#include <dune/common/math.hh>
#include <dune/common/promotiontraits.hh>
#include <dune/common/quadmath.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/std/assume.hh>

namespace Dune
{
  /**
   * \ingroup Numbers
   * \brief Number class for high precision floating point number using the MPF(R) library mpreal implementation
   */
  template< unsigned int precision >
  class BigFloat
    : public mpfr::mpreal
  {
    using Base = mpfr::mpreal;
    using Prec = mp_prec_t;

    static_assert(precision > 0 && precision < std::numeric_limits<Prec>::max());

  public:
    //! default constructor, initialize to zero.
    BigFloat () noexcept
      : Base(0, Prec(precision))
    {}

    /**
     * \brief constructor from pointer to null-terminated byte string
     * \note this is the only reliable way to initialize with higher precision values
     */
    explicit BigFloat (const char* str)
      : Base(str, Prec(precision))
    {}

    //! copy-construct from mpreal value.
    BigFloat (const Base& v) noexcept
      : Base(v)
    {}

    //! move-construct from mpreal value.
    BigFloat (Base&& v) noexcept
      : Base(std::move(v))
    {}

    //! initialize from any floating-point or integer type
    template <class T,
      std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    BigFloat (const T& v) noexcept
      : Base(v, Prec(precision))
    {}

#if HAVE_QUADMATH
    //! initialize from Float128 type
    BigFloat (const Dune::Float128& v)
      : Base([](const Dune::Float128& v) {
          std::ostringstream oss; oss << v;
          return oss.str();
        }(v), Prec(precision))
    {}
#endif

    BigFloat (const BigFloat&) = default;
    BigFloat (BigFloat&&) = default;

    //! assignment from floating-point or integer type
    template <class T,
      std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    BigFloat& operator= (const T& v) noexcept
    {
      static_cast<Base&>(*this) = v;
      return *this;
    }

#if HAVE_QUADMATH
    //! assignment from Float128 type
    BigFloat& operator= (const Dune::Float128& v)
    {
      std::ostringstream oss; oss << v;
      static_cast<Base&>(*this) = oss.str();
      return *this;
    }
#endif

    BigFloat& operator= (const BigFloat&) = default;
    BigFloat& operator= (BigFloat&&) = default;

    //! equality of two BigFloat numbers, uses IEEE semantics (NaN != NaN)
    template <unsigned int p1, unsigned int p2>
    friend bool operator== (const BigFloat<p1>& A, const BigFloat<p2>& B)
    {
      const Base& a = static_cast<const Base&>(A);
      const Base& b = static_cast<const Base&>(B);
      return a == b;
    }

    //! three-way comparison of BigFloat numbers implements a partial ordering, due to NaN values
    template <unsigned int p1, unsigned int p2>
    friend std::partial_ordering operator<=> (const BigFloat<p1>& A, const BigFloat<p2>& B)
    {
      const Base& a = static_cast<const Base&>(A);
      const Base& b = static_cast<const Base&>(B);
      if (isnan(a) || isnan(b))
        return std::partial_ordering::unordered;
      if (a < b)
        return std::partial_ordering::less;
      if (a > b)
        return std::partial_ordering::greater;
      return std::partial_ordering::equivalent;
    }

    //! Explicit conversion into floating-point or integer type
    template <class T,
      std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    explicit operator T () const
    {
#ifdef MPREAL_HAVE_EXPLICIT_CONVERTERS
      return T(static_cast<const Base&>(*this));
#else
      if constexpr(std::is_same_v<T,bool>)
        return this->toBool();
      else if constexpr(std::is_same_v<T,long long>)
        return this->toLLong();
      else if constexpr(std::is_same_v<T,unsigned long long>)
        return this->toULLong();
      else if constexpr(std::is_signed_v<T>)  // e.g. long
        return T(this->toLong());
      else if constexpr(std::is_unsigned_v<T>)  // e.g. unsigned long
        return T(this->toULong());
      else if constexpr(std::is_same_v<T,float>)
        return this->toFloat();
      else if constexpr(std::is_same_v<T,double>)
        return this->toDouble();
      else if constexpr(std::is_floating_point_v<T>) // e.g. long double
        return T(this->toLDouble());
      else {
        DUNE_ASSUME(false);
        return T{};
      }
#endif
    }

#if HAVE_QUADMATH
    //! Explicit conversion into Float128 type
    explicit operator Dune::Float128 () const
    {
      const int n = std::numeric_limits<Dune::Impl::Float128>::max_digits10;
      return Float128(this->toString(n,10).c_str());
    }
#endif
  };

} // end namespace Dune


namespace Dune
{
  template <unsigned int precision>
  struct IsNumber<BigFloat<precision>>
    : public std::true_type {};

  template <unsigned int precision1, unsigned int precision2>
  struct PromotionTraits<BigFloat<precision1>, BigFloat<precision2>>
  {
    using PromotedType = BigFloat<(precision1 > precision2 ? precision1 : precision2)>;
  };

  template <unsigned int precision>
  struct PromotionTraits<BigFloat<precision>,BigFloat<precision>>
  {
    using PromotedType = BigFloat<precision>;
  };

  template <unsigned int precision, class T>
  struct PromotionTraits<BigFloat<precision>, T>
  {
    using PromotedType = BigFloat<std::max<unsigned int>(8*sizeof(T), precision)>;
  };

  template <class T, unsigned int precision>
  struct PromotionTraits<T, BigFloat<precision>>
  {
    using PromotedType = BigFloat<std::max<unsigned int>(8*sizeof(T), precision)>;
  };


  template< unsigned int precision >
  struct MathematicalConstants<BigFloat<precision>>
  {
    using T = BigFloat<precision>;
    static const T e ()
    {
      return exp(T(1));
    }

    static const T pi ()
    {
      return mpfr::const_pi(mp_prec_t(precision));
    }
  };

} // end namespace Dune

namespace std
{
  template <unsigned int precision>
  inline void swap (Dune::BigFloat<precision>& x, Dune::BigFloat<precision>& y)
  {
    return mpfr::swap(x, y);
  }

  //! Specialization of numeric_limits for known precision width
  template <unsigned int precision>
  class numeric_limits<Dune::BigFloat<precision>>
      : public numeric_limits<mpfr::mpreal>
  {
    using type = Dune::BigFloat<precision>;
    using Base = numeric_limits<mpfr::mpreal>;

    static constexpr int bits2digits (int prec)
    {
      constexpr double LOG10_2 = 0.301029995663981195213738894724493;
      return int(prec * LOG10_2);
    }

  public:
    inline static type min () { return mpfr::minval(precision); }
    inline static type max () {  return  mpfr::maxval(precision); }
    inline static type lowest () { return -mpfr::maxval(precision); }
    inline static type epsilon () { return  mpfr::machine_epsilon(precision); }
    inline static type round_error () { return Base::round_error(precision); }

    static constexpr int digits = int(precision);
    static constexpr int digits10 = bits2digits(precision);
    static constexpr int max_digits10 = bits2digits(precision);
  };

} // end namespace std

#else // HAVE_MPFR

#include <dune/common/typetraits.hh>

namespace Dune
{
  template< unsigned int precision >
  class BigFloat
  {
    static_assert(AlwaysFalse<BigFloat<precision>>::value,
      "The BigFloat type requires MPFR to be found and enabled. Use find_package(MPFR) and add_dune_mpfr_flags(target) in CMake to activate this package on your target.");
  };

} // end namespace std

#endif // HAVE_MPFR
#endif // DUNE_COMMON_BIGFLOAT_HH
