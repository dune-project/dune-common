// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/**
   Dune::array

   is a wrapper for int*, char*, etc.
   you have 2 template parameters for basetype and size
 */

#ifndef ARRAY_HH
#define ARRAY_HH

#include <assert.h>
#include <string.h>
#include <iostream>

namespace Dune {

  template <int N, class T=int>
  class array {
  private:
    T data_[N];
  public:
    inline array() {};
    // usefull copy contructors
    inline array(T i) {
      for (int n=0; n<N; n++) data_[n]=i;
    };
    inline array(const array<N,T> &a) {
      memcpy(data_, a.data_, N*sizeof(T));
      //for (int n=0; n<N; n++) data_[n]=a[n];
    };
    // change your contents
    inline bool operator==(int i) {
      bool equal=true;
      for (int n=0; n<N; n++) equal*=(data_[n]==i);
      return equal;
    }
    inline bool operator==(const array<N,T>& a) const {
      bool equal=true;
      for (int n=0; n<N; n++) equal*=(data_[n]==a[n]);
      return equal;
    };
    inline bool operator!=(const array<N,T>& a) const {
      return !(operator==(a));
    };
    inline const array<N,T>& operator+=(const array<N,T> &a) {
      for (int n=0; n<N; n++) data_[n]+=a[n];
      return (*this);
    };
    // get your size (usefull for debugging)
    inline int size() const {
      return N;
    };
    // index access
    inline T& operator[](int n) {
      return data_[n];
    };
    inline const T& operator[](int n) const {
      return data_[n];
    };
    // usefull copyoperators
    inline array<N,T> &operator=(const array<N,T> &a) {
      for (int n=0; n<N; n++) data_[n]=a.data_[n];
      return (*this);
    };
    inline array<N,T> &operator=(T i) {
      for (int n=0; n<N; n++) data_[n]=i;
      return (*this);
    };
    // cast operator for basetype-pointer
    inline operator T*() {
      return data_;
    };
  };

  // print your array
  template <int N, class T>
  inline std::ostream &
  operator<< (std::ostream &o, const array<N,T>& a) {
    o << "[";
    for (int n=0; n<N-1; n++)
      o << a[n] << ",";
    o << a[N-1] << "]";
    return o;
  };
}

#endif // ARRAY_HH
