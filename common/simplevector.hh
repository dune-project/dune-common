// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SIMPLEVECTOR_HH__
#define __DUNE_SIMPLEVECTOR_HH__


//***********************************************************************
//
//  implementation of peter array
//
//***********************************************************************

#include <iostream>
#include <iomanip>
#include <string>
#include <rpc/xdr.h>

#include "array.hh"

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  //! A simple vector class
  template <class T>
  class SimpleVector : public Array<T> {
  public:
    //! make empty vector
    SimpleVector() {};

    //! make array with m components
    SimpleVector(int m) : Array<T>::Array(m) {}

    //! assignment from scalar
    SimpleVector<T>& operator= (const T t)
    {
      for (int i=0; i<this->n; ++i) this->p[i] = t;
      return *this;
    }

    //! Addition
    SimpleVector<T>& operator+= (const SimpleVector<T>& vec)
    {
      for (int i=0; i<this->n; ++i)
        this->p[i] += vec[i];
      return *this;
    }

    //! Subtraction
    SimpleVector<T>& operator-= (const SimpleVector<T>& vec)
    {
      for (int i=0; i<this->n; ++i)
        this->p[i] -= vec[i];
      return *this;
    }

    //! scalar product of two vectors, no check for size !
    T ddot (const SimpleVector<T>& x)
    {
      T sum = 0;
      for (int i=0; i<this->n; ++i) sum += this->p[i]*x.p[i];
      return sum;
    }

    //! add scalar times other vector
    void daxpy (T a, const SimpleVector<T>& x)
    {
      for (int i=0; i<this->n; ++i) this->p[i] += a*x.p[i];
    }

    //! Vector subtraction
    friend SimpleVector<T> operator-(const SimpleVector<T>& v1,
                                     const SimpleVector<T>& v2) {
      assert(v1.size() == v2.size());
      SimpleVector<T> out(v1.size());
      for (int i=0; i<out.size(); i++)
        out[i] = v1[i] - v2[i];

      return out;
    }

  };

  /** @} */

} // end namespace Dune

#endif
