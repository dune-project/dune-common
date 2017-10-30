#ifndef DUNE_COMMON_SIMD_LOOP_HH
#define DUNE_COMMON_SIMD_LOOP_HH

#include <array>
#include <ostream>
#include <cstddef>
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

#endif
