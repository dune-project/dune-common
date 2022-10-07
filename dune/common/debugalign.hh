// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_DEBUGALIGN_HH
#define DUNE_DEBUGALIGN_HH

#include <algorithm>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdlib> // abs
#include <functional>
#include <istream>
#include <ostream>
#include <type_traits>
#include <utility>

#include <dune/common/classname.hh>
#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh>
#include <dune/common/typetraits.hh>

namespace Dune {

  //! type of the handler called by `violatedAlignment()`
  using ViolatedAlignmentHandler =
    std::function<void(const char*, std::size_t, const void*)>;

  //! access the handler called by `violatedAlignment()`
  /**
   * This may be used to obtain the handler for the purpose of calling, or for
   * saving it somewhere to restore it later.  It may also be used to set the
   * handler simply by assigning a new handler.  Setting the handler races
   * with other accesses.
   */
  ViolatedAlignmentHandler &violatedAlignmentHandler();

  //! called when an alignment violation is detected
  /**
   * \p className         Name of the class whose alignment was violated
   * \p expectedAlignment The (over-)alignment that the class expected
   * \p address           The address the class actually found itself at.
   *
   * The main purpose of the function is to serve as a convenient breakpoint
   * for debugging -- which is why we put it in an external compilation unit
   * so it isn't inlined.
   */
  void violatedAlignment(const char *className, std::size_t expectedAlignment,
                         const void *address);

  //! check whether an address conforms to the given alignment
  inline bool isAligned(const void *p, std::size_t align)
  {
    // a more portable way to do this would be to abuse std::align(), but that
    // isn't supported by g++-4.9 yet
    return std::uintptr_t(p) % align == 0;
  }

  //! CRTP base mixin class to check alignment
  template<std::size_t align, class Impl>
  class alignas(align) AlignedBase
  {
    void checkAlignment() const
    {
      auto pimpl = static_cast<const Impl*>(this);
      if(!isAligned(pimpl, align))
        violatedAlignment(className<Impl>().c_str(), align, pimpl);
    }
  public:
    AlignedBase()                    { checkAlignment(); }
    AlignedBase(const AlignedBase &) { checkAlignment(); }
    AlignedBase(AlignedBase &&)      { checkAlignment(); }
    ~AlignedBase()                   { checkAlignment(); }

    AlignedBase& operator=(const AlignedBase &) = default;
    AlignedBase& operator=(AlignedBase &&)      = default;
  };

  //! an alignment large enough to trigger alignment errors
  static constexpr auto debugAlignment = 2*alignof(std::max_align_t);

  namespace AlignedNumberImpl {

    template<class T, std::size_t align = debugAlignment>
    class AlignedNumber;

  } // namespace AlignedNumberImpl

  using AlignedNumberImpl::AlignedNumber;

  template<class T, std::size_t align>
  struct IsNumber<AlignedNumberImpl::AlignedNumber<T,align>>
      : public std::true_type {};

  //! align a value to a certain alignment
  template<std::size_t align = debugAlignment, class T>
  AlignedNumber<T, align> aligned(T value) { return { std::move(value) }; }

  // The purpose of this namespace is to move the `<cmath>` function overloads
  // out of namespace `Dune`.  This avoids problems where people called
  // e.g. `sqrt(1.0)` inside the `Dune` namespace, without first doing `using
  // std::sqrt;`.  Without any `Dune::sqrt()`, such a use will find
  // `::sqrt()`, but with `Dune::sqrt()` it will find only `Dune::sqrt()`,
  // which does not have an overload for `double`.
  namespace AlignedNumberImpl {

    //! aligned wrappers for arithmetic types
    template<class T, std::size_t align>
    class AlignedNumber
      : public AlignedBase<align, AlignedNumber<T, align> >
    {
      T value_;

    public:
      AlignedNumber() = default;
      AlignedNumber(T value) : value_(std::move(value)) {}
      template<class U, std::size_t uAlign,
               class = std::enable_if_t<(align >= uAlign) &&
                                        std::is_convertible<U, T>::value> >
      AlignedNumber(const AlignedNumber<U, uAlign> &o) : value_(U(o)) {}

      // accessors
      template<class U,
               class = std::enable_if_t<std::is_convertible<T, U>::value> >
      explicit operator U() const { return value_; }

      const T &value() const { return value_; }
      T &value() { return value_; }

      // I/O
      template<class charT, class Traits>
      friend std::basic_istream<charT, Traits>&
      operator>>(std::basic_istream<charT, Traits>& str, AlignedNumber &u)
      {
        return str >> u.value_;
      }

      template<class charT, class Traits>
      friend std::basic_ostream<charT, Traits>&
      operator<<(std::basic_ostream<charT, Traits>& str,
                 const AlignedNumber &u)
      {
        return str << u.value_;
      }

      // The trick with `template<class U = T, class = std::void_t<expr(U)> >` is
      // needed because at least g++-4.9 seems to evaluates a default argument
      // in `template<class = std::void_t<expr(T))> >` as soon as possible and will
      // error out if `expr(T)` is invalid.  E.g. for `expr(T)` =
      // `decltype(--std::declval<T&>())`, instantiating `AlignedNumber<bool>`
      // will result in an unrecoverable error (`--` cannot be applied to a
      // `bool`).

      // Increment, decrement
      template<class U = T, class = std::void_t<decltype(++std::declval<U&>())> >
      AlignedNumber &operator++() { ++value_; return *this; }

      template<class U = T, class = std::void_t<decltype(--std::declval<U&>())> >
      AlignedNumber &operator--() { --value_; return *this; }

      template<class U = T, class = std::void_t<decltype(std::declval<U&>()++)> >
      decltype(auto) operator++(int) { return aligned<align>(value_++); }

      template<class U = T, class = std::void_t<decltype(std::declval<U&>()--)> >
      decltype(auto) operator--(int) { return aligned<align>(value_--); }

      // unary operators
      template<class U = T,
               class = std::void_t<decltype(+std::declval<const U&>())> >
      decltype(auto) operator+() const { return aligned<align>(+value_); }

      template<class U = T,
               class = std::void_t<decltype(-std::declval<const U&>())> >
      decltype(auto) operator-() const { return aligned<align>(-value_); }

      /*
       * silence warnings from GCC about using `~` on a bool
       * (when instantiated for T=bool)
       */
#if __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wbool-operation"
#endif
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wbool-operation"
#endif
      template<class U = T,
               class = std::void_t<decltype(~std::declval<const U&>())> >
      decltype(auto) operator~() const { return aligned<align>(~value_); }
#if __GNUC__ >= 7
#  pragma GCC diagnostic pop
#endif
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

      template<class U = T,
               class = std::void_t<decltype(!std::declval<const U&>())> >
      decltype(auto) operator!() const { return aligned<align>(!value_); }

      // assignment operators
#define DUNE_ASSIGN_OP(OP)                                              \
      template<class U, std::size_t uAlign,                             \
               class = std::enable_if_t<                                \
                         ( uAlign <= align &&                           \
                           sizeof(std::declval<T&>() OP std::declval<U>()) ) \
                           > >                                          \
      AlignedNumber &operator OP(const AlignedNumber<U, uAlign> &u)     \
      {                                                                 \
        value_ OP U(u);                                                 \
        return *this;                                                   \
      }                                                                 \
                                                                        \
      template<class U,                                                 \
               class = std::void_t<decltype(std::declval<T&>() OP       \
                                            std::declval<U>())> >       \
      AlignedNumber &operator OP(const U &u)                            \
      {                                                                 \
        value_ OP u;                                                    \
        return *this;                                                   \
      }                                                                 \
                                                                        \
      static_assert(true, "Require semicolon to unconfuse editors")

      DUNE_ASSIGN_OP(+=);
      DUNE_ASSIGN_OP(-=);

      DUNE_ASSIGN_OP(*=);
      DUNE_ASSIGN_OP(/=);
      DUNE_ASSIGN_OP(%=);

      DUNE_ASSIGN_OP(^=);
      DUNE_ASSIGN_OP(&=);
      DUNE_ASSIGN_OP(|=);

      DUNE_ASSIGN_OP(<<=);
      DUNE_ASSIGN_OP(>>=);

#undef DUNE_ASSIGN_OP
    };

    // binary operators
#define DUNE_BINARY_OP(OP)                                              \
    template<class T, std::size_t tAlign, class U, std::size_t uAlign,  \
             class = std::void_t<decltype(std::declval<T>()             \
                                     OP std::declval<U>())> >           \
    decltype(auto)                                                      \
      operator OP(const AlignedNumber<T, tAlign> &t,                    \
                  const AlignedNumber<U, uAlign> &u)                    \
    {                                                                   \
      /* can't use std::max(); not constexpr */                         \
      return aligned<(tAlign > uAlign ? tAlign : uAlign)>(T(t) OP U(u)); \
    }                                                                   \
                                                                        \
    template<class T, class U, std::size_t uAlign,                      \
             class = std::void_t<decltype(std::declval<T>()             \
                                     OP std::declval<U>())> >           \
    decltype(auto)                                                      \
      operator OP(const T &t, const AlignedNumber<U, uAlign> &u)        \
    {                                                                   \
      return aligned<uAlign>(t OP U(u));                                \
    }                                                                   \
                                                                        \
    template<class T, std::size_t tAlign, class U,                      \
             class = std::void_t<decltype(std::declval<T>()             \
                                     OP std::declval<U>())> >           \
    decltype(auto)                                                      \
      operator OP(const AlignedNumber<T, tAlign> &t, const U &u)        \
    {                                                                   \
      return aligned<tAlign>(T(t) OP u);                                \
    }                                                                   \
                                                                        \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_BINARY_OP(+);
    DUNE_BINARY_OP(-);

    DUNE_BINARY_OP(*);
    DUNE_BINARY_OP(/);
    DUNE_BINARY_OP(%);

    DUNE_BINARY_OP(^);
    DUNE_BINARY_OP(&);
    DUNE_BINARY_OP(|);

    DUNE_BINARY_OP(<<);
    DUNE_BINARY_OP(>>);

    DUNE_BINARY_OP(==);
    DUNE_BINARY_OP(!=);
    DUNE_BINARY_OP(<);
    DUNE_BINARY_OP(>);
    DUNE_BINARY_OP(<=);
    DUNE_BINARY_OP(>=);

    DUNE_BINARY_OP(&&);
    DUNE_BINARY_OP(||);

#undef DUNE_BINARY_OP

    //////////////////////////////////////////////////////////////////////
    //
    // Overloads for the functions provided by the standard library
    //
#define DUNE_UNARY_FUNC(name)                                       \
    template<class T, std::size_t align>                            \
    decltype(auto) name(const AlignedNumber<T, align> &u)           \
    {                                                               \
      using std::name;                                              \
      return aligned<align>(name(T(u)));                            \
    }                                                               \
    static_assert(true, "Require semicolon to unconfuse editors")

    //
    // <cmath> functions
    //

    // note: only unary functions are provided at the moment.  Getting all the
    // overloads right for functions with more than one argument is tricky.
    // All <cmath> functions appear in the list below in the order they are
    // listed in in the standard, but the unimplemented ones are commented
    // out.

    // note: abs is provided by both <cstdlib> (for integer) and <cmath> (for
    // floating point).  This overload works for both.
    DUNE_UNARY_FUNC(abs);
    DUNE_UNARY_FUNC(acos);
    DUNE_UNARY_FUNC(acosh);
    DUNE_UNARY_FUNC(asin);
    DUNE_UNARY_FUNC(asinh);
    DUNE_UNARY_FUNC(atan);
    // atan2
    DUNE_UNARY_FUNC(atanh);
    DUNE_UNARY_FUNC(cbrt);
    DUNE_UNARY_FUNC(ceil);
    // copysign
    DUNE_UNARY_FUNC(cos);
    DUNE_UNARY_FUNC(cosh);
    DUNE_UNARY_FUNC(erf);
    DUNE_UNARY_FUNC(erfc);
    DUNE_UNARY_FUNC(exp);
    DUNE_UNARY_FUNC(exp2);
    DUNE_UNARY_FUNC(expm1);
    DUNE_UNARY_FUNC(fabs);
    // fdim
    DUNE_UNARY_FUNC(floor);
    // fma
    // fmax
    // fmin
    // fmod
    // frexp
    // hypos
    DUNE_UNARY_FUNC(ilogb);
    // ldexp
    DUNE_UNARY_FUNC(lgamma);
    DUNE_UNARY_FUNC(llrint);
    DUNE_UNARY_FUNC(llround);
    DUNE_UNARY_FUNC(log);
    DUNE_UNARY_FUNC(log10);
    DUNE_UNARY_FUNC(log1p);
    DUNE_UNARY_FUNC(log2);
    DUNE_UNARY_FUNC(logb);
    DUNE_UNARY_FUNC(lrint);
    DUNE_UNARY_FUNC(lround);
    // modf
    DUNE_UNARY_FUNC(nearbyint);
    // nextafter
    // nexttoward
    // pow
    // remainder
    // remquo
    DUNE_UNARY_FUNC(rint);
    DUNE_UNARY_FUNC(round);
    // scalbln
    // scalbn
    DUNE_UNARY_FUNC(sin);
    DUNE_UNARY_FUNC(sinh);
    DUNE_UNARY_FUNC(sqrt);
    DUNE_UNARY_FUNC(tan);
    DUNE_UNARY_FUNC(tanh);
    DUNE_UNARY_FUNC(tgamma);
    DUNE_UNARY_FUNC(trunc);

    DUNE_UNARY_FUNC(isfinite);
    DUNE_UNARY_FUNC(isinf);
    DUNE_UNARY_FUNC(isnan);
    DUNE_UNARY_FUNC(isnormal);
    DUNE_UNARY_FUNC(signbit);

    // isgreater
    // isgreaterequal
    // isless
    // islessequal
    // islessgreater
    // isunordered

    //
    // <complex> functions
    //

    // not all functions are implemented, and unlike for <cmath>, no
    // comprehensive list is provided
    DUNE_UNARY_FUNC(real);

#undef DUNE_UNARY_FUNC

    // We need to overload min() and max() since they require types to be
    // LessThanComparable, which requires `a<b` to be "convertible to bool".
    // That wording seems to be a leftover from C++03, and today is probably
    // equivalent to "implicitly convertible".  There is also issue 2114
    // <https://cplusplus.github.io/LWG/issue2114> in the standard (still open
    // as of 2018-07-06), which strives to require both "implicitly" and
    // "contextually" convertible -- plus a few other things.
    //
    // We do not want our debug type to automatically decay to the underlying
    // type, so we do not want to make the conversion non-explicit.  So the
    // only option left is to overload min() and max().

    template<class T, std::size_t align>
    auto max(const AlignedNumber<T, align> &a,
             const AlignedNumber<T, align> &b)
    {
      using std::max;
      return aligned<align>(max(T(a), T(b)));
    }

    template<class T, std::size_t align>
    auto max(const T &a, const AlignedNumber<T, align> &b)
    {
      using std::max;
      return aligned<align>(max(a, T(b)));
    }

    template<class T, std::size_t align>
    auto max(const AlignedNumber<T, align> &a, const T &b)
    {
      using std::max;
      return aligned<align>(max(T(a), b));
    }

    template<class T, std::size_t align>
    auto min(const AlignedNumber<T, align> &a,
             const AlignedNumber<T, align> &b)
    {
      using std::min;
      return aligned<align>(min(T(a), T(b)));
    }

    template<class T, std::size_t align>
    auto min(const T &a, const AlignedNumber<T, align> &b)
    {
      using std::min;
      return aligned<align>(min(a, T(b)));
    }

    template<class T, std::size_t align>
    auto min(const AlignedNumber<T, align> &a, const T &b)
    {
      using std::min;
      return aligned<align>(min(T(a), b));
    }

  } // namespace AlignedNumberImpl

  // SIMD-like functions from "conditional.hh"
  template<class T, std::size_t align>
  AlignedNumber<T, align>
  cond(const AlignedNumber<bool, align> &b,
       const AlignedNumber<T, align> &v1, const AlignedNumber<T, align> &v2)
  {
    return b ? v1 : v2;
  }

  // SIMD-like functions from "rangeutilities.hh"
  template<class T, std::size_t align>
  T max_value(const AlignedNumber<T, align>& val)
  {
    return T(val);
  }

  template<class T, std::size_t align>
  T min_value(const AlignedNumber<T, align>& val)
  {
    return T(val);
  }

  template<std::size_t align>
  bool any_true(const AlignedNumber<bool, align>& val)
  {
    return bool(val);
  }

  template<std::size_t align>
  bool all_true(const AlignedNumber<bool, align>& val)
  {
    return bool(val);
  }

  // SIMD-like functionality from "simd/interface.hh"
  namespace Simd {
    namespace Overloads {

      template<class T, std::size_t align>
      struct ScalarType<AlignedNumber<T, align> > { using type = T; };

      template<class U, class T, std::size_t align>
      struct RebindType<U, AlignedNumber<T, align> > {
        using type = AlignedNumber<U, align>;
      };

      template<class T, std::size_t align>
      struct LaneCount<AlignedNumber<T, align> > : index_constant<1> {};

      template<class T, std::size_t align>
      T& lane(ADLTag<5>, std::size_t l, AlignedNumber<T, align> &v)
      {
        assert(l == 0);
        return v.value();
      }

      template<class T, std::size_t align>
      T lane(ADLTag<5>, std::size_t l, const AlignedNumber<T, align> &v)
      {
        assert(l == 0);
        return v.value();
      }

      template<class T, std::size_t align>
      const AlignedNumber<T, align> &
      cond(ADLTag<5>, AlignedNumber<bool, align> mask,
           const AlignedNumber<T, align> &ifTrue,
           const AlignedNumber<T, align> &ifFalse)
      {
        return mask ? ifTrue : ifFalse;
      }

      template<std::size_t align>
      bool anyTrue(ADLTag<5>, const AlignedNumber<bool, align> &mask)
      {
        return bool(mask);
      }

    } // namespace Overloads

  } // namespace Simd

} // namespace Dune

#endif // DUNE_DEBUGALIGN_HH
