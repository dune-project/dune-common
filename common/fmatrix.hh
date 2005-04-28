// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_FMATRIX_HH
#define DUNE_FMATRIX_HH

#include <math.h>
#include <complex>
#include <iostream>
#include "exceptions.hh"
#include "fvector.hh"
#include "precision.hh"

namespace Dune {

  /**
              @addtogroup DenseMatVec
              @{
   */

  /*! \file

     \brief  This file implements a matrix constructed from a given type
     representing a field and compile-time given number of rows and columns.
   */

  template<class K, int n, int m> class FieldMatrix;

  /** @brief Error thrown if operations of a FieldMatrix fail. */
  class FMatrixError : public Exception {};

  // template meta program for assignment from scalar
  template<int I>
  struct fmmeta_assignscalar {
    template<class T, class K>
    static void assignscalar (T* x, const K& k)
    {
      fmmeta_assignscalar<I-1>::assignscalar(x,k);
      x[I] = k;
    }
  };
  template<>
  struct fmmeta_assignscalar<0> {
    template<class T, class K>
    static void assignscalar (T* x, const K& k)
    {
      x[0] = k;
    }
  };

  // template meta program for operator+=
  template<int I>
  struct fmmeta_plusequal {
    template<class T>
    static void plusequal (T& x, const T& y)
    {
      x[I] += y[I];
      fmmeta_plusequal<I-1>::plusequal(x,y);
    }
  };
  template<>
  struct fmmeta_plusequal<0> {
    template<class T>
    static void plusequal (T& x, const T& y)
    {
      x[0] += y[0];
    }
  };

  // template meta program for operator-=
  template<int I>
  struct fmmeta_minusequal {
    template<class T>
    static void minusequal (T& x, const T& y)
    {
      x[I] -= y[I];
      fmmeta_minusequal<I-1>::minusequal(x,y);
    }
  };
  template<>
  struct fmmeta_minusequal<0> {
    template<class T>
    static void minusequal (T& x, const T& y)
    {
      x[0] -= y[0];
    }
  };

  // template meta program for operator*=
  template<int I>
  struct fmmeta_multequal {
    template<class T, class K>
    static void multequal (T& x, const K& k)
    {
      x[I] *= k;
      fmmeta_multequal<I-1>::multequal(x,k);
    }
  };
  template<>
  struct fmmeta_multequal<0> {
    template<class T, class K>
    static void multequal (T& x, const K& k)
    {
      x[0] *= k;
    }
  };

  // template meta program for operator/=
  template<int I>
  struct fmmeta_divequal {
    template<class T, class K>
    static void divequal (T& x, const K& k)
    {
      x[I] /= k;
      fmmeta_divequal<I-1>::divequal(x,k);
    }
  };
  template<>
  struct fmmeta_divequal<0> {
    template<class T, class K>
    static void divequal (T& x, const K& k)
    {
      x[0] /= k;
    }
  };

  // template meta program for dot
  template<int I>
  struct fmmeta_dot {
    template<class X, class Y, class K>
    static K dot (const X& x, const Y& y)
    {
      return x[I]*y[I] + fmmeta_dot<I-1>::template dot<X,Y,K>(x,y);
    }
  };
  template<>
  struct fmmeta_dot<0> {
    template<class X, class Y, class K>
    static K dot (const X& x, const Y& y)
    {
      return x[0]*y[0];
    }
  };

  // template meta program for umv(x,y)
  template<int I>
  struct fmmeta_umv {
    template<class Mat, class X, class Y, int c>
    static void umv (const Mat& A, const X& x, Y& y)
    {
      typedef typename Mat::row_type R;
      typedef typename Mat::field_type K;
      y[I] += fmmeta_dot<c>::template dot<R,X,K>(A[I],x);
      fmmeta_umv<I-1>::template umv<Mat,X,Y,c>(A,x,y);
    }
  };
  template<>
  struct fmmeta_umv<0> {
    template<class Mat, class X, class Y, int c>
    static void umv (const Mat& A, const X& x, Y& y)
    {
      typedef typename Mat::row_type R;
      typedef typename Mat::field_type K;
      y[0] += fmmeta_dot<c>::template dot<R,X,K>(A[0],x);
    }
  };

  // template meta program for mmv(x,y)
  template<int I>
  struct fmmeta_mmv {
    template<class Mat, class X, class Y, int c>
    static void mmv (const Mat& A, const X& x, Y& y)
    {
      typedef typename Mat::row_type R;
      typedef typename Mat::field_type K;
      y[I] -= fmmeta_dot<c>::template dot<R,X,K>(A[I],x);
      fmmeta_mmv<I-1>::template mmv<Mat,X,Y,c>(A,x,y);
    }
  };
  template<>
  struct fmmeta_mmv<0> {
    template<class Mat, class X, class Y, int c>
    static void mmv (const Mat& A, const X& x, Y& y)
    {
      typedef typename Mat::row_type R;
      typedef typename Mat::field_type K;
      y[0] -= fmmeta_dot<c>::template dot<R,X,K>(A[0],x);
    }
  };

  template<class K, int n, int m, class X, class Y>
  inline void fm_mmv (const FieldMatrix<K,n,m>& A,  const X& x, Y& y)
  {
    for (int i=0; i<n; i++)
      for (int j=0; j<m; j++)
        y[i] -= A[i][j]*x[j];
  }

  template<class K>
  inline void fm_mmv (const FieldMatrix<K,1,1>& A, const FieldVector<K,1>& x, FieldVector<K,1>& y)
  {
    y[0] -= A[0][0]*x[0];
  }

  // template meta program for usmv(x,y)
  template<int I>
  struct fmmeta_usmv {
    template<class Mat, class K, class X, class Y, int c>
    static void usmv (const Mat& A, const K& alpha, const X& x, Y& y)
    {
      typedef typename Mat::row_type R;
      y[I] += alpha*fmmeta_dot<c>::template dot<R,X,K>(A[I],x);
      fmmeta_usmv<I-1>::template usmv<Mat,K,X,Y,c>(A,alpha,x,y);
    }
  };
  template<>
  struct fmmeta_usmv<0> {
    template<class Mat, class K,  class X, class Y, int c>
    static void usmv (const Mat& A, const K& alpha, const X& x, Y& y)
    {
      typedef typename Mat::row_type R;
      y[0] += alpha*fmmeta_dot<c>::template dot<R,X,K>(A[0],x);
    }
  };

  // conjugate komplex does nothing for non-complex types
  template<class K>
  inline K fm_ck (const K& k)
  {
    return k;
  }

  // conjugate komplex
  template<class K>
  inline std::complex<K> fm_ck (const std::complex<K>& c)
  {
    return std::complex<K>(c.real(),-c.imag());
  }


  //! solve small system
  template<class K, int n, class V>
  void fm_solve (const FieldMatrix<K,n,n>& Ain,  V& x, const V& b)
  {
    // make a copy of a to store factorization
    FieldMatrix<K,n,n> A(Ain);

    // Gaussian elimination with maximum column pivot
    double norm=A.infinity_norm_real();     // for relative thresholds
    double pivthres = std::max(FMatrixPrecision<>::absolute_limit(),norm*FMatrixPrecision<>::pivoting_limit());
    double singthres = std::max(FMatrixPrecision<>::absolute_limit(),norm*FMatrixPrecision<>::singular_limit());
    V& rhs = x;          // use x to store rhs
    rhs = b;             // copy data

    // elimination phase
    for (int i=0; i<n; i++)      // loop over all rows
    {
      double pivmax=fvmeta_absreal(A[i][i]);

      // pivoting ?
      if (pivmax<pivthres)
      {
        // compute maximum of row
        int imax=i; double abs;
        for (int k=i+1; k<n; k++)
          if ((abs=fvmeta_absreal(A[k][i]))>pivmax)
          {
            pivmax = abs; imax = k;
          }
        // swap rows
        if (imax!=i)
          for (int j=i; j<n; j++)
            std::swap(A[i][j],A[imax][j]);
      }

      // singular ?
      if (pivmax<singthres)
        DUNE_THROW(FMatrixError,"matrix is singular");

      // eliminate
      for (int k=i+1; k<n; k++)
      {
        K factor = -A[k][i]/A[i][i];
        for (int j=i+1; j<n; j++)
          A[k][j] += factor*A[i][j];
        rhs[k] += factor*rhs[i];
      }
    }

    // backsolve
    for (int i=n-1; i>=0; i--)
    {
      for (int j=i+1; j<n; j++)
        rhs[i] -= A[i][j]*x[j];
      x[i] = rhs[i]/A[i][i];
    }
  }

  //! special case for 1x1 matrix, x and b may be identical
  template<class K, class V>
  inline void fm_solve (const FieldMatrix<K,1,1>& A,  V& x, const V& b)
  {
#ifdef DUNE_FMatrix_WITH_CHECKING
    if (fvmeta_absreal(A[0][0])<FMatrixPrecision<>::absolute_limit())
      DUNE_THROW(FMatrixError,"matrix is singular");
#endif
    x[0] = b[0]/A[0][0];
  }

  //! special case for 2x2 matrix, x and b may be identical
  template<class K, class V>
  inline void fm_solve (const FieldMatrix<K,2,2>& A,  V& x, const V& b)
  {
#ifdef DUNE_FMatrix_WITH_CHECKING
    K detinv = A[0][0]*A[1][1]-A[0][1]*A[1][0];
    if (fvmeta_absreal(detinv)<FMatrixPrecision<>::absolute_limit())
      DUNE_THROW(FMatrixError,"matrix is singular");
    detinv = 1/detinv;
#else
    K detinv = 1.0/(A[0][0]*A[1][1]-A[0][1]*A[1][0]);
#endif

    K temp = b[0];
    x[0] = detinv*(A[1][1]*b[0]-A[0][1]*b[1]);
    x[1] = detinv*(A[0][0]*b[1]-A[1][0]*temp);
  }



  //! compute inverse
  template<class K, int n>
  void fm_invert (FieldMatrix<K,n,n>& B)
  {
    FieldMatrix<K,n,n> A(B);
    FieldMatrix<K,n,n>& L=A;
    FieldMatrix<K,n,n>& U=A;

    double norm=A.infinity_norm_real();     // for relative thresholds
    double pivthres = std::max(FMatrixPrecision<>::absolute_limit(),norm*FMatrixPrecision<>::pivoting_limit());
    double singthres = std::max(FMatrixPrecision<>::absolute_limit(),norm*FMatrixPrecision<>::singular_limit());

    // LU decomposition of A in A
    for (int i=0; i<n; i++)      // loop over all rows
    {
      double pivmax=fvmeta_absreal(A[i][i]);

      // pivoting ?
      if (pivmax<pivthres)
      {
        // compute maximum of column
        int imax=i; double abs;
        for (int k=i+1; k<n; k++)
          if ((abs=fvmeta_absreal(A[k][i]))>pivmax)
          {
            pivmax = abs; imax = k;
          }
        // swap rows
        if (imax!=i)
          for (int j=i; j<n; j++)
            std::swap(A[i][j],A[imax][j]);
      }

      // singular ?
      if (pivmax<singthres)
        DUNE_THROW(FMatrixError,"matrix is singular");

      // eliminate
      for (int k=i+1; k<n; k++)
      {
        K factor = A[k][i]/A[i][i];
        L[k][i] = factor;
        for (int j=i+1; j<n; j++)
          A[k][j] -= factor*A[i][j];
      }
    }

    // initialize inverse
    B = 0;
    for (int i=0; i<n; i++) B[i][i] = 1;

    // L Y = I; multiple right hand sides
    for (int i=0; i<n; i++)
      for (int j=0; j<i; j++)
        for (int k=0; k<n; k++)
          B[i][k] -= L[i][j]*B[j][k];

    // U A^{-1} = Y
    for (int i=n-1; i>=0; i--)
      for (int k=0; k<n; k++)
      {
        for (int j=i+1; j<n; j++)
          B[i][k] -= U[i][j]*B[j][k];
        B[i][k] /= U[i][i];
      }
  }

  //! compute inverse n=1
  template<class K>
  void fm_invert (FieldMatrix<K,1,1>& A)
  {
#ifdef DUNE_FMatrix_WITH_CHECKING
    if (fvmeta_absreal(A[0][0])<FMatrixPrecision<>::absolute_limit())
      DUNE_THROW(FMatrixError,"matrix is singular");
#endif
    A[0][0] = 1/A[0][0];
  }

  //! compute inverse n=2
  template<class K>
  void fm_invert (FieldMatrix<K,2,2>& A)
  {
    K detinv = A[0][0]*A[1][1]-A[0][1]*A[1][0];
#ifdef DUNE_FMatrix_WITH_CHECKING
    if (fvmeta_absreal(detinv)<FMatrixPrecision<>::absolute_limit())
      DUNE_THROW(FMatrixError,"matrix is singular");
#endif
    detinv = 1/detinv;

    K temp=A[0][0];
    A[0][0] =  A[1][1]*detinv;
    A[0][1] = -A[0][1]*detinv;
    A[1][0] = -A[1][0]*detinv;
    A[1][1] =  temp*detinv;
  }

  //! left multiplication with matrix
  template<class K, int n, int m>
  void fm_leftmultiply (const FieldMatrix<K,n,n>& M, FieldMatrix<K,n,m>& A)
  {
    FieldMatrix<K,n,m> C(A);

    for (int i=0; i<n; i++)
      for (int j=0; j<m; j++)
      {
        A[i][j] = 0;
        for (int k=0; k<n; k++)
          A[i][j] += M[i][k]*C[k][j];
      }
  }

  //! left multiplication with matrix, n=1
  template<class K>
  void fm_leftmultiply (const FieldMatrix<K,1,1>& M, FieldMatrix<K,1,1>& A)
  {
    A[0][0] *= M[0][0];
  }

  //! left multiplication with matrix, n=2
  template<class K>
  void fm_leftmultiply (const FieldMatrix<K,2,2>& M, FieldMatrix<K,2,2>& A)
  {
    FieldMatrix<K,2,2> C(A);

    A[0][0] = M[0][0]*C[0][0] + M[0][1]*C[1][0];
    A[0][1] = M[0][0]*C[0][1] + M[0][1]*C[1][1];
    A[1][0] = M[1][0]*C[0][0] + M[1][1]*C[1][0];
    A[1][1] = M[1][0]*C[0][1] + M[1][1]*C[1][1];
  }

  //! right multiplication with matrix
  template<class K, int n, int m>
  void fm_rightmultiply (const FieldMatrix<K,m,m>& M, FieldMatrix<K,n,m>& A)
  {
    FieldMatrix<K,n,m> C(A);

    for (int i=0; i<n; i++)
      for (int j=0; j<m; j++)
      {
        A[i][j] = 0;
        for (int k=0; k<m; k++)
          A[i][j] += C[i][k]*M[k][j];
      }
  }

  //! right multiplication with matrix, n=1
  template<class K>
  void fm_rightmultiply (const FieldMatrix<K,1,1>& M, FieldMatrix<K,1,1>& A)
  {
    A[0][0] *= M[0][0];
  }

  //! right multiplication with matrix, n=2
  template<class K>
  void fm_rightmultiply (const FieldMatrix<K,2,2>& M, FieldMatrix<K,2,2>& A)
  {
    FieldMatrix<K,2,2> C(A);

    A[0][0] = C[0][0]*M[0][0] + C[0][1]*M[1][0];
    A[0][1] = C[0][0]*M[0][1] + C[0][1]*M[1][1];
    A[1][0] = C[1][0]*M[0][0] + C[1][1]*M[1][0];
    A[1][1] = C[1][0]*M[0][1] + C[1][1]*M[1][1];
  }

  /**
      @brief A dense n x m matrix.

     Matrices represent linear maps from a vector space V to a vector space W.
       This class represents such a linear map by storing a two-dimensional
       array of numbers of a given field type K. The number of rows and
       columns is given at compile time.

           Implementation of all members uses template meta programs where appropriate
   */
  template<class K, int n, int m>
  class FieldMatrix
  {
  public:
    // standard constructor and everything is sufficient ...

    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain. This is 1.
      blocklevel = 1
    };

    //! Each row is implemented by a field vector
    typedef FieldVector<K,m> row_type;

    //! export size
    enum {
      //! The number of rows.
      rows = n,
      //! The number of columns.
      cols = m
    };

    //===== constructors
    /** \brief Default constructor
     */
    FieldMatrix () {}

    /** \brief Constructor initializing the whole matrix with a scalar
     */
    FieldMatrix (const K& k)
    {
      for (int i=0; i<n; i++) p[i] = k;
    }

    //===== random access interface to rows of the matrix

    //! random access to the rows
    row_type& operator[] (int i)
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(FMatrixError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const row_type& operator[] (int i) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(FMatrixError,"index out of range");
#endif
      return p[i];
    }


    //===== iterator interface to rows of the matrix
    //! Iterator class for sequential access
    typedef FieldIterator<FieldMatrix<K,n,m>,row_type> Iterator;
    //! typedef for stl compliant access
    typedef Iterator iterator;
    //! rename the iterators for easier access
    typedef Iterator RowIterator;
    //! rename the iterators for easier access
    typedef typename row_type::Iterator ColIterator;

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(*this,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(*this,n);
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(*this,n-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(*this,-1);
    }

    //! Iterator class for sequential access
    typedef FieldIterator<const FieldMatrix<K,n,m>,const row_type> ConstIterator;
    //! typedef for stl compliant access
    typedef ConstIterator const_iterator;
    //! rename the iterators for easier access
    typedef ConstIterator ConstRowIterator;
    //! rename the iterators for easier access
    typedef typename row_type::ConstIterator ConstColIterator;

    //! begin iterator
    ConstIterator begin () const
    {
      return ConstIterator(*this,0);
    }

    //! end iterator
    ConstIterator end () const
    {
      return ConstIterator(*this,n);
    }

    //! begin iterator
    ConstIterator rbegin () const
    {
      return ConstIterator(*this,n-1);
    }

    //! end iterator
    ConstIterator rend () const
    {
      return ConstIterator(*this,-1);
    }

    //===== assignment from scalar
    FieldMatrix& operator= (const K& k)
    {
      fmmeta_assignscalar<n-1>::assignscalar(p,k);
      return *this;
    }

    //===== vector space arithmetic

    //! vector space addition
    FieldMatrix& operator+= (const FieldMatrix& y)
    {
      fmmeta_plusequal<n-1>::plusequal(*this,y);
      return *this;
    }

    //! vector space subtraction
    FieldMatrix& operator-= (const FieldMatrix& y)
    {
      fmmeta_minusequal<n-1>::minusequal(*this,y);
      return *this;
    }

    //! vector space multiplication with scalar
    FieldMatrix& operator*= (const K& k)
    {
      fmmeta_multequal<n-1>::multequal(*this,k);
      return *this;
    }

    //! vector space division by scalar
    FieldMatrix& operator/= (const K& k)
    {
      fmmeta_divequal<n-1>::divequal(*this,k);
      return *this;
    }

    //===== linear maps

    //! y += A x
    template<class X, class Y>
    void umv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
#endif
      fmmeta_umv<n-1>::template umv<FieldMatrix,X,Y,m-1>(*this,x,y);
    }

    //! y += A^T x
    template<class X, class Y>
    void umtv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += p[i][j]*x[i];
    }

    //! y += A^H x
    template<class X, class Y>
    void umhv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += fm_ck(p[i][j])*x[i];
    }

    //! y -= A x
    template<class X, class Y>
    void mmv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
#endif
      fmmeta_mmv<n-1>::template mmv<FieldMatrix,X,Y,m-1>(*this,x,y);
      //fm_mmv(*this,x,y);
    }

    //! y -= A^T x
    template<class X, class Y>
    void mmtv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] -= p[i][j]*x[i];
    }

    //! y -= A^H x
    template<class X, class Y>
    void mmhv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] -= fm_ck(p[i][j])*x[i];
    }

    //! y += alpha A x
    template<class X, class Y>
    void usmv (const K& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
#endif
      fmmeta_usmv<n-1>::template usmv<FieldMatrix,K,X,Y,m-1>(*this,alpha,x,y);
    }

    //! y += alpha A^T x
    template<class X, class Y>
    void usmtv (const K& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += alpha*p[i][j]*x[i];
    }

    //! y += alpha A^H x
    template<class X, class Y>
    void usmhv (const K& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += alpha*fm_ck(p[i][j])*x[i];
    }

    //===== norms

    //! frobenius norm: sqrt(sum over squared values of entries)
    double frobenius_norm () const
    {
      double sum=0;
      for (int i=0; i<n; ++i) sum += p[i].two_norm2();
      return sqrt(sum);
    }

    //! square of frobenius norm, need for block recursion
    double frobenius_norm2 () const
    {
      double sum=0;
      for (int i=0; i<n; ++i) sum += p[i].two_norm2();
      return sum;
    }

    //! infinity norm (row sum norm, how to generalize for blocks?)
    double infinity_norm () const
    {
      double max=0;
      for (int i=0; i<n; ++i) max = std::max(max,p[i].one_norm());
      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      double max=0;
      for (int i=0; i<n; ++i) max = std::max(max,p[i].one_norm_real());
      return max;
    }

    //===== solve

    /** \brief Solve system A x = b
     *
     * \exception FMatrixError if the matrix is singular
     */
    template<class V>
    void solve (V& x, const V& b) const
    {
      fm_solve(*this,x,b);
    }

    /** \brief Compute inverse
     *
     * \exception FMatrixError if the matrix is singular
     */
    void invert ()
    {
      fm_invert(*this);
    }

    //! calculates the determinant of this matrix
    K determinant () const;

    //! Multiplies M from the left to this matrix
    FieldMatrix& leftmultiply (const FieldMatrix<K,n,n>& M)
    {
      fm_leftmultiply(M,*this);
      return *this;
    }

    //! Multiplies M from the right to this matrix
    FieldMatrix& rightmultiply (const FieldMatrix<K,n,n>& M)
    {
      fm_rightmultiply(M,*this);
      return *this;
    }


    //===== sizes

    //! number of blocks in row direction
    int N () const
    {
      return n;
    }

    //! number of blocks in column direction
    int M () const
    {
      return m;
    }

    //! row dimension of block r
    int rowdim (int r) const
    {
      return 1;
    }

    //! col dimension of block c
    int coldim (int c) const
    {
      return 1;
    }

    //! dimension of the destination vector space
    int rowdim () const
    {
      return n;
    }

    //! dimension of the source vector space
    int coldim () const
    {
      return m;
    }

    //===== query

    //! return true when (i,j) is in pattern
    bool exists (int i, int j) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(FMatrixError,"index out of range");
      if (j<0 || i>=m) DUNE_THROW(FMatrixError,"index out of range");
#endif
      return true;
    }

    //===== conversion operator

    /** \brief Sends the matrix to an output stream */
    void print (std::ostream& s) const
    {
      for (int i=0; i<n; i++)
        s << p[i] << std::endl;
    }

    /** \brief Sends the matrix to an output stream */
    friend std::ostream& operator<< (std::ostream& s, const FieldMatrix<K,n,m>& a)
    {
      a.print(s);
      return s;
    }

  private:
    // the data, very simply a built in array with rowwise ordering
    row_type p[n];
  };


  namespace HelpMat {

    // calculation of determinat of matrix
    template <class K, int row,int col>
    static inline K determinantMatrix (const FieldMatrix<K,row,col> &matrix)
    {
      if (row!=col)
        DUNE_THROW(FMatrixError, "There is no determinant for a " << row << "x" << col << " matrix!");

      DUNE_THROW(FMatrixError, "No implementation of determinantMatrix "
                 << "for FieldMatrix<" << row << "," << col << "> !");

      return 0.0;
    }

    template <typename K>
    static inline K determinantMatrix (const FieldMatrix<K,1,1> &matrix)
    {
      return matrix[0][0];
    }

    template <typename K>
    static inline K determinantMatrix (const FieldMatrix<K,2,2> &matrix)
    {
      return matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0];
    }

    template <typename K>
    static inline K determinantMatrix (const FieldMatrix<K,3,3> &matrix)
    {
      // code generated by maple
      K t4  = matrix[0][0] * matrix[1][1];
      K t6  = matrix[0][0] * matrix[1][2];
      K t8  = matrix[0][1] * matrix[1][0];
      K t10 = matrix[0][2] * matrix[1][0];
      K t12 = matrix[0][1] * matrix[2][0];
      K t14 = matrix[0][2] * matrix[2][0];

      K det = (t4*matrix[2][2]-t6*matrix[2][1]-t8*matrix[2][2]+
               t10*matrix[2][1]+t12*matrix[1][2]-t14*matrix[1][1]);
      return det;
    }

  } // end namespace HelpMat

  // implementation of the determinant
  template <class K, int n, int m>
  inline K FieldMatrix<K,n,m>::determinant () const
  {
    return HelpMat::determinantMatrix(*this);
  }

#ifdef USE_DEPRECATED_K1
  /** \brief Special type for 1x1 matrices
   */
  template<class K>
  class K11Matrix
  {
  public:
    // standard constructor and everything is sufficient ...

    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain.
      //! This is always one for this type.
      blocklevel = 1
    };

    //! export size
    enum {
      //! \brief The number of rows.
      //! This is always one for this type.
      rows = 1,
      //! \brief The number of columns.
      //! This is always one for this type.
      cols = 1
    };

    //===== assignment from scalar

    K11Matrix& operator= (const K& k)
    {
      a = k;
      return *this;
    }

    //===== vector space arithmetic

    //! vector space addition
    K11Matrix& operator+= (const K& y)
    {
      a += y;
      return *this;
    }

    //! vector space subtraction
    K11Matrix& operator-= (const K& y)
    {
      a -= y;
      return *this;
    }

    //! vector space multiplication with scalar
    K11Matrix& operator*= (const K& k)
    {
      a *= k;
      return *this;
    }

    //! vector space division by scalar
    K11Matrix& operator/= (const K& k)
    {
      a /= k;
      return *this;
    }

    //===== linear maps

    //! y += A x
    void umv (const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p += a * x.p;
    }

    //! y += A^T x
    void umtv (const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p += a * x.p;
    }

    //! y += A^H x
    void umhv (const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p += fm_ck(a) * x.p;
    }

    //! y -= A x
    void mmv (const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p -= a * x.p;
    }

    //! y -= A^T x
    void mmtv (const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p -= a * x.p;
    }

    //! y -= A^H x
    void mmhv (const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p -= fm_ck(a) * x.p;
    }

    //! y += alpha A x
    void usmv (const K& alpha, const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p += alpha * a * x.p;
    }

    //! y += alpha A^T x
    void usmtv (const K& alpha, const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p += alpha * a * x.p;
    }

    //! y += alpha A^H x
    void usmhv (const K& alpha, const K1Vector<K>& x, K1Vector<K>& y) const
    {
      y.p += alpha * fm_ck(a) * x.p;
    }

    //===== norms

    //! frobenius norm: sqrt(sum over squared values of entries)
    double frobenius_norm () const
    {
      return sqrt(fvmeta_abs2(a));
    }

    //! square of frobenius norm, need for block recursion
    double frobenius_norm2 () const
    {
      return fvmeta_abs2(a);
    }

    //! infinity norm (row sum norm, how to generalize for blocks?)
    double infinity_norm () const
    {
      return fvmeta_abs(a);
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      return fvmeta_abs_real(a);
    }

    //===== solve

    //! Solve system A x = b
    void solve (K1Vector<K>& x, const K1Vector<K>& b) const
    {
      x.p = b.p/a;
    }

    //! compute inverse
    void invert ()
    {
      a = 1/a;
    }

    //! left multiplication
    K11Matrix& leftmultiply (const K11Matrix<K>& M)
    {
      a *= M.a;
      return *this;
    }

    //! left multiplication
    K11Matrix& rightmultiply (const K11Matrix<K>& M)
    {
      a *= M.a;
      return *this;
    }


    //===== sizes

    //! number of blocks in row direction
    int N () const
    {
      return 1;
    }

    //! number of blocks in column direction
    int M () const
    {
      return 1;
    }

    //! row dimension of block r
    int rowdim (int r) const
    {
      return 1;
    }

    //! col dimension of block c
    int coldim (int c) const
    {
      return 1;
    }

    //! dimension of the destination vector space
    int rowdim () const
    {
      return 1;
    }

    //! dimension of the source vector space
    int coldim () const
    {
      return 1;
    }

    //===== query

    //! return true when (i,j) is in pattern
    bool exists (int i, int j) const
    {
      return i==0 && j==0;
    }

    //===== conversion operator

    operator K () const {return a;}

  private:
    // the data, just a single scalar
    K a;
  };
#endif // USE_DEPRECATED_K1

  /** \brief Special type for 1x1 matrices
   */
  template<class K>
  class FieldMatrix<K,1,1>
  {
  public:
    // standard constructor and everything is sufficient ...

    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain.
      //! This is always one for this type.
      blocklevel = 1
    };

    //! Each row is implemented by a field vector
    typedef FieldVector<K,1> row_type;

    //! export size
    enum {
      //! \brief The number of rows.
      //! This is always one for this type.
      rows = 1,
      n = 1,
      //! \brief The number of columns.
      //! This is always one for this type.
      cols = 1,
      m = 1
    };

    //===== constructors
    /** \brief Default constructor
     */
    FieldMatrix () {}

    /** \brief Constructor initializing the whole matrix with a scalar
     */
    FieldMatrix (const K& k)
    {
      a = k;
    }

    //===== random access interface to rows of the matrix

    //! random access to the rows
    row_type& operator[] (int i)
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(FMatrixError,"index out of range");
#endif
      return a;
    }

    //! same for read only access
    const row_type& operator[] (int i) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(FMatrixError,"index out of range");
#endif
      return a;
    }

    //===== iterator interface to rows of the matrix
    //! Iterator class for sequential access
    typedef FieldIterator<FieldMatrix<K,n,m>,row_type> Iterator;
    //! typedef for stl compliant access
    typedef Iterator iterator;
    //! rename the iterators for easier access
    typedef Iterator RowIterator;
    //! rename the iterators for easier access
    typedef typename row_type::Iterator ColIterator;

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(*this,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(*this,n);
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(*this,n-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(*this,-1);
    }

    //! Iterator class for sequential access
    typedef FieldIterator<const FieldMatrix<K,n,m>,const row_type> ConstIterator;
    //! typedef for stl compliant access
    typedef ConstIterator const_iterator;
    //! rename the iterators for easier access
    typedef ConstIterator ConstRowIterator;
    //! rename the iterators for easier access
    typedef typename row_type::ConstIterator ConstColIterator;

    //! begin iterator
    ConstIterator begin () const
    {
      return ConstIterator(*this,0);
    }

    //! end iterator
    ConstIterator end () const
    {
      return ConstIterator(*this,n);
    }

    //! begin iterator
    ConstIterator rbegin () const
    {
      return ConstIterator(*this,n-1);
    }

    //! end iterator
    ConstIterator rend () const
    {
      return ConstIterator(*this,-1);
    }

    //===== assignment from scalar

    FieldMatrix& operator= (const K& k)
    {
      a[0] = k;
      return *this;
    }

    //===== vector space arithmetic

    //! vector space addition
    FieldMatrix& operator+= (const K& y)
    {
      a[0] += y;
      return *this;
    }

    //! vector space subtraction
    FieldMatrix& operator-= (const K& y)
    {
      a[0] -= y;
      return *this;
    }

    //! vector space multiplication with scalar
    FieldMatrix& operator*= (const K& k)
    {
      a[0] *= k;
      return *this;
    }

    //! vector space division by scalar
    FieldMatrix& operator/= (const K& k)
    {
      a[0] /= k;
      return *this;
    }

    //===== linear maps

    //! y += A x
    void umv (const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p += a[0] * x.p;
    }

    //! y += A^T x
    void umtv (const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p += a[0] * x.p;
    }

    //! y += A^H x
    void umhv (const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p += fm_ck(a[0]) * x.p;
    }

    //! y -= A x
    void mmv (const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p -= a[0] * x.p;
    }

    //! y -= A^T x
    void mmtv (const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p -= a[0] * x.p;
    }

    //! y -= A^H x
    void mmhv (const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p -= fm_ck(a[0]) * x.p;
    }

    //! y += alpha A x
    void usmv (const K& alpha, const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p += alpha * a[0] * x.p;
    }

    //! y += alpha A^T x
    void usmtv (const K& alpha, const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p += alpha * a[0] * x.p;
    }

    //! y += alpha A^H x
    void usmhv (const K& alpha, const FieldVector<K,1>& x, FieldVector<K,1>& y) const
    {
      y.p += alpha * fm_ck(a[0]) * x.p;
    }

    //===== norms

    //! frobenius norm: sqrt(sum over squared values of entries)
    double frobenius_norm () const
    {
      return sqrt(fvmeta_abs2(a[0]));
    }

    //! square of frobenius norm, need for block recursion
    double frobenius_norm2 () const
    {
      return fvmeta_abs2(a[0]);
    }

    //! infinity norm (row sum norm, how to generalize for blocks?)
    double infinity_norm () const
    {
      return fvmeta_abs(a[0]);
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      return fvmeta_abs_real(a[0]);
    }

    //===== solve

    //! Solve system A x = b
    void solve (FieldVector<K,1>& x, const FieldVector<K,1>& b) const
    {
      x.p = b.p/a[0];
    }

    //! compute inverse
    void invert ()
    {
      a[0] = 1/a[0];
    }

    //! left multiplication
    FieldMatrix& leftmultiply (const FieldMatrix& M)
    {
      a[0] *= M.a[0];
      return *this;
    }

    //! left multiplication
    FieldMatrix& rightmultiply (const FieldMatrix& M)
    {
      a[0] *= M.a[0];
      return *this;
    }


    //===== sizes

    //! number of blocks in row direction
    int N () const
    {
      return 1;
    }

    //! number of blocks in column direction
    int M () const
    {
      return 1;
    }

    //! row dimension of block r
    int rowdim (int r) const
    {
      return 1;
    }

    //! col dimension of block c
    int coldim (int c) const
    {
      return 1;
    }

    //! dimension of the destination vector space
    int rowdim () const
    {
      return 1;
    }

    //! dimension of the source vector space
    int coldim () const
    {
      return 1;
    }

    //===== query

    //! return true when (i,j) is in pattern
    bool exists (int i, int j) const
    {
      return i==0 && j==0;
    }

    //===== conversion operator

    operator K () const {return a[0];}

  private:
    // the data, just a single row with a single scalar
    row_type a;
  };

  namespace FMatrixHelp {


    //! invert scalar without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,1,1> &matrix, FieldMatrix<K,1,1> &inverse)
    {
      inverse[0][0] = 1.0/matrix[0][0];
      return matrix[0][0];
    }


    //! invert 2x2 Matrix without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,2,2> &matrix, FieldMatrix<K,2,2> &inverse)
    {
      // code generated by maple
      K det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      K det_1 = 1.0/det;
      inverse[0][0] =   matrix[1][1] * det_1;
      inverse[0][1] = - matrix[0][1] * det_1;
      inverse[1][0] = - matrix[1][0] * det_1;
      inverse[1][1] =   matrix[0][0] * det_1;
      return det;
    }

    //! invert 3x3 Matrix without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
    {
      // code generated by maple
      K t4  = matrix[0][0] * matrix[1][1];
      K t6  = matrix[0][0] * matrix[1][2];
      K t8  = matrix[0][1] * matrix[1][0];
      K t10 = matrix[0][2] * matrix[1][0];
      K t12 = matrix[0][1] * matrix[2][0];
      K t14 = matrix[0][2] * matrix[2][0];

      K det = (t4*matrix[2][2]-t6*matrix[2][1]-t8*matrix[2][2]+
               t10*matrix[2][1]+t12*matrix[1][2]-t14*matrix[1][1]);
      K t17 = 1.0/det;

      inverse[0][0] =  (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1])*t17;
      inverse[0][1] = -(matrix[0][1] * matrix[2][2] - matrix[0][2] * matrix[2][1])*t17;
      inverse[0][2] =  (matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1])*t17;
      inverse[1][0] = -(matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0])*t17;
      inverse[1][1] =  (matrix[0][0] * matrix[2][2] - t14) * t17;
      inverse[1][2] = -(t6-t10) * t17;
      inverse[2][0] =  (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]) * t17;
      inverse[2][1] = -(matrix[0][0] * matrix[2][1] - t12) * t17;
      inverse[2][2] =  (t4-t8) * t17;

      return det;
    }

    //! calculates ret = matrix * x
    template <typename K, int dim>
    static void multAssign(const FieldMatrix<K,dim,dim> &matrix, const FieldVector<K,dim> & x, FieldVector<K,dim> & ret)
    {
      for(int i=0; i<dim; i++)
      {
        ret[i] = 0.0;
        for(int j=0; j<dim; j++)
        {
          ret[i] += matrix[i][j]*x[j];
        }
      }
    }

    //! calculates ret = matrix * x
    template <typename K, int dim>
    static FieldVector<K,dim> mult(const FieldMatrix<K,dim,dim> &matrix, const FieldVector<K,dim> & x)
    {
      FieldVector<K,dim> ret;
      multAssign(matrix,x,ret);
      return ret;
    }

  } // end namespace FMatrixHelp


  /** @} end documentation */

} // end namespace

#endif
