// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FIXEDVECTOR_HH__
#define __DUNE_FIXEDVECTOR_HH__

#include <iostream>
#include <math.h>

#include "misc.hh"
#include "fixedarray.hh"

namespace Dune {
  /** @defgroup Common Dune Common Module

     This module contains classes for general usage in dune, such as e.g.
     (small) dense matrices and vectors or containers.

     @{
   */


  //************************************************************************
  /*!
     Generic vector class for short vectors in d dimensions. Used e.g.
     for global or local coordinates.
   */
  template<int dim, class T = double>
  class Vec : public FixedArray<T, dim> {
  public:

    //! Constructor making uninitialized vector
    Vec() {}

    //! Constructor making vector from built-in array
    Vec (T* y) {for(int i=0; i<dim; i++) this->a[i]=y[i];}

    //! Constructor making vector with one coordinate set, others zeroed
    Vec (int k, T t)
    {
      for (int i=0; i<dim; i++) this->a[i] = 0;
      this->a[k] = t;
    }

    //! Constructor making vector with identical coordinates
    Vec (T t)
    {
      for (int i=0; i<dim; i++) this->a[i] = t;
    }

    //! operator () for read/write access to element of the vector
    T& operator() (int i) {return this->a[i];}

    //! operator () for read access to element of the vector
    const T& operator() (int i) const {return this->a[i];}

    //! operator+ adds two vectors
    Vec<dim,T>& operator+= (const Vec<dim,T>& b)
    {
      for (int i=0; i<dim; i++) this->a[i] += b.a[i];
      return *this;
    }

    //! Vector addition
    Vec<dim,T> operator+ (const Vec<dim,T>& b) const
    {
      Vec<dim,T> z = *this;
      return (z+=b);
    }

    //! operator- binary minus
    Vec<dim,T>& operator-= (const Vec<dim,T>& b)
    {
      for (int i=0; i<dim; i++) this->a[i] -= b.a[i];
      return *this;
    }

    //! Vector subtraction
    Vec<dim,T> operator- (const Vec<dim,T>& b) const
    {
      Vec<dim,T> z = *this;
      return (z-=b);
    }

    //! scalar product of two vectors with operator*
    T operator* (const Vec<dim,T>& b) const
    {
      T s=0; for (int i=0; i<dim; i++) s += this->a[i]*b.a[i];return s;
    }

    //! multiplication of vector with scalar
    Vec<dim,T> operator* (T k) const
    {
      Vec<dim,T> z; for (int i=0; i<dim; i++) z.a[i] =  k*this->a[i];return z;
    }

    //! multiplication assignment with scalar
    Vec<dim,T>& operator*= (T k)
    {
      for (int i=0; i<dim; i++) this->a[i] *= k;
      return *this;
    }

    //! 1 norm
    T norm1 () const
    {
      T s=0.0;
      for (int i=0; i<dim; i++) s += ABS(this->a[i]);
      return s;
    }

    //! 2 norm
    T norm2 () const
    {
      T s=0.0;
      for (int i=0; i<dim; i++) s += this->a[i]*this->a[i];
      return sqrt(s);
    }

    //! Infinity norm
    T norminfty () const
    {
      T s=0.0;
      for (int i=0; i<dim; i++)
        if (ABS(this->a[i])>s) s = ABS(this->a[i]);
      return s;
    }

    //! Euclidean distance of two vectors
    T distance (const Vec<dim,T>& b) const
    {
      T s=0.0;
      for (int i=0; i<dim; i++) s += (this->a[i]-b.a[i])*(this->a[i]-b.a[i]);
      return sqrt(s);
    }
  };

  //! multiplication of scalar with vector
  template<int dim, class T>
  inline Vec<dim,T> operator* (T k, Vec<dim,T> b)
  {
    Vec<dim,T> z; for (int i=0; i<dim; i++) z(i) = k*b(i);return z;
  }

  //! unary minus
  template<int dim, class T>
  inline Vec<dim,T> operator- (Vec<dim,T> b)
  {
    Vec<dim,T> z; for (int i=0; i<dim; i++) z(i) = -b(i);return z;
  }

  /** @} */

} // end namespace Dune


#endif
