// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __MATVEC_HH__
#define __MATVEC_HH__

#include <iostream>
#include <math.h>

#include "misc.hh"

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
  class Vec {
  public:

    //! remember the storage type
    typedef T MemberType;

    enum { n = (dim > 0) ? dim : 1 };

    //! know length
    enum { dimension = dim };

    //! Constructor making uninizialized vector
    Vec() {}

    //! Constructor making vector from built-in array
    Vec (T* y) {for(int i=0; i<n; i++) x[i]=y[i];}

    //! Constructor making vector with one coordinate set, others zeroed
    Vec (int k, T t)
    {
      for (int i=0; i<n; i++) x[i] = 0;
      x[k] = t;
    }

    //! Constructor making vector with identical coordinates
    Vec (T t)
    {
      for (int i=0; i<n; i++) x[i] = t;
    }

    //! assign component type to all components
    Vec<n,T>& operator= (const Vec<n,T>& b)
    {
      for (int i=0; i<n; i++) x[i] = b.x[i];
      return *this;
    }

    //! assign component type to all components
    Vec<n,T>& operator= (T t)
    {
      for (int i=0; i<n; i++) x[i]=t;
      return *this;
    }
    Vec<n,T>& operator= (T* t)
    {
      for (int i=0; i<n; i++) x[i]=t[i];
      return *this;
    }

    //! operator () for read/write access to element of the vector
    T& operator() (int i) {return x[i];}

    //! operator () for read access to element of the vector
    const T& operator() (int i) const {return x[i];}

    //! read only operation needed
    T read (int i) const { return x[i];}

    //! operator+ adds two vectors
    Vec<n,T>& operator+= (const Vec<n,T>& b)
    {
      for (int i=0; i<n; i++) x[i] += b.x[i];
      return *this;
    }
    Vec<n,T> operator+ (const Vec<n,T>& b) const
    {
      Vec<n,T> z = *this;
      return (z+=b);
    }

    //! operator- binary minus
    Vec<n,T>& operator-= (const Vec<n,T>& b)
    {
      for (int i=0; i<n; i++) x[i] -= b.x[i];
      return *this;
    }
    Vec<n,T> operator- (const Vec<n,T>& b) const
    {
      Vec<n,T> z = *this;
      return (z-=b);
    }

    //! scalar product of two vectors with operator*
    T operator* (const Vec<n,T>& b) const
    {
      T s=0; for (int i=0; i<n; i++) s += x[i]*b.x[i];return s;
    }

    //! multiplication of vector with scalar
    T operator* (T k) const
    {
      Vec<n,T> z; for (int i=0; i<n; i++) z.x[i] =  k*x[i];return z;
    }

    //! multiplication assignment with scalar
    Vec<n,T>& operator*= (T k)
    {
      for (int i=0; i<n; i++) x[i] *= k;
      return *this;
    }

    //! 1 norm
    T norm1 () const
    {
      T s=0.0;
      for (int i=0; i<n; i++) s += ABS(x[i]);
      return s;
    }

    //! 2 norm
    T norm2 () const
    {
      T s=0.0;
      for (int i=0; i<n; i++) s += x[i]*x[i];
      return sqrt(s);
    }

    //! \infty norm
    T norminfty () const
    {
      T s=0.0;
      for (int i=0; i<n; i++)
        if (ABS(x[i])>s) s = ABS(x[i]);
      return s;
    }

    //! Eucledian distance of two vectors
    T distance (const Vec<n,T>& b) const
    {
      T s=0.0;
      for (int i=0; i<n; i++) s += (x[i]-b.x[i])*(x[i]-b.x[i]);
      return sqrt(s);
    }

    void print (std::ostream& s, int indent) const
    {
      for (int k=0; k<indent; k++) s << " ";
      s << "Vec [ ";
      for (int i=0; i<n; i++) s << x[i] << " ";
      s << "]";
    }

  private:
    //! built-in array to hold the data.
    T x[n];
  };

  //! multiplication of scalar with vector
  template<int n, class T>
  inline Vec<n,T> operator* (T k, Vec<n,T> b)
  {
    Vec<n,T> z; for (int i=0; i<n; i++) z(i) = k*b(i);return z;
  }

  //! unary minus
  template<int n, class T>
  inline Vec<n,T> operator- (Vec<n,T> b)
  {
    Vec<n,T> z; for (int i=0; i<n; i++) z(i) = -b(i);return z;
  }

  template <int n, class T>
  inline std::ostream& operator<< (std::ostream& s, Vec<n,T>& v)
  {
    v.print(s,0);
    return s;
  }

  //************************************************************************
  /*!
     Generic vector class for short vectors in d dimensions. Used e.g. for global or local coordinates.
   */
  template<int n, int dim, class T = double>
  class Mat {
  public:
    enum { m = (dim > 0) ? dim : 1 };

    //! remember the dimension of the matrix
    enum { dimRow = dim };
    enum { dimCol = n };

    //! Constructor making uninizialized matrix
    Mat() {}

    //! Constructor setting all entries to t
    Mat(T t)
    {
      for(int j=0; j<m; j++)
        for (int i=0; i<n; i++) a[j](i) = t;
    }

    //! operator () for read/write access to element in matrix
    T& operator() (int i, int j) {return a[j](i);}

    //! operator () for read/write access to element in matrix
    const T& operator() (int i, int j) const {return a[j](i);}

    //! operator () for read/write access to column vector
    Vec<n,T>& operator() (int j) {return a[j];}

    //! matrix/vector multiplication
    Vec<n,T> operator* (const Vec<m,T>& x)
    {
      Vec<n,T> z(0.0);
      for (int j=0; j<m; j++)
        for (int i=0; i<n; i++) z(i) += a[j](i) * x.read(j);
      return z;
    }

    //! multiplication assignment with scalar
    Mat<n,m,T>& operator*= (T t)
    {
      for(int j=0; j<m; j++) a[j] *= t;
      return *this;
    }

    void print (std::ostream& s, int indent)
    {
      for (int k=0; k<indent; k++) s << " ";
      s << "Mat [n=" << n << ",m=" << m << "]" << std::endl;
      for (int i=0; i<n; i++)
      {
        for (int k=0; k<indent+2; k++) s << " ";
        s << "row " << i << " [ ";
        for (int j=0; j<m; j ++) s << this->operator()(i,j) << " ";
        s << "]" << std::endl;
      }
    }

    //! calculates the determinant of this matrix
    T determinant () const;

    //! calculates the inverse of this matrix and stores it in
    //! the parameter inverse, return is the determinant
    T invert (Mat<n,m,T>& inverse) const;

  private:
    //! built-in array to hold the data
    Vec<n,T> a[m];
  };

  namespace HelpMat {

    // calculation of determinat of matrix
    template <int row,int col, typename T>
    static inline T determinantMatrix (const Mat<row,col,T> &matrix)
    {
      std::cerr << "No default implementation of determinantMatrix for Mat<" << row << "," << col << "> !\n";
      abort();
      return 0.0;
    }

    template <typename T>
    static inline T determinantMatrix (const Mat<1,1,T> &matrix)
    {
      return matrix(0,0);
    }

    template <typename T>
    static inline T determinantMatrix (const Mat<2,2,T> &matrix)
    {
      T det = (matrix(0,0)*matrix(1,1) - matrix(0,1)*matrix(1,0));
      return det;
    }
    template <typename T>
    static inline T determinantMatrix (const Mat<3,3,T> &matrix)
    {
      // code generated by maple
      T t4  = matrix(0,0) * matrix(1,1);
      T t6  = matrix(0,0) * matrix(1,2);
      T t8  = matrix(0,1) * matrix(1,0);
      T t10 = matrix(0,2) * matrix(1,0);
      T t12 = matrix(0,1) * matrix(2,0);
      T t14 = matrix(0,2) * matrix(2,0);

      T det = (t4*matrix(2,2)-t6*matrix(2,1)-t8*matrix(2,2)+
               t10*matrix(2,1)+t12*matrix(1,2)-t14*matrix(1,1));
      return det;
    }
    // calculation of inverse of matrix
    template <int row,int col, typename T>
    static inline T invertMatrix (const Mat<row,col,T> &matrix, Mat<row,col,T> &inverse)
    {
      std::cerr << "No default implementation of invertMatrix for Mat<" << row << "," << col << "> !\n";
      abort();
      return 0.0;
    }

    // invert scalar
    template <typename T>
    static inline T invertMatrix (const Mat<1,1,T> &matrix, Mat<1,1,T> &inverse)
    {
      inverse(0,0) = 1.0/matrix(0,0);
      return (matrix(0,0));
    }


    // invert 2x2 Matrix
    template <typename T>
    static inline T invertMatrix (const Mat<2,2,T> &matrix, Mat<2,2,T> &inverse)
    {
      // code generated by maple
      T det = (matrix(0,0)*matrix(1,1) - matrix(0,1)*matrix(1,0));
      T det_1 = 1.0/det;
      inverse(0,0) =   matrix(1,1) * det_1;
      inverse(0,1) = - matrix(0,1) * det_1;
      inverse(1,0) = - matrix(1,0) * det_1;
      inverse(1,1) =   matrix(0,0) * det_1;
      return det;
    }

    // invert 3x3 Matrix
    template <typename T>
    static inline T invertMatrix (const Mat<3,3,T> &matrix, Mat<3,3,T> &inverse)
    {
      // code generated by maple
      T t4  = matrix(0,0) * matrix(1,1);
      T t6  = matrix(0,0) * matrix(1,2);
      T t8  = matrix(0,1) * matrix(1,0);
      T t10 = matrix(0,2) * matrix(1,0);
      T t12 = matrix(0,1) * matrix(2,0);
      T t14 = matrix(0,2) * matrix(2,0);

      T det = (t4*matrix(2,2)-t6*matrix(2,1)-t8*matrix(2,2)+
               t10*matrix(2,1)+t12*matrix(1,2)-t14*matrix(1,1));
      T t17 = 1.0/det;

      inverse(0,0) =  (matrix(1,1) * matrix(2,2) - matrix(1,2) * matrix(2,1))*t17;
      inverse(0,1) = -(matrix(0,1) * matrix(2,2) - matrix(0,2) * matrix(2,1))*t17;
      inverse(0,2) =  (matrix(0,1) * matrix(1,2) - matrix(0,2) * matrix(1,1))*t17;
      inverse(1,0) = -(matrix(1,0) * matrix(2,2) - matrix(1,2) * matrix(2,0))*t17;
      inverse(1,1) =  (matrix(0,0) * matrix(2,2) - t14) * t17;
      inverse(1,2) = -(t6-t10) * t17;
      inverse(2,0) =  (matrix(1,0) * matrix(2,1) - matrix(1,1) * matrix(2,0)) * t17;
      inverse(2,1) = -(matrix(0,0) * matrix(2,1) - t12) * t17;
      inverse(2,2) =  (t4-t8) * t17;

      return det;
    }

  } // end namespace HelpMat
  template <int n, int m, class T>
  inline std::ostream& operator<< (std::ostream& s, Mat<n,m,T>& A)
  {
    A.print(s,0);
    return s;
  }

  // implementation od determinant
  template <int n, int m, class T>
  inline T Mat<n,m,T>::determinant () const
  {
    return HelpMat::determinantMatrix(*this);
  }

  // implementation of invert
  template <int n, int m, class T>
  inline T Mat<n,m,T>::invert (Mat<n,m,T>& inverse) const
  {
    return HelpMat::invertMatrix( *this , inverse);
  }


  /** @} */

} // end namespace Dune


#endif
