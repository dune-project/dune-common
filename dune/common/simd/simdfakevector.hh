#ifndef DUNE_SIMD_FAKEVECTOR_HH
#define DUNE_SIMD_FAKEVECTOR_HH

#include <array>
#include <iostream>

//using namespace std;

namespace Dune {
  template<class T, int S> class simdfakevector: public std::array<T,S> {

   typedef std::array<T,S> arr_t;

  public:

  //Prefix operators
  #define DUNE_SIMD_FAKEVECTOR_PREFIX_OP(SYMBOL)	\
   auto operator SYMBOL() {				\
    simdfakevector<T,S> out;				\
    for(unsigned int i=0; i<arr_t::size(); i++){	\
     out[i] = SYMBOL((*this)[i]);			\
    }							\
    return out;						\
   }							\

   DUNE_SIMD_FAKEVECTOR_PREFIX_OP(++);
   DUNE_SIMD_FAKEVECTOR_PREFIX_OP(--);
   DUNE_SIMD_FAKEVECTOR_PREFIX_OP(+);
   DUNE_SIMD_FAKEVECTOR_PREFIX_OP(-);
   DUNE_SIMD_FAKEVECTOR_PREFIX_OP(!);

  #undef DUNE_SIMD_FAKEVECTOR_PREFIX_OPS

  //Postfix operators
  #define DUNE_SIMD_FAKEVECTOR_POSTFIX_OP(SYMBOL)	\
   auto operator SYMBOL(int){				\
    simdfakevector<T,S> out = *this;			\
    SYMBOL(*this);					\
    return out;						\
   }							\

   DUNE_SIMD_FAKEVECTOR_POSTFIX_OP(++);
   DUNE_SIMD_FAKEVECTOR_POSTFIX_OP(--);

  #undef DUNE_SIMD_FAKEVECTOR_POSTFIX_OPS

  //Assignment operators
  #define DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(SYMBOL)	\
   void operator SYMBOL(const T s) {			\
    for(unsigned int i=0; i<arr_t::size(); i++){	\
     (*this)[i] SYMBOL s;				\
    }							\
   }							\							\
   void operator SYMBOL(const simdfakevector<T,S> &V) {	\
   /**@ToDo: size comparision*/				\
    for(unsigned int i=0; i<V.size(); i++){		\
     (*this)[i] SYMBOL V[i];				\
    }							\
   }							\

   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(+=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(-=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(*=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(/=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(%=);

   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(<<=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(>>=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(&=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(|=);
   DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS(^=);

  #undef DUNE_SIMD_FAKEVECTOR_ASSIGNMENT_OPS

   //dummy function for testing purpose
   void print(){
    for(auto it = arr_t::begin(); it != arr_t::end(); ++it){
     std::cout << *it << std::endl;
    }
   }
  };

  //Arithmetic operators
  #define DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(SYMBOL)			\
  template<class T, int S>						\
   auto operator SYMBOL(const simdfakevector<T,S> &V, const T s) {	\
    simdfakevector<T,S> out;						\
    for(unsigned int i=0; i<V.size(); i++){				\
     out[i] = V[i] SYMBOL s;						\
    }									\
    return out;								\
   }									\									\
  template<class T, int S>						\
   auto operator SYMBOL(const T s, const simdfakevector<T,S> &V) {	\
    return V SYMBOL s;							\
   }									\											\
  template<class T, int S>								\
   auto operator SYMBOL(const simdfakevector<T,S> &V, const simdfakevector<T,S> &W) {	\
   /**@ToDo: size comparision*/								\
    simdfakevector<T,S> out;								\
     for(unsigned int i=0; i<V.size(); i++){						\
      out[i] = V[i] SYMBOL W[i];							\
     }											\
    return out;										\
   }											\

   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(+);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(-);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(*);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(/);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(%);

   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(<);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(>);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(<=);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(>=);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(==);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(!=);

   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(&);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(|);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(^);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(&&);
   DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS(||);

  #undef DUNE_SIMD_FAKEVECTOR_ARITHMETIC_OPS
}

#endif
