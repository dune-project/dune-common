// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SIMD_LOOP_HH
#define DUNE_COMMON_SIMD_LOOP_HH

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ostream>

#include <dune/common/math.hh>
#include <dune/common/simd/simd.hh>
#include <dune/common/typetraits.hh>

namespace Dune {


/*
 * silence warnings from GCC about using integer operands on a bool
 * (when instantiated for T=bool)
 */
#if __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wbool-operation"
#  pragma GCC diagnostic ignored "-Wint-in-bool-context"
#  define GCC_WARNING_DISABLED
#endif

/*
 * silence warnings from Clang about using bitwise operands on
 * a bool (when instantiated for T=bool)
 */
#ifdef __clang__
#if __has_warning("-Wbitwise-instead-of-logical")
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wbitwise-instead-of-logical"
#  define CLANG_WARNING_DISABLED
#endif
#endif

/*
 * Introduce a simd pragma if OpenMP is available in standard version >= 4
 */
#if _OPENMP >= 201307
  #define DUNE_PRAGMA_OMP_SIMD _Pragma("omp simd")
#else
  #define DUNE_PRAGMA_OMP_SIMD
#endif


  /**
    *  This class specifies a vector-like type deriving from std::array
    *  for memory management and basic accessibility.
    *  This type is capable of dealing with all (well-defined) operators
    *  and is usable with the SIMD-interface.
    *
    *  @tparam T Base type. Could also be vectorized type.
    *  @tparam S Size
    *  @tparam minimum alignment. It is inherited to rebound types.
    */

  template<class T, std::size_t S, std::size_t A = 0>
  class alignas(A==0?alignof(T):A) LoopSIMD : public std::array<T,S> {

  public:

    //default constructor
    LoopSIMD() {
      assert(reinterpret_cast<uintptr_t>(this) % std::min(alignof(LoopSIMD<T,S,A>),alignof(std::max_align_t)) == 0);
    }

    // broadcast constructor initializing the content with a given value
    LoopSIMD(Simd::Scalar<T> i) : LoopSIMD() {
      this->fill(i);
    }

    template<std::size_t OA>
      explicit LoopSIMD(const LoopSIMD<T,S,OA>& other)
      : std::array<T,S>(other)
    {
      assert(reinterpret_cast<uintptr_t>(this) % std::min(alignof(LoopSIMD<T,S,A>),alignof(std::max_align_t)) == 0);
    }

    /*
     *  Definition of basic operators
     */

    //Prefix operators
#define DUNE_SIMD_LOOP_PREFIX_OP(SYMBOL)         \
    auto operator SYMBOL() {                     \
      DUNE_PRAGMA_OMP_SIMD                       \
      for(std::size_t i=0; i<S; i++){            \
        SYMBOL(*this)[i];                        \
      }                                          \
      return *this;                              \
    }                                            \
    static_assert(true, "expecting ;")

    DUNE_SIMD_LOOP_PREFIX_OP(++);
    DUNE_SIMD_LOOP_PREFIX_OP(--);
#undef DUNE_SIMD_LOOP_PREFIX_OP

    //Unary operators
#define DUNE_SIMD_LOOP_UNARY_OP(SYMBOL)          \
    auto operator SYMBOL() const {               \
      LoopSIMD<T,S,A> out;                        \
      DUNE_PRAGMA_OMP_SIMD                       \
      for(std::size_t i=0; i<S; i++){            \
        out[i] = SYMBOL((*this)[i]);             \
      }                                          \
      return out;                                \
    }                                            \
    static_assert(true, "expecting ;")

    DUNE_SIMD_LOOP_UNARY_OP(+);
    DUNE_SIMD_LOOP_UNARY_OP(-);
    DUNE_SIMD_LOOP_UNARY_OP(~);

    auto operator!() const {
      Simd::Mask<LoopSIMD<T,S,A>> out;
      DUNE_PRAGMA_OMP_SIMD
      for(std::size_t i=0; i<S; i++){
        out[i] = !((*this)[i]);
      }
      return out;
    }
#undef DUNE_SIMD_LOOP_UNARY_OP

    //Postfix operators
#define DUNE_SIMD_LOOP_POSTFIX_OP(SYMBOL)        \
    auto operator SYMBOL(int){                   \
      LoopSIMD<T,S,A> out = *this;               \
      SYMBOL(*this);                             \
      return out;                                \
    }                                            \
    static_assert(true, "expecting ;")

   DUNE_SIMD_LOOP_POSTFIX_OP(++);
   DUNE_SIMD_LOOP_POSTFIX_OP(--);
#undef DUNE_SIMD_LOOP_POSTFIX_OP

    //Assignment operators
#define DUNE_SIMD_LOOP_ASSIGNMENT_OP(SYMBOL)              \
    auto operator SYMBOL(const Simd::Scalar<T> s) {               \
      DUNE_PRAGMA_OMP_SIMD                                \
      for(std::size_t i=0; i<S; i++){                     \
        (*this)[i] SYMBOL s;                              \
      }                                                   \
      return *this;                                       \
    }                                                     \
                                                          \
    auto operator SYMBOL(const LoopSIMD<T,S,A> &v) {      \
      DUNE_PRAGMA_OMP_SIMD                                \
      for(std::size_t i=0; i<S; i++){                     \
        (*this)[i] SYMBOL v[i];                           \
      }                                                   \
      return *this;                                       \
    }                                                     \
    static_assert(true, "expecting ;")

    DUNE_SIMD_LOOP_ASSIGNMENT_OP(+=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(-=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(*=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(/=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(%=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(<<=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(>>=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(&=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(|=);
    DUNE_SIMD_LOOP_ASSIGNMENT_OP(^=);
#undef DUNE_SIMD_LOOP_ASSIGNMENT_OP
  };

  //Arithmetic operators
#define DUNE_SIMD_LOOP_BINARY_OP(SYMBOL)                        \
  template<class T, std::size_t S, std::size_t A>                                \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v, const Simd::Scalar<T> s) { \
    LoopSIMD<T,S,A> out;                                                 \
    DUNE_PRAGMA_OMP_SIMD                                        \
    for(std::size_t i=0; i<S; i++){                             \
      out[i] = v[i] SYMBOL s;                                   \
    }                                                           \
    return out;                                                 \
  }                                                             \
  template<class T, std::size_t S, std::size_t A>                              \
  auto operator SYMBOL(const Simd::Scalar<T> s, const LoopSIMD<T,S,A> &v) { \
    LoopSIMD<T,S,A> out;                                                 \
    DUNE_PRAGMA_OMP_SIMD                                        \
    for(std::size_t i=0; i<S; i++){                             \
      out[i] = s SYMBOL v[i];                                   \
    }                                                           \
    return out;                                                 \
  }                                                             \
  template<class T, std::size_t S, std::size_t A>                              \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v,                         \
                       const LoopSIMD<T,S,A> &w) {                       \
    LoopSIMD<T,S,A> out;                                                 \
    DUNE_PRAGMA_OMP_SIMD                                        \
    for(std::size_t i=0; i<S; i++){                             \
      out[i] = v[i] SYMBOL w[i];                                \
    }                                                           \
    return out;                                                 \
  }                                                             \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_BINARY_OP(+);
  DUNE_SIMD_LOOP_BINARY_OP(-);
  DUNE_SIMD_LOOP_BINARY_OP(*);
  DUNE_SIMD_LOOP_BINARY_OP(/);
  DUNE_SIMD_LOOP_BINARY_OP(%);

  DUNE_SIMD_LOOP_BINARY_OP(&);
  DUNE_SIMD_LOOP_BINARY_OP(|);
  DUNE_SIMD_LOOP_BINARY_OP(^);

#undef DUNE_SIMD_LOOP_BINARY_OP

  //Bitshift operators
#define DUNE_SIMD_LOOP_BITSHIFT_OP(SYMBOL)                        \
  template<class T, std::size_t S, std::size_t A, class U>                       \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v, const U s) {            \
    LoopSIMD<T,S,A> out;                                                 \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL s;                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S, std::size_t A, class U, std::size_t AU>       \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v,                         \
                       const LoopSIMD<U,S,AU> &w) {                       \
    LoopSIMD<T,S,A> out;                                                 \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL w[i];                                  \
    }                                                             \
    return out;                                                   \
  }                                                               \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_BITSHIFT_OP(<<);
  DUNE_SIMD_LOOP_BITSHIFT_OP(>>);

#undef DUNE_SIMD_LOOP_BITSHIFT_OP

  //Comparison operators
#define DUNE_SIMD_LOOP_COMPARISON_OP(SYMBOL)                      \
  template<class T, std::size_t S, std::size_t A, class U>                       \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v, const U s) {            \
    Simd::Mask<LoopSIMD<T,S,A>> out;                                     \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL s;                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S, std::size_t A>                                \
  auto operator SYMBOL(const Simd::Scalar<T> s, const LoopSIMD<T,S,A> &v) { \
    Simd::Mask<LoopSIMD<T,S,A>> out;                                     \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = s SYMBOL v[i];                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S, std::size_t A>                                \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v,                         \
                       const LoopSIMD<T,S,A> &w) {                       \
    Simd::Mask<LoopSIMD<T,S,A>> out;                                     \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL w[i];                                  \
    }                                                             \
    return out;                                                   \
  }                                                               \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_COMPARISON_OP(<);
  DUNE_SIMD_LOOP_COMPARISON_OP(>);
  DUNE_SIMD_LOOP_COMPARISON_OP(<=);
  DUNE_SIMD_LOOP_COMPARISON_OP(>=);
  DUNE_SIMD_LOOP_COMPARISON_OP(==);
  DUNE_SIMD_LOOP_COMPARISON_OP(!=);
#undef DUNE_SIMD_LOOP_COMPARISON_OP

  //Boolean operators
#define DUNE_SIMD_LOOP_BOOLEAN_OP(SYMBOL)                         \
  template<class T, std::size_t S, std::size_t A>                                \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v, const Simd::Scalar<T> s) { \
    Simd::Mask<LoopSIMD<T,S,A>> out;                                     \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL s;                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S, std::size_t A>                                \
  auto operator SYMBOL(const Simd::Mask<T> s, const LoopSIMD<T,S,A> &v) { \
    Simd::Mask<LoopSIMD<T,S,A>> out;                                     \
    DUNE_PRAGMA_OMP_SIMD                                          \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = s SYMBOL v[i];                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S, std::size_t A>                                \
  auto operator SYMBOL(const LoopSIMD<T,S,A> &v,                         \
                       const LoopSIMD<T,S,A> &w) {                       \
    Simd::Mask<LoopSIMD<T,S,A>> out;                                     \
    DUNE_PRAGMA_OMP_SIMD                                          \
      for(std::size_t i=0; i<S; i++){                             \
        out[i] = v[i] SYMBOL w[i];                                \
      }                                                           \
    return out;                                                   \
  }                                                               \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_BOOLEAN_OP(&&);
  DUNE_SIMD_LOOP_BOOLEAN_OP(||);
#undef DUNE_SIMD_LOOP_BOOLEAN_OP

  //prints a given LoopSIMD
  template<class T, std::size_t S, std::size_t A>
  std::ostream& operator<< (std::ostream &os, const LoopSIMD<T,S,A> &v) {
    os << "[";
    for(std::size_t i=0; i<S-1; i++) {
      os << v[i] << ", ";
    }
    os << v[S-1] << "]";
    return os;
  }

  namespace Simd {
    namespace Overloads {
      /*
       *  Implementation/Overloads of the functions needed for
       *  SIMD-interface-compatibility
       */

      //Implementation of SIMD-interface-types
      template<class T, std::size_t S, std::size_t A>
      struct ScalarType<LoopSIMD<T,S,A>> {
        using type = Simd::Scalar<T>;
      };

      template<class U, class T, std::size_t S, std::size_t A>
      struct RebindType<U, LoopSIMD<T,S,A>> {
        using type =  LoopSIMD<Simd::Rebind<U, T>,S,A>;
      };

      //Implementation of SIMD-interface-functionality
      template<class T, std::size_t S, std::size_t A>
      struct LaneCount<LoopSIMD<T,S,A>> : index_constant<S*lanes<T>()> {};

      template<class T, std::size_t S, std::size_t A>
      auto lane(ADLTag<5>, std::size_t l, LoopSIMD<T,S,A> &&v)
        -> decltype(std::move(Simd::lane(l%lanes<T>(), v[l/lanes<T>()])))
      {
        return std::move(Simd::lane(l%lanes<T>(), v[l/lanes<T>()]));
      }

      template<class T, std::size_t S, std::size_t A>
      auto lane(ADLTag<5>, std::size_t l, const LoopSIMD<T,S,A> &v)
        -> decltype(Simd::lane(l%lanes<T>(), v[l/lanes<T>()]))
      {
        return Simd::lane(l%lanes<T>(), v[l/lanes<T>()]);
      }

      template<class T, std::size_t S, std::size_t A>
      auto lane(ADLTag<5>, std::size_t l, LoopSIMD<T,S,A> &v)
        -> decltype(Simd::lane(l%lanes<T>(), v[l/lanes<T>()]))
      {
        return Simd::lane(l%lanes<T>(), v[l/lanes<T>()]);
      }

      template<class T, std::size_t S, std::size_t AM, std::size_t AD>
      auto cond(ADLTag<5>, Simd::Mask<LoopSIMD<T,S,AM>> mask,
                LoopSIMD<T,S,AD> ifTrue, LoopSIMD<T,S,AD> ifFalse) {
        LoopSIMD<T,S,AD> out;
        for(std::size_t i=0; i<S; i++) {
          out[i] = Simd::cond(mask[i], ifTrue[i], ifFalse[i]);
        }
        return out;
      }

      template<class M, class T, std::size_t S, std::size_t A>
      auto cond(ADLTag<5, std::is_same<bool, Simd::Scalar<M> >::value
                && Simd::lanes<M>() == Simd::lanes<LoopSIMD<T,S,A> >()>,
                M mask, LoopSIMD<T,S,A> ifTrue, LoopSIMD<T,S,A> ifFalse)
      {
        LoopSIMD<T,S,A> out;
        for(auto l : range(Simd::lanes(mask)))
          Simd::lane(l, out) = Simd::lane(l, mask) ? Simd::lane(l, ifTrue) : Simd::lane(l, ifFalse);
        return out;
      }

      template<class M, std::size_t S, std::size_t A>
      bool anyTrue(ADLTag<5>, LoopSIMD<M,S,A> mask) {
        bool out = false;
        for(std::size_t i=0; i<S; i++) {
          out |= Simd::anyTrue(mask[i]);
        }
        return out;
      }

      template<class M, std::size_t S, std::size_t A>
      bool allTrue(ADLTag<5>, LoopSIMD<M,S,A> mask) {
        bool out = true;
        for(std::size_t i=0; i<S; i++) {
          out &= Simd::allTrue(mask[i]);
        }
        return out;
      }

      template<class M, std::size_t S, std::size_t A>
      bool anyFalse(ADLTag<5>, LoopSIMD<M,S,A> mask) {
        bool out = false;
        for(std::size_t i=0; i<S; i++) {
          out |= Simd::anyFalse(mask[i]);
        }
        return out;
      }

      template<class M, std::size_t S, std::size_t A>
      bool allFalse(ADLTag<5>, LoopSIMD<M,S,A> mask) {
        bool out = true;
        for(std::size_t i=0; i<S; i++) {
          out &= Simd::allFalse(mask[i]);
        }
        return out;
      }
    }  //namespace Overloads

  }  //namespace Simd


  /*
   *  Overloads the unary cmath-operations. Operations requiring
   *  or returning more than one argument are not supported.
   *  Due to inconsistency with the return values, cmath-operations
   *  on integral types are also not supported-
   */

#define DUNE_SIMD_LOOP_CMATH_UNARY_OP(expr)                          \
  template<class T, std::size_t S, std::size_t A, typename Sfinae =                 \
           typename std::enable_if_t<!std::is_integral<Simd::Scalar<T>>::value> > \
  auto expr(const LoopSIMD<T,S,A> &v) {                                  \
    using std::expr;                                                 \
    LoopSIMD<T,S,A> out;                                              \
    for(std::size_t i=0; i<S; i++) {                                 \
      out[i] = expr(v[i]);                                           \
    }                                                                \
    return out;                                                      \
  }                                                                  \
  static_assert(true, "expecting ;")

#define DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(expr, returnType)  \
  template<class T, std::size_t S, std::size_t A, typename Sfinae =                 \
           typename std::enable_if_t<!std::is_integral<Simd::Scalar<T>>::value> > \
  auto expr(const LoopSIMD<T,S,A> &v) {                                  \
    using std::expr;                                                 \
    LoopSIMD<returnType,S> out;                                      \
    for(std::size_t i=0; i<S; i++) {                                 \
      out[i] = expr(v[i]);                                           \
    }                                                                \
    return out;                                                      \
  }                                                                  \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_CMATH_UNARY_OP(cos);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(sin);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(tan);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(acos);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(asin);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(atan);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(cosh);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(sinh);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(tanh);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(acosh);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(asinh);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(atanh);

  DUNE_SIMD_LOOP_CMATH_UNARY_OP(exp);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(log);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(log10);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(exp2);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(expm1);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(ilogb, int);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(log1p);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(log2);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(logb);

  DUNE_SIMD_LOOP_CMATH_UNARY_OP(sqrt);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(cbrt);

  DUNE_SIMD_LOOP_CMATH_UNARY_OP(erf);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(erfc);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(tgamma);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(lgamma);

  DUNE_SIMD_LOOP_CMATH_UNARY_OP(ceil);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(floor);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(trunc);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(round);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(lround, long);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(llround, long long);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(rint);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(lrint, long);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(llrint, long long);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(nearbyint);

  DUNE_SIMD_LOOP_CMATH_UNARY_OP(fabs);
  DUNE_SIMD_LOOP_CMATH_UNARY_OP(abs);

#undef DUNE_SIMD_LOOP_CMATH_UNARY_OP
#undef DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN


  /*  not implemented cmath-functions:
   *  atan2
   *  frexp, idexp
   *  modf
   *  scalbn, scalbln
   *  pow
   *  hypot
   *  remainder, remquo
   *  copysign
   *  nan
   *  nextafter, nexttoward
   *  fdim, fmax, fmin
   */

  /*
   * Overloads specific functions usually provided by the std library
   * More overloads will be provided should the need arise.
   */

#define DUNE_SIMD_LOOP_STD_UNARY_OP(expr)                  \
  template<class T, std::size_t S, std::size_t A>          \
  auto expr(const LoopSIMD<T,S,A> &v) {                     \
    using std::expr;                        \
    LoopSIMD<T,S,A> out;                     \
    for(std::size_t i=0; i<S; i++) {        \
      out[i] = expr(v[i]);                  \
    }                                       \
    return out;                             \
  }                                         \
                                                    \
  template<class T, std::size_t S, std::size_t A>                  \
  auto expr(const LoopSIMD<std::complex<T>,S,A> &v) {               \
    using std::expr;                                \
    LoopSIMD<T,S,A> out;                             \
    for(std::size_t i=0; i<S; i++) {                \
      out[i] = expr(v[i]);                          \
    }                                               \
    return out;                                     \
  }                                                 \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_STD_UNARY_OP(real);
  DUNE_SIMD_LOOP_STD_UNARY_OP(imag);

#undef DUNE_SIMD_LOOP_STD_UNARY_OP

#define DUNE_SIMD_LOOP_STD_BINARY_OP(expr)                    \
  template<class T, std::size_t S, std::size_t A>                           \
  auto expr(const LoopSIMD<T,S,A> &v, const LoopSIMD<T,S,A> &w) {        \
    using std::expr;                                          \
    LoopSIMD<T,S,A> out;                                       \
    for(std::size_t i=0; i<S; i++) {                          \
      out[i] = expr(v[i],w[i]);                               \
    }                                                         \
    return out;                                               \
  }                                                           \
  static_assert(true, "expecting ;")

  DUNE_SIMD_LOOP_STD_BINARY_OP(max);
  DUNE_SIMD_LOOP_STD_BINARY_OP(min);

#undef DUNE_SIMD_LOOP_STD_BINARY_OP

  namespace MathOverloads {
    template<class T, std::size_t S, std::size_t A>
    auto isNaN(const LoopSIMD<T,S,A> &v, PriorityTag<3>, ADLTag) {
      Simd::Mask<LoopSIMD<T,S,A>> out;
      for(auto l : range(S))
        out[l] = Dune::isNaN(v[l]);
      return out;
    }

    template<class T, std::size_t S, std::size_t A>
    auto isInf(const LoopSIMD<T,S,A> &v, PriorityTag<3>, ADLTag) {
      Simd::Mask<LoopSIMD<T,S,A>> out;
      for(auto l : range(S))
        out[l] = Dune::isInf(v[l]);
      return out;
    }

    template<class T, std::size_t S, std::size_t A>
    auto isFinite(const LoopSIMD<T,S,A> &v, PriorityTag<3>, ADLTag) {
      Simd::Mask<LoopSIMD<T,S,A>> out;
      for(auto l : range(S))
        out[l] = Dune::isFinite(v[l]);
      return out;
    }
  } //namespace MathOverloads

  template<class T, std::size_t S, std::size_t A>
  struct IsNumber<LoopSIMD<T,S,A>> :
          public std::integral_constant<bool, IsNumber<T>::value>{
  };

#ifdef CLANG_WARNING_DISABLED
#  pragma clang diagnostic pop
#  undef CLANG_WARNING_DISABLED
#endif

#ifdef GCC_WARNING_DISABLED
#  pragma GCC diagnostic pop
#  undef GCC_WARNING_DISABLED
#endif

} //namespace Dune

#endif
