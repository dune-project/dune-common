#ifndef DUNE_SIMD_FAKEVECTOR_HH
#define DUNE_SIMD_FAKEVECTOR_HH

#include <array>
#include <iostream>
//#include <dune/common/simd/simd.hh>

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
  template<class T, int S, class = std::enable_if_t<std::is_same<T, bool>::value>>			\
  auto operator SYMBOL(const simdfakevector<T,S> &v, const T s) {       				\
    simdfakevector<bool,S> out;                                         				\
    for(std::size_t i=0; i<S; i++){                                     				\
      out[i] = v[i] SYMBOL s;                                           				\
    }                                                                   				\
    return out;                                                         				\
  }                                                                     				\
  template<class T, int S, class =  std::enable_if_t<std::is_same<T, bool>::value>>			\
  auto operator SYMBOL(const bool s, const simdfakevector<T,S> &v) {      				\
    return v SYMBOL s;                                                  				\
  }                                                                     				\
  template<class T, int S, typename std::enable_if_t<std::is_same<T, bool>::value>>			\
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

    /**
      *  Functions to determine if a given object is a simdfakevector-type
      */

    template<class V>
    struct isFakevector : std::false_type {};

    template<class V, class = std::enable_if_t<!std::is_same<V, std::decay<V>::type >::value>>
    struct isFakevector<V> : isFakevector<std::decay<V>::type > {};

    template<typename T, typename S>
    struct isFakevector<simdfakevector<T,S>> : std::true_type {};

    template<class V>
    struct isFakevectormask : std::false_type {};

    template<class V, class = std::enable_if_t<!std::is_same<V, std::decay<V>::type >::value>>
    struct isFakevectormask<V> : isFakevectormask<std::decay<V>::type > {};

    template<typename S>
    struct isFakevectormask<simdfakevector<bool,S>> : std::true_type {};



    namespace Overloads {
      /**
        *  Implementation/Overloads of the functions needed for SIMD-compatibility
        */

      //Implementation of SIMD-types
      template<class V>
      struct ScalarType<V, std::enable_if_t<isFakevector<V>::value> > {
        using type = typename simdvectortype<V::value_type,1>;
      };

      template<class V>
      struct IndexType<V, std::enable_if_t<isFakevector<V>::value> > {
        using type = typename simdvectortype<int,V::size_type>;
      };

      template<class V>
      struct MaskType<V, std::enable_if_t<isFakevector<V>::value> > {
        using type = typename simdvectortype<bool,V::size_type>;
      };

      //Implementation of SIMD-functionality
      template<class V, class = std::enable_if_t<isFakevector<V>::value> >
      std::size_t LaneCount(ADLTag<5>, const V &v) {
        return v.size();
      }

      template<class V, class = std::enable_if_t<isFakevector<V>::value> >
      Scalar<V> lane(ADLtag<5>, std::size_t l, const V &v) {
        return v[l];
      }

      /**@ToDo*/
      template<class V, class = std::enable_if_t<isFakevector<V>::value> >
      V cond(ADLTag<5>, MaskType<V> mask, V ifTrue, V ifFalse) {
        return ifTrue;
      }

      /**@ToDo*/
      template<class M, class = std::enable_if_t<isFakevectormask<M>::value> >
      bool anyTrue(ADLTag<5>, M mask) {
        return true;
      }

      /**@ToDo*/
      template<class M, class = std::enable_if_t<isFakevectormask<M>::value> >
      bool allTrue(ADLTag<5>, M mask) {
        return true;
      }

      /**@ToDo*/
      template<class M, class = std::enable_if_t<isFakevectormask<M>::value> >
      bool anyFalse(ADLTag<5>, M mask) {
        return true;
      }

      /**@ToDo*/
      template<class M, class = std::enable_if_t<isFakevectormask<M>::value> >
      bool allFalse(ADLTag<5>, M mask) {
        return true;
      }

    }  //namespace Overloads
  }  //namespace Simd
}  //namespace Dune

#endif
