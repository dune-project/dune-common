#ifndef DUNE_COMMON_SIMD_LOOP_HH
#define DUNE_COMMON_SIMD_LOOP_HH

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <ostream>

#include <dune/common/simd/simd.hh>


namespace Dune {

  /**
    *  This class specifies a vector-like type deriving from std::array
    *  for memory management and basic accessibility.
    *  This type is capable of dealing with all (well-defined) operators
    *  and is usable with the SIMD-interface.
    */

  template<class T, std::size_t S>
  class LoopSIMD : public std::array<T,S> {

  public:

    //default constructor
    LoopSIMD() {}

    //constructor initializing the content with a given value
    template<class I>
    LoopSIMD(I i) : LoopSIMD() {
      this->fill(i);
    }

    /*
     *  Definition of basic operators
     */

    //Prefix operators
#define DUNE_SIMD_LOOP_PREFIX_OP(SYMBOL)         \
    auto operator SYMBOL() {                     \
      for(std::size_t i=0; i<S; i++){            \
        SYMBOL(*this)[i];                        \
      }                                          \
      return *this;                              \
    }

    DUNE_SIMD_LOOP_PREFIX_OP(++);
    DUNE_SIMD_LOOP_PREFIX_OP(--);
#undef DUNE_SIMD_LOOP_PREFIX_OP

    //Unary operators
#define DUNE_SIMD_LOOP_UNARY_OP(SYMBOL)          \
    auto operator SYMBOL() const {               \
      LoopSIMD<T,S> out;                         \
      for(std::size_t i=0; i<S; i++){            \
        out[i] = SYMBOL((*this)[i]);             \
      }                                          \
      return out;                                \
    }

    DUNE_SIMD_LOOP_UNARY_OP(+);
    DUNE_SIMD_LOOP_UNARY_OP(-);
    DUNE_SIMD_LOOP_UNARY_OP(~);

    auto operator!() const {
      LoopSIMD<bool,S> out;
      for(std::size_t i=0; i<S; i++){
        out[i] = !((*this)[i]);
      }
      return out;
    }
#undef DUNE_SIMD_LOOP_UNARY_OP

    //Postfix operators
#define DUNE_SIMD_LOOP_POSTFIX_OP(SYMBOL)        \
    auto operator SYMBOL(int){                   \
      LoopSIMD<T,S> out = *this;                 \
      SYMBOL(*this);                             \
      return out;                                \
    }

   DUNE_SIMD_LOOP_POSTFIX_OP(++);
   DUNE_SIMD_LOOP_POSTFIX_OP(--);
#undef DUNE_SIMD_LOOP_POSTFIX_OP

    //Assignment operators
#define DUNE_SIMD_LOOP_ASSIGNMENT_OP(SYMBOL)              \
    auto operator SYMBOL(const T s) {                     \
      for(std::size_t i=0; i<S; i++){                     \
        (*this)[i] SYMBOL s;                              \
      }                                                   \
      return *this;                                       \
    }                                                     \
    auto operator SYMBOL(const LoopSIMD<T,S> &v) {        \
      for(std::size_t i=0; i<S; i++){                     \
        (*this)[i] SYMBOL v[i];                           \
      }                                                   \
      return *this;                                       \
    }

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
  template<class T, std::size_t S>                              \
  auto operator SYMBOL(const LoopSIMD<T,S> &v, const T s) {     \
    LoopSIMD<T,S> out;                                          \
    for(std::size_t i=0; i<S; i++){                             \
      out[i] = v[i] SYMBOL s;                                   \
    }                                                           \
    return out;                                                 \
  }                                                             \
  template<class T, std::size_t S>                              \
  auto operator SYMBOL(const T s, const LoopSIMD<T,S> &v) {     \
    LoopSIMD<T,S> out;                                          \
    for(std::size_t i=0; i<S; i++){                             \
      out[i] = s SYMBOL v[i];                                   \
    }                                                           \
    return out;                                                 \
  }                                                             \
  template<class T, std::size_t S>                              \
  auto operator SYMBOL(const LoopSIMD<T,S> &v,                  \
                       const LoopSIMD<T,S> &w) {                \
    LoopSIMD<T,S> out;                                          \
    for(std::size_t i=0; i<S; i++){                             \
      out[i] = v[i] SYMBOL w[i];                                \
    }                                                           \
    return out;                                                 \
  }

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
  template<class T, std::size_t S, class U>                       \
  auto operator SYMBOL(const LoopSIMD<T,S> &v, const U s) {       \
    LoopSIMD<T,S> out;                                            \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL s;                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S, class U>                       \
  auto operator SYMBOL(const LoopSIMD<T,S> &v,                    \
                       const LoopSIMD<U,S> &w) {                  \
    LoopSIMD<T,S> out;                                            \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL w[i];                                  \
    }                                                             \
    return out;                                                   \
  }

  DUNE_SIMD_LOOP_BITSHIFT_OP(<<);
  DUNE_SIMD_LOOP_BITSHIFT_OP(>>);

#undef DUNE_SIMD_LOOP_BITSHIFT_OP

  //Comparison operators
#define DUNE_SIMD_LOOP_COMPARISON_OP(SYMBOL)                      \
  template<class T, std::size_t S>                                \
  auto operator SYMBOL(const LoopSIMD<T,S> &v, const T s) {       \
    LoopSIMD<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL s;                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S>                                \
  auto operator SYMBOL(const T s, const LoopSIMD<T,S> &v) {       \
    LoopSIMD<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = s SYMBOL v[i];                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S>                                \
  auto operator SYMBOL(const LoopSIMD<T,S> &v,                    \
                       const LoopSIMD<T,S> &w) {                  \
    LoopSIMD<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL w[i];                                  \
    }                                                             \
    return out;                                                   \
  }                                                               \

  DUNE_SIMD_LOOP_COMPARISON_OP(<);
  DUNE_SIMD_LOOP_COMPARISON_OP(>);
  DUNE_SIMD_LOOP_COMPARISON_OP(<=);
  DUNE_SIMD_LOOP_COMPARISON_OP(>=);
  DUNE_SIMD_LOOP_COMPARISON_OP(==);
  DUNE_SIMD_LOOP_COMPARISON_OP(!=);
#undef DUNE_SIMD_LOOP_COMPARISON_OP

  //Boolean operators
#define DUNE_SIMD_LOOP_BOOLEAN_OP(SYMBOL)                         \
  template<class T, std::size_t S>                                \
  auto operator SYMBOL(const LoopSIMD<T,S> &v, const T s) {       \
    LoopSIMD<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = v[i] SYMBOL s;                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S>                                \
  auto operator SYMBOL(const bool s, const LoopSIMD<T,S> &v) {    \
    LoopSIMD<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                               \
      out[i] = s SYMBOL v[i];                                     \
    }                                                             \
    return out;                                                   \
  }                                                               \
  template<class T, std::size_t S>                                \
  auto operator SYMBOL(const LoopSIMD<T,S> &v,                    \
                       const LoopSIMD<T,S> &w) {                  \
    LoopSIMD<bool,S> out;                                         \
      for(std::size_t i=0; i<S; i++){                             \
        out[i] = v[i] SYMBOL w[i];                                \
      }                                                           \
    return out;                                                   \
  }

  DUNE_SIMD_LOOP_BOOLEAN_OP(&&);
  DUNE_SIMD_LOOP_BOOLEAN_OP(||);
#undef DUNE_SIMD_LOOP_BOOLEAN_OP

  //prints a given LoopSIMD
  template<class T, std::size_t S>
  std::ostream& operator<< (std::ostream &os, const LoopSIMD<T,S> &v) {
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
      template<class T, std::size_t S>
      struct ScalarType<LoopSIMD<T,S>> {
        using type = T;
      };

      template<class T, std::size_t S>
      struct IndexType<LoopSIMD<T,S>> {
        using type =  LoopSIMD<std::size_t,S>;
      };

      template<class T, std::size_t S>
      struct MaskType<LoopSIMD<T,S>> {
        using type =  LoopSIMD<bool,S>;
      };

      //Implementation of SIMD-interface-functionality
      template<class T, std::size_t S>
      struct LaneCount<LoopSIMD<T,S>> : index_constant<S> {};

      template<class T, std::size_t S>
      T lane(ADLTag<5>, std::size_t l, const LoopSIMD<T,S> &v) {
       return v[l];
      }

      template<class T, std::size_t S>
      T& lane(ADLTag<5>, std::size_t l, LoopSIMD<T,S> &v) {
        return v[l];
      }

      template<class T, std::size_t S>
      auto cond(ADLTag<5>, LoopSIMD<bool,S> mask,
                LoopSIMD<T,S> ifTrue, LoopSIMD<T,S> ifFalse) {
        LoopSIMD<T,S> out;
        for(std::size_t i=0; i<S; i++) {
          out[i] = mask[i] ? ifTrue[i] : ifFalse[i];
        }
        return out;
      }

      template<std::size_t S>
      bool anyTrue(ADLTag<5>, LoopSIMD<bool,S> mask) {
        bool out = false;
        for(std::size_t i=0; i<S; i++) {
          out |= mask[i];
        }
        return out;
      }

      template<std::size_t S>
      bool allTrue(ADLTag<5>, LoopSIMD<bool,S> mask) {
        bool out = true;
        for(std::size_t i=0; i<S; i++) {
          out &= mask[i];
        }
        return out;
      }

      template<std::size_t S>
      bool anyFalse(ADLTag<5>, LoopSIMD<bool,S> mask) {
        bool out = false;
        for(std::size_t i=0; i<S; i++) {
          out |= !mask[i];
        }
        return out;
      }

      template<std::size_t S>
      bool allFalse(ADLTag<5>, LoopSIMD<bool,S> mask) {
        bool out = true;
        for(std::size_t i=0; i<S; i++) {
          out &= !mask[i];
        }
        return out;
      }
    }  //namespace Overloads
  }  //namespace Simd
}  //namespace Dune

/*
 *  Overloads the unary cmath-operations. Operations requiring
 *  or returning more than one argument are not supported.
 *  Due to inconsistency with the return values, cmath-operations
 *  on integral types are also not supported-
 */

#define DUNE_SIMD_LOOP_CMATH_UNARY_OP(expr)                          \
template<class T, std::size_t S, typename Sfinae =                   \
         typename std::enable_if_t<!std::is_integral<T>::value> >    \
auto expr(const Dune::LoopSIMD<T,S> &v) {                            \
  Dune::LoopSIMD<T,S> out;                                           \
  for(std::size_t i=0; i<S; i++) {                                   \
    out[i] = expr(v[i]);                                             \
  }                                                                  \
  return out;                                                        \
}

#define DUNE_SIMD_LOOP_CMATH_UNARY_OP_WITH_RETURN(expr, returnType)  \
template<class T, std::size_t S, typename Sfinae =                   \
         typename std::enable_if_t<!std::is_integral<T>::value> >    \
auto expr(const Dune::LoopSIMD<T,S> &v){                             \
  Dune::LoopSIMD<returnType,S> out;                                  \
  for(std::size_t i=0; i<S; i++) {                                   \
    out[i] = expr(v[i]);                                             \
  }                                                                  \
  return out;                                                        \
}


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
  /* the abs function needs special treatment*/
  //  DUNE_SIMD_LOOP_CMATH_UNARY_OP(abs);

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
 *
 * @ToDo: check for fabs
 * @ToDo: check if sfinae is needed for specific functions (I think it is, at least for abs)
 */

namespace Dune {
  template<class T, std::size_t S>
  auto abs(const Dune::LoopSIMD<T,S> &v) {
    using std::abs;
    Dune::LoopSIMD<T,S> out;
    for(std::size_t i=0; i<S; i++) {
      out[i] = abs(v[i]);
    }
    return out;
  }

#define DUNE_SIMD_LOOP_STD_BINARY_OP(expr)                                \
  template<class T, std::size_t S>                                        \
  auto expr(const Dune::LoopSIMD<T,S> &v, const Dune::LoopSIMD<T,S> &w) { \
    using std::expr;                                                      \
    Dune::LoopSIMD<T,S> out;                                              \
    for(std::size_t i=0; i<S; i++) {                                      \
      out[i] = expr(v[i],w[i]);                                           \
    }                                                                     \
    return out;                                                           \
  }

  //DUNE_SIMD_STD_UNARY_OP(abs);
  DUNE_SIMD_LOOP_STD_BINARY_OP(max);
  DUNE_SIMD_LOOP_STD_BINARY_OP(min);

//#undef DUNE_SIMD_LOOP_STD_UNARY_OP
#undef DUNE_SIMD_LOOP_STD_BINARY_OP

} //namespace Dune

#endif
