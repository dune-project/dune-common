// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FMATRIX_HH
#define DUNE_FMATRIX_HH

#include <math.h>
#include <complex>
#include <iostream>

#include "istlexception.hh"
#include "allocator.hh"
#include "dune/common/fvector.hh"
#include "precision.hh"

/*! \file

   \brief  This file implements a matrix constructed from a given type
   representing a field and compile-time given number of rows and columns.
 */

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  template<class K, int n, int m> class FieldMatrix;

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
    double pivthres = std::max(ISTLPrecision<>::absolute_limit(),norm*ISTLPrecision<>::pivoting_limit());
    double singthres = std::max(ISTLPrecision<>::absolute_limit(),norm*ISTLPrecision<>::singular_limit());
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
        DUNE_THROW(ISTLError,"matrix is singular");

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
#ifdef DUNE_ISTL_WITH_CHECKING
    if (fvmeta_absreal(A[0][0])<ISTLPrecision<>::absolute_limit())
      DUNE_THROW(ISTLError,"matrix is singular");
#endif
    x[0] = b[0]/A[0][0];
  }

  //! special case for 2x2 matrix, x and b may be identical
  template<class K, class V>
  inline void fm_solve (const FieldMatrix<K,2,2>& A,  V& x, const V& b)
  {
#ifdef DUNE_ISTL_WITH_CHECKING
    K detinv = A[0][0]*A[1][1]-A[0][1]*A[1][0];
    if (fvmeta_absreal(detinv)<ISTLPrecision<>::absolute_limit())
      DUNE_THROW(ISTLError,"matrix is singular");
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
    double pivthres = std::max(ISTLPrecision<>::absolute_limit(),norm*ISTLPrecision<>::pivoting_limit());
    double singthres = std::max(ISTLPrecision<>::absolute_limit(),norm*ISTLPrecision<>::singular_limit());

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
        DUNE_THROW(ISTLError,"matrix is singular");

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
#ifdef DUNE_ISTL_WITH_CHECKING
    if (fvmeta_absreal(A[0][0])<ISTLPrecision<>::absolute_limit())
      DUNE_THROW(ISTLError,"matrix is singular");
#endif
    A[0][0] = 1/A[0][0];
  }

  //! compute inverse n=2
  template<class K>
  void fm_invert (FieldMatrix<K,2,2>& A)
  {
    K detinv = A[0][0]*A[1][1]-A[0][1]*A[1][0];
#ifdef DUNE_ISTL_WITH_CHECKING
    if (fvmeta_absreal(detinv)<ISTLPrecision<>::absolute_limit())
      DUNE_THROW(ISTLError,"matrix is singular");
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


  /** Matrices represent linear maps from a vector space V to a vector space W.
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
    enum {blocklevel = 1};

    //! Each row is implemented by a field vector
    typedef FieldVector<K,m> row_type;

    //! export size
    enum {rows = n, cols = m};

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
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const row_type& operator[] (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }


    //===== iterator interface to rows of the matrix

    // forward declaration
    class ConstIterator;

    //! Iterator access to rows
    class Iterator
    {
    public:
      //! constructor
      Iterator (row_type* _p, int _i)
      {
        p = _p;
        i = _i;
      }

      //! empty constructor, use with care!
      Iterator ()
      {       }

      //! prefix increment
      Iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      Iterator& operator--()
      {
        --i;
        return *this;
      }

      //! equality
      bool operator== (const Iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      row_type& operator* ()
      {
        return p[i];
      }

      //! arrow
      row_type* operator-> ()
      {
        return p+i;
      }

      //! return index
      int index ()
      {
        return i;
      }

      friend class ConstIterator;

    private:
      row_type* p;
      int i;
    };

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(p,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(p,n);
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(p,n-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(p,-1);
    }

    //! rename the iterators for easier access
    typedef Iterator RowIterator;
    typedef typename row_type::Iterator ColIterator;


    //! Iterator access to rows
    class ConstIterator
    {
    public:
      //! constructor
      ConstIterator (const row_type* _p, int _i) : p(_p), i(_i)
      {       }

      //! empty constructor, use with care!
      ConstIterator ()
      {
        p = 0;
        i = 0;
      }

      //! prefix increment
      ConstIterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      ConstIterator& operator--()
      {
        --i;
        return *this;
      }

      //! equality
      bool operator== (const ConstIterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const ConstIterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! equality
      bool operator== (const Iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      const row_type& operator* () const
      {
        return p[i];
      }

      //! arrow
      const row_type* operator-> () const
      {
        return p+i;
      }

      //! return index
      int index () const
      {
        return i;
      }

      friend class Iterator;

    private:
      const row_type* p;
      int i;
    };

    //! begin iterator
    ConstIterator begin () const
    {
      return ConstIterator(p,0);
    }

    //! end iterator
    ConstIterator end () const
    {
      return ConstIterator(p,n);
    }

    //! begin iterator
    ConstIterator rbegin () const
    {
      return ConstIterator(p,n-1);
    }

    //! end iterator
    ConstIterator rend () const
    {
      return ConstIterator(p,-1);
    }

    //! rename the iterators for easier access
    typedef ConstIterator ConstRowIterator;
    typedef typename row_type::ConstIterator ConstColIterator;


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
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      fmmeta_umv<n-1>::template umv<FieldMatrix,X,Y,m-1>(*this,x,y);
    }

    //! y += A^T x
    template<class X, class Y>
    void umtv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += p[i][j]*x[i];
    }

    //! y += A^H x
    template<class X, class Y>
    void umhv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += fm_ck(p[i][j])*x[i];
    }

    //! y -= A x
    template<class X, class Y>
    void mmv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      fmmeta_mmv<n-1>::template mmv<FieldMatrix,X,Y,m-1>(*this,x,y);
      //fm_mmv(*this,x,y);
    }

    //! y -= A^T x
    template<class X, class Y>
    void mmtv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] -= p[i][j]*x[i];
    }

    //! y -= A^H x
    template<class X, class Y>
    void mmhv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] -= fm_ck(p[i][j])*x[i];
    }

    //! y += alpha A x
    template<class X, class Y>
    void usmv (const K& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      fmmeta_usmv<n-1>::template usmv<FieldMatrix,K,X,Y,m-1>(*this,alpha,x,y);
    }

    //! y += alpha A^T x
    template<class X, class Y>
    void usmtv (const K& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif

      for (int i=0; i<n; i++)
        for (int j=0; j<m; j++)
          y[j] += alpha*p[i][j]*x[i];
    }

    //! y += alpha A^H x
    template<class X, class Y>
    void usmhv (const K& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
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
     * \exception ISTLError if the matrix is singular
     */
    template<class V>
    void solve (V& x, const V& b) const
    {
      fm_solve(*this,x,b);
    }

    /** \brief Compute inverse
     *
     * \exception ISTLError if the matrix is singular
     */
    void invert ()
    {
      fm_invert(*this);
    }

    //! calculates the determinant of this matrix
    K determinant () const;

    //! left multiplication
    FieldMatrix& leftmultiply (const FieldMatrix<K,n,n>& M)
    {
      fm_leftmultiply(M,*this);
      return *this;
    }

    //! left multiplication
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
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
      if (j<0 || i>=m) DUNE_THROW(ISTLError,"index out of range");
#endif
      return true;
    }

    //===== conversion operator

    /** \brief Returns the entry [0][0] */
    operator K () const DUNE_DEPRECATED {return p[0][0];}

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
        DUNE_THROW(ISTLError, "There is no determinant for a " << row << "x" << col << " matrix!");

      DUNE_THROW(ISTLError, "No implementation of determinantMatrix "
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


  /**! Matrices of size 1x1 are treated in a special way
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
    enum {blocklevel = 1};

    //! export size
    enum {rows = 1, cols = 1};

    //===== random access interface to rows of the matrix

    //! random access to data
    K& operator() ()
    {
      return a;
    }

    //! same for read only access
    const K& operator() () const
    {
      return a;
    }

    //===== assignment from scalar

    K11Matrix& operator= (const K& k)
    {
      a = k;
      return *this;
    }

    //===== vector space arithmetic

    //! vector space addition
    K11Matrix& operator+= (const K11Matrix& y)
    {
      a += y.a;
      return *this;
    }

    //! vector space subtraction
    K11Matrix& operator-= (const K11Matrix& y)
    {
      a -= y.a;
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

    // return true when (i,j) is in pattern
    bool exists (int i, int j) const
    {
      return true;
    }

    //===== conversion operator

    operator K () const {return a;}

  private:
    // the data, very simply a built in array with rowwise ordering
    K a;
  };

  /** @} end documentation */

} // end namespace

#endif
