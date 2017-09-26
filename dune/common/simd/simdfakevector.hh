#ifndef DUNE_SIMD_FAKEVECTOR_HH
#define DUNE_SIMD_FAKEVECTOR_HH

#include <array>
#include <iostream>
#include <dune/common/simd/simd.hh>

//using namespace std;

namespace Dune {

  /**
    *  This class specifies a vector-like type deriving from std::array for memory management and
    *  basic accessibility.
    *  This type is capable of dealing with all (well-defined) operators and is usable with the
    *  SIMD-interface.
    */

  template<class T, int S>
  class simdfakevector: public std::array<T,S> {

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
    DUNE_SIMD_FAKEVECTOR_UNARY_OP(!);
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
    /**@ToDo: size comparision*/				\
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
#undef DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OP

    //Bitwise operators
#define DUNE_SIMD_FAKEVECTOR_BITWISE_OP(SYMBOL) 			\
    auto operator SYMBOL(const T s) {                   	\
      for(std::size_t i=0; i<S; i++){		      		\
        (*this)[i] SYMBOL s;                            	\
      }								\
      return *this;                                             \
    }                                                   	\
    auto operator SYMBOL(const simdfakevector<T,S> &v) {        \
    /**@ToDo: size comparision*/                                \
      for(std::size_t i=0; i<S; i++){	           		\
        (*this)[i] SYMBOL v[i];                        	 	\
      }                                                		\
      return *this;						\
    }

    DUNE_SIMD_FAKEVECTOR_BITWISE_OP(<<=);
    DUNE_SIMD_FAKEVECTOR_BITWISE_OP(>>=);
    DUNE_SIMD_FAKEVECTOR_BITWISE_OP(&=);
    DUNE_SIMD_FAKEVECTOR_BITWISE_OP(|=);
    DUNE_SIMD_FAKEVECTOR_BITWISE_OP(^=);
#undef DUNE_SIMD_FAKEVECTOR_BITWISE_OP

    //dummy function for testing purpose
    void print(){
      for(auto it = arr_t::begin(); it != arr_t::end(); ++it){
        std::cout << *it << std::endl;
      }
    }
  };

  //Arithmetic operators
#define DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP(SYMBOL)			\
  template<class T, int S>						\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {	\
    simdfakevector<T,S> out;						\
    for(std::size_t i=0; i<S; i++){					\
      out[i] = v[i] SYMBOL s;						\
    }									\
    return out;								\
  }									\
  template<class T, int S>						\
  auto operator SYMBOL(const T s, const simdfakevector<T,S> &v) {	\
    return v SYMBOL s;							\
  }									\
  template<class T, int S>								\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const simdfakevector<T,S> &w) {	\
   /**@ToDo: size comparision*/								\
    simdfakevector<T,S> out;								\
      for(std::size_t i=0; i<S; i++){							\
        out[i] = v[i] SYMBOL w[i];							\
      }											\
    return out;										\
  }											\

  DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP(+);
  DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP(-);
  DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP(*);
  DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP(/);
  DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP(%);
#undef DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OP

  //Logical operators
#define DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(SYMBOL)	                        \
  template<class T, int S>                                              \
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {       \
    simdfakevector<bool,S> out;                                         \
    for(std::size_t i=0; i<S; i++){                                     \
      out[i] = v[i] SYMBOL s;                                           \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
  template<class T, int S>                                              \
  auto operator SYMBOL(const T s, const simdfakevector<T,S> &v) {       \
    return v SYMBOL s;                                                  \
  }                                                                     \
  template<class T, int S>                                                              \
  auto operator SYMBOL(const simdfakevector<T,S> &v, const simdfakevector<T,S> &w) {    \
   /**@ToDo: size comparision*/                                                         \
    simdfakevector<bool,S> out;                                                         \
      for(std::size_t i=0; i<S; i++){                                                   \
        out[i] = v[i] SYMBOL w[i];                                                      \
      }                                                                                 \
    return out;                                                                         \
  }                                                                                     \

  DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(<);
  DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(>);
  DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(<=);
  DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(>=);
  DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(==);
  DUNE_SIMD_FAKEVECTOR_LOGICAL_OP(!=);
#undef DUNE_SIMD_FAKEVECTOR_LOGICAL_OP

  //Boolean operators
#define DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(SYMBOL)                        					\
  template<class T, int S, typename std::enable_if_t<std::is_same<T, bool>::value, bool>* = nullptr>	\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {       				\
    simdfakevector<bool,S> out;                                         				\
    for(std::size_t i=0; i<S; i++){                                     				\
      out[i] = v[i] SYMBOL s;                                           				\
    }                                                                   				\
    return out;                                                         				\
  }                                                                     				\
  template<class T, int S, typename std::enable_if_t<std::is_same<T, bool>::value, bool>* = nullptr>	\
  auto operator SYMBOL(const bool s, const simdfakevector<T,S> &v) {      				\
    return v SYMBOL s;                                                  				\
  }                                                                     				\
  template<class T, int S, typename std::enable_if_t<std::is_same<T, bool>::value, bool>* = nullptr>    \
  auto operator SYMBOL(const simdfakevector<T,S> &v, const simdfakevector<T,S> &w) {    		\
   /**@ToDo: size comparision*/                                                         		\
    simdfakevector<bool,S> out;                                                         		\
      for(std::size_t i=0; i<S; i++){                                                   		\
        out[i] = v[i] SYMBOL w[i];                                                      		\
      }                                                                                 		\
    return out;                                                                         		\
  }
  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(&);
  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(|);
  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(^);
  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(&&);
  DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP(||);
#undef DUNE_SIMD_FAKEVECTOR_BOOLEAN_OP

  namespace Simd {
    namespace Overloads {
      /**
        *  Implementation/Overloads of the functions needed for SIMD-compatibility
        */

      /**@ToDo: check for type of V via SFINAE (has to be some sort of simdvectortype)
        *       perhaps need for implementation of some kind of getType()/getSize() function?
        */

      template<class V>
      Struct ScalarType<V> {
        using type = simdvectortype<T,1>;
      };

      template<class V>
      Struct IndexType<V> {
        using type = simdvectortype<int,S>;
      };

      template<class V>
      Struct MaskType<V> {
        using type = simdvectortype<bool,S>;
      };

/**
      template<class V>
      Scalar<V> lane(ADLtag<5>, std::size_t l, const V &v) {
        return v[l];
      }
*/

    }  //namespace Overloads
  }  //namespace Simd
}  //namespace Dune

#endif
