// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FMATRIX_HH__
#define __DUNE_FMATRIX_HH__

#include <math.h>
#include <complex>

#include "istlexception.hh"
#include "allocator.hh"
#include "fvector.hh"

/*! \file __FILE__

   This file implements a matrix constructed from a given type
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
  template<class K, int n, class X, class Y>
  inline void fm_solve (const FieldMatrix<K,n,n>& A,  X& x, Y& b)
  {
    // LU decomposition with maximum column pivot
    DUNE_THROW(ISTLError,"not implemented yet");
  }

  //! special case for 1x1 matrix, x and b may be identical
  template<class K, class X, class Y>
  inline void fm_solve (const FieldMatrix<K,1,1>& A,  X& x, Y& b)
  {
    x[0] = b[0]/A[0][0];
  }

  //! special case for 2x2 matrix, x and b may be identical
  template<class K, class X, class Y>
  inline void fm_solve (const FieldMatrix<K,2,2>& A,  X& x, Y& b)
  {
    K detinv = 1.0/(A[0][0]*A[1][1]-A[0][1]*A[1][0]);

    K temp = b[0];
    x[0] = detinv*(A[1][1]*b[0]-A[0][1]*b[1]);
    x[1] = detinv*(A[0][0]*b[1]-A[1][0]*temp);
  }


  /**! Matrices represent linear maps from a vector space V to a vector space W.
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

    //! rename the iterators for easier access
    typedef Iterator RowIterator;
    typedef typename row_type::Iterator ColIterator;


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
    void umv (X& x, Y& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      fmmeta_umv<n-1>::template umv<FieldMatrix,X,Y,m-1>(*this,x,y);
    }

    //! y += A^T x
    template<class X, class Y>
    void umtv (X& x, Y& y)
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
    void umhv (X& x, Y& y)
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
    void mmv (X& x, Y& y)
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
    void mmtv (X& x, Y& y)
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
    void mmhv (X& x, Y& y)
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
    void usmv (const K& alpha, X& x, Y& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      fmmeta_usmv<n-1>::template usmv<FieldMatrix,K,X,Y,m-1>(*this,alpha,x,y);
    }

    //! y += alpha A^T x
    template<class X, class Y>
    void usmtv (const K& alpha, X& x, Y& y)
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
    void usmhv (const K& alpha, X& x, Y& y)
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

    //! Solve system A x = b
    template<class X, class Y>
    void solve (X& x, Y& b) const
    {
      fm_solve(*this,x,b);
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

    // return true when (i,j) is in pattern
    bool exists (int i, int j)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
      if (j<0 || i>=m) DUNE_THROW(ISTLError,"index out of range");
#endif
      return true;
    }

    //===== conversion operator

    operator K () {return p[0][0];}


  private:
    // the data, very simply a built in array with rowwise ordering
    row_type p[n];
  };



  /** @} end documentation */

} // end namespace

#endif
