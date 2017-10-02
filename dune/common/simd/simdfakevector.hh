#ifndef DUNE_SIMD_FAKEVECTOR_HH
#define DUNE_SIMD_FAKEVECTOR_HH

#include <array>
#include <iostream>
#include <dune/common/simd/simd.hh>

namespace Dune {

  /**
    *  This class specifies a vector-like type deriving from std::array for memory management and
    *  basic accessibility.
    *  This type is capable of dealing with all (well-defined) operators and is usable with the
    *  SIMD-interface.
    */

  template<class T, std::size_t S>
  class simdfakevector: public std::array<T,S> {

    /**ToDo:remove*/
    typedef std::array<T,S> arr_t;

  public:

    /**
      *  Definition of basic operators
      */

    //Prefix operators
#define DUNE_SIMD_FAKEVECTOR_PREFIX_OP(SYMBOL)		\
    auto operator SYMBOL() {				\
      for(std::size_t i=0; i<S; i++){			\
        SYMBOL(*this)[i];				\
      }							\
      return *this;					\
    }

    DUNE_SIMD_FAKEVECTOR_PREFIX_OP(++);
    DUNE_SIMD_FAKEVECTOR_PREFIX_OP(--);
#undef DUNE_SIMD_FAKEVECTOR_PREFIX_OP

#define DUNE_SIMD_FAKEVECTOR_UNARY_OP(SYMBOL)		\
    auto operator SYMBOL() {                            \
      simdfakevector<T,S> out;                          \
      for(std::size_t i=0; i<S; i++){		        \
        out[i] = SYMBOL((*this)[i]);                    \
      }                                                 \
      return out;                            		\
    }

    DUNE_SIMD_FAKEVECTOR_UNARY_OP(+);
    DUNE_SIMD_FAKEVECTOR_UNARY_OP(-);

    auto operator!() {
      simdfakevector<bool,S> out;
      for(std::size_t i=0; i<S; i++){
        out[i] = !((*this)[i]);
      }
      return out;
    }
#undef DUNE_SIMD_FAKEVECTOR_UNARY_OP

    //Postfix operators
#define DUNE_SIMD_FAKEVECTOR_POSTFIX_OP(SYMBOL)		\
    auto operator SYMBOL(int){				\
      simdfakevector<T,S> out = *this;			\
      SYMBOL(*this);					\
      return out;					\
    }

   DUNE_SIMD_FAKEVECTOR_POSTFIX_OP(++);
   DUNE_SIMD_FAKEVECTOR_POSTFIX_OP(--);
#undef DUNE_SIMD_FAKEVECTOR_POSTFIX_OP

    //Assignment operators
#define DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(SYMBOL)		\
    auto operator SYMBOL(const T s) {				\
      for(std::size_t i=0; i<S; i++){				\
        (*this)[i] SYMBOL s;					\
      }								\
      return *this;						\
    }								\
    auto operator SYMBOL(const simdfakevector<T,S> &v) {	\
      for(std::size_t i=0; i<S; i++){				\
        (*this)[i] SYMBOL v[i];					\
      }								\
      return *this;						\
    }

    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(+=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(-=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(*=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(/=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(%=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(<<=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(>>=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(&=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(|=);
    DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP(^=);
#undef DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP
  };

  //Arithmetic operators
#define DUNE_SIMD_FAKEVECTOR_BINARY_OP(SYMBOL)				\
  template<class T, std::size_t S>					\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {	\
    simdfakevector<T,S> out;						\
    for(std::size_t i=0; i<S; i++){                                     \
      out[i] = v[i] SYMBOL s;                                           \
    }                                                                   \
    return out;                                                         \
  }									\
  template<class T, std::size_t S>					\
  auto operator SYMBOL(const T s, const simdfakevector<T,S> &v) {	\
    simdfakevector<T,S> out;                                            \
    for(std::size_t i=0; i<S; i++){                                     \
      out[i] = s SYMBOL v[i];                                           \
    }                                                                   \
    return out;                                                         \
  }									\
  template<class T, std::size_t S>				         		\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const simdfakevector<T,S> &w) {	\
    simdfakevector<T,S> out;								\
    for(std::size_t i=0; i<S; i++){							\
      out[i] = v[i] SYMBOL w[i];							\
    }											\
    return out;										\
  }

  DUNE_SIMD_FAKEVECTOR_BINARY_OP(+);
  DUNE_SIMD_FAKEVECTOR_BINARY_OP(-);
  DUNE_SIMD_FAKEVECTOR_BINARY_OP(*);
  DUNE_SIMD_FAKEVECTOR_BINARY_OP(/);
  DUNE_SIMD_FAKEVECTOR_BINARY_OP(%);

  DUNE_SIMD_FAKEVECTOR_BINARY_OP(&);
  DUNE_SIMD_FAKEVECTOR_BINARY_OP(|);
  DUNE_SIMD_FAKEVECTOR_BINARY_OP(^);

#undef DUNE_SIMD_FAKEVECTOR_BINARY_OP

  //Comparison operators
#define DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(SYMBOL)	                \
  template<class T, std::size_t S>                                      \
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {       \
    simdfakevector<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                                     \
      out[i] = v[i] SYMBOL s;                                           \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
  template<class T, std::size_t S>                                      \
  auto operator SYMBOL(const T s, const simdfakevector<T,S> &v) {       \
    simdfakevector<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                                     \
      out[i] = s SYMBOL v[i];                                           \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
  template<class T, std::size_t S>                                                      \
  auto operator SYMBOL(const simdfakevector<T,S> &v, const simdfakevector<T,S> &w) {    \
    simdfakevector<bool,S> out;                                                         \
    for(std::size_t i=0; i<S; i++){                                                     \
      out[i] = v[i] SYMBOL w[i];                                                        \
    }                                                                                   \
    return out;                                                                         \
  }                                                                                     \

  DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(<);
  DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(>);
  DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(<=);
  DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(>=);
  DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(==);
  DUNE_SIMD_FAKEVECTOR_COMPARISON_OP(!=);
#undef DUNE_SIMD_FAKEVECTOR_COMPARISON_OP

  //Boolean operators
#define DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(SYMBOL)                        					\
  template<class T, std::size_t S>                                                            		\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {       				\
    simdfakevector<bool,S> out;                                         				\
    for(std::size_t i=0; i<S; i++){                                     				\
      out[i] = v[i] SYMBOL s;                                           				\
    }                                                                   				\
    return out;                                                         				\
  }                                                                     				\
  template<class T, std::size_t S>                                                       		\
  auto operator SYMBOL(const bool s, const simdfakevector<T,S> &v) {      				\
    simdfakevector<bool,S> out;                                                                         \
    for(std::size_t i=0; i<S; i++){                                                                     \
      out[i] = s SYMBOL v[i];                                                                           \
    }                                                                                                   \
    return out;                                                                                         \
  }                                                                      				\
  template<class T, std::size_t S>                                                         		\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const simdfakevector<T,S> &w) {    		\
    simdfakevector<bool,S> out;                                                         		\
      for(std::size_t i=0; i<S; i++){                                                   		\
        out[i] = v[i] SYMBOL w[i];                                                      		\
      }                                                                                 		\
    return out;                                                                         		\
  }

  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(&&);
  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(||);
#undef DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP

  //prints a given simdfakevector
  template<class T, std::size_t S>
  std::ostream& operator<< (std::ostream &os, const simdfakevector<T,S> &v) {
    os << "[";
    for(std::size_t i; i<S-1; i++) {
      os << v[i] << ", ";
    }
    os << v[S-1] << "]";
    return os;
  }

  namespace Simd {
    namespace Overloads {
      /**
        *  Implementation/Overloads of the functions needed for SIMD-compatibility
        */

      //Implementation of SIMD-types
      template<class T, std::size_t S>
      struct ScalarType<simdfakevector<T,S>> {
        using type = typename simdvectortype<T,1>;
      };

      template<class T, std::size_t S>
      struct IndexType<simdfakevector<T,S>> {
        using type = typename simdvectortype<std::size_t,S>;
      };

      template<class T, std::size_t S>
      struct MaskType<simdfakevector<T,S>> {
        using type = typename simdvectortype<bool,S>;
      };

      //Implementation of SIMD-functionality
      template<class T, std::size_t S>
      struct LaneCount(ADLTag<5>, const simdfakevector<T,S> &v) {
        return S;
      };

      template<class T, std::size_t S>
      auto lane(ADLtag<5>, std::size_t l, const simdfakevector<T,S> &v) {
        return v[l];
      }

      template<class T, std::size_t S>
      auto lane(ADLtag<5>, std::size_t l, simdfakevector<T,S> &v) {
        return v[l];
      }

      template<class T, std::size_t S>
      auto cond(ADLTag<5>, simdfakevector<bool,S> mask, simdfakevector<T,S> ifTrue, simdfakevector<T,S> ifFalse) {
        simdfakevector<T,S> out;
        for(std::size_t i; i<S; i++) {
          out[i] = mask[i] ? ifTrue[i] : ifFalse[i];
      }

      template<std::size_t S>
      bool anyTrue(ADLTag<5>, simdfakevector<bool,S> mask) {
        bool out = false;
        for(std::size_t i; i<S; i++) {
          out |= mask[i];
        }
        return out;
      }

      template<std::size_t S>
      bool allTrue(ADLTag<5>, simdfakevector<bool,S> mask) {
        bool out = true;
        for(std::size_t i; i<S; i++) {
          out &= mask[i];
        }
        return out;
      }

      template<std::size_t S>
      bool anyFalse(ADLTag<5>, simdfakevector<bool,S> mask) {
        bool out = false;
        for(std::size_t i; i<S; i++) {
          out |= !mask[i];
        }
        return out;
      }

      template<std::size_t S>
      bool allFalse(ADLTag<5>, simdfakevector<bool,S> mask) {
        bool out = true;
        for(std::size_t i; i<S; i++) {
          out  &= !mask[i];
        }
        return out;
      }
    }  //namespace Overloads
  }  //namespace Simd
}  //namespace Dune

#endif
