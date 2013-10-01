// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: fmatrix.hh 6128 2010-09-08 13:50:00Z christi $
#ifndef DUNE_DENSEMATRIX_HH
#define DUNE_DENSEMATRIX_HH

#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/precision.hh>
#include <dune/common/static_assert.hh>
#include <dune/common/classname.hh>
#include <dune/common/math.hh>


namespace Dune
{

  template<typename M> class DenseMatrix;

  template<typename M>
  struct FieldTraits< DenseMatrix<M> >
  {
    typedef const typename FieldTraits< typename DenseMatVecTraits<M>::value_type >::field_type field_type;
    typedef const typename FieldTraits< typename DenseMatVecTraits<M>::value_type >::real_type real_type;
  };

  /*
     work around a problem of FieldMatrix/FieldVector,
     there is no unique way to obtain the size of a class
   */
  template<class K, int N, int M> class FieldMatrix;
  template<class K, int N> class FieldVector;
  namespace {
    template<class V>
    struct VectorSize
    {
      static typename V::size_type size(const V & v) { return v.size(); }
    };

    template<class K, int N>
    struct VectorSize< const FieldVector<K,N> >
    {
      typedef FieldVector<K,N> V;
      static typename V::size_type size(const V & v) { return N; }
    };
  }

  /**
      @addtogroup DenseMatVec
      @{
   */

  /*! \file

     \brief  Implements a matrix constructed from a given type
     representing a field and a compile-time given number of rows and columns.
   */

  /**
     \brief you have to specialize this function for any type T that should be assignable to a DenseMatrix
     \tparam M Type of the matrix implementation class implementing the dense matrix
   */
  template<typename M, typename T>
  void istl_assign_to_fmatrix(DenseMatrix<M>& f, const T& t)
  {
    DUNE_THROW(NotImplemented, "You need to specialise the method istl_assign_to_fmatrix(DenseMatrix<M>& f, const T& t) "
               << "(with M being " << className<M>() << ") "
               << "for T == " << className<T>() << "!");
  }

  namespace
  {
    template<bool b>
    struct DenseMatrixAssigner
    {
      template<typename M, typename T>
      static void assign(DenseMatrix<M>& fm, const T& t)
      {
        istl_assign_to_fmatrix(fm, t);
      }

    };


    template<>
    struct DenseMatrixAssigner<true>
    {
      template<typename M, typename T>
      static void assign(DenseMatrix<M>& fm, const T& t)
      {
        fm = static_cast<const typename DenseMatVecTraits<M>::value_type>(t);
      }
    };
  }

  /** @brief Error thrown if operations of a FieldMatrix fail. */
  class FMatrixError : public Exception {};

  /**
      @brief A dense n x m matrix.

      Matrices represent linear maps from a vector space V to a vector space W.
      This class represents such a linear map by storing a two-dimensional
      %array of numbers of a given field type K. The number of rows and
      columns is given at compile time.

      \tparam MAT type of the matrix implementation
   */
  template<typename MAT>
  class DenseMatrix
  {
    typedef DenseMatVecTraits<MAT> Traits;

    // Curiously recurring template pattern
    MAT & asImp() { return static_cast<MAT&>(*this); }
    const MAT & asImp() const { return static_cast<const MAT&>(*this); }

  public:
    //===== type definitions and constants

    //! type of derived matrix class
    typedef typename Traits::derived_type derived_type;

    //! export the type representing the field
    typedef typename Traits::value_type value_type;

    //! export the type representing the field
    typedef typename Traits::value_type field_type;

    //! export the type representing the components
    typedef typename Traits::value_type block_type;

    //! The type used for the index access and size operation
    typedef typename Traits::size_type size_type;

    //! The type used to represent a row (must fulfill the Dune::DenseVector interface)
    typedef typename Traits::row_type row_type;

    //! The type used to represent a reference to a row (usually row_type &)
    typedef typename Traits::row_reference row_reference;

    //! The type used to represent a reference to a constant row (usually const row_type &)
    typedef typename Traits::const_row_reference const_row_reference;

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain. This is 1.
      blocklevel = 1
    };

    //===== access to components

    //! random access
    row_reference operator[] ( size_type i )
    {
      return asImp().mat_access(i);
    }

    const_row_reference operator[] ( size_type i ) const
    {
      return asImp().mat_access(i);
    }

    //! size method (number of rows)
    size_type size() const
    {
      return rows();
    }

    //===== iterator interface to rows of the matrix
    //! Iterator class for sequential access
    typedef DenseIterator<DenseMatrix,row_type> Iterator;
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
      return Iterator(*this,rows());
    }

    //! @returns an iterator that is positioned before
    //! the end iterator of the vector, i.e. at the last entry.
    Iterator beforeEnd ()
    {
      return Iterator(*this,rows()-1);
    }

    //! @returns an iterator that is positioned before
    //! the first entry of the vector.
    Iterator beforeBegin ()
    {
      return Iterator(*this,-1);
    }

    //! Iterator class for sequential access
    typedef DenseIterator<const DenseMatrix,const row_type> ConstIterator;
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
      return ConstIterator(*this,rows());
    }

    //! @returns an iterator that is positioned before
    //! the end iterator of the vector. i.e. at the last element
    ConstIterator beforeEnd () const
    {
      return ConstIterator(*this,rows()-1);
    }

    //! @returns an iterator that is positioned before
    //! the first entry of the vector.
    ConstIterator beforeBegin () const
    {
      return ConstIterator(*this,-1);
    }

    //===== assignment from scalar
    DenseMatrix& operator= (const field_type& f)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] = f;
      return *this;
    }

    template<typename T>
    DenseMatrix& operator= (const T& t)
    {
      DenseMatrixAssigner<Conversion<T,field_type>::exists>::assign(*this, t);
      return *this;
    }
    //===== vector space arithmetic

    //! vector space addition
    template <class Other>
    DenseMatrix& operator+= (const DenseMatrix<Other>& y)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] += y[i];
      return *this;
    }

    //! vector space subtraction
    template <class Other>
    DenseMatrix& operator-= (const DenseMatrix<Other>& y)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] -= y[i];
      return *this;
    }

    //! vector space multiplication with scalar
    DenseMatrix& operator*= (const field_type& k)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    DenseMatrix& operator/= (const field_type& k)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] /= k;
      return *this;
    }

    //! vector space axpy operation (*this += k y)
    template <class Other>
    DenseMatrix &axpy (const field_type &k, const DenseMatrix<Other> &y )
    {
      for( size_type i = 0; i < rows(); ++i )
        (*this)[ i ].axpy( k, y[ i ] );
      return *this;
    }

    //! Binary matrix comparison
    template <class Other>
    bool operator== (const DenseMatrix<Other>& y) const
    {
      for (size_type i=0; i<rows(); i++)
        if ((*this)[i]!=y[i])
          return false;
      return true;
    }
    //! Binary matrix incomparison
    template <class Other>
    bool operator!= (const DenseMatrix<Other>& y) const
    {
      return !operator==(y);
    }


    //===== linear maps

    //! y = A x
    template<class X, class Y>
    void mv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(FMatrixError,"Index out of range");
      if (y.N()!=N()) DUNE_THROW(FMatrixError,"Index out of range");
#endif
      for (size_type i=0; i<rows(); ++i)
      {
        y[i] = 0;
        for (size_type j=0; j<cols(); j++)
          y[i] += (*this)[i][j] * x[j];
      }
    }

    //! y = A^T x
    template< class X, class Y >
    void mtv ( const X &x, Y &y ) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      //assert( &x != &y );
      //This assert did not work for me. Compile error:
      //  comparison between distinct pointer types ‘const
      //  Dune::FieldVector<double, 3>*’ and ‘Dune::FieldVector<double, 2>*’ lacks a cast
      if( x.N() != N() )
        DUNE_THROW( FMatrixError, "Index out of range." );
      if( y.N() != M() )
        DUNE_THROW( FMatrixError, "Index out of range." );
#endif
      for( size_type i = 0; i < cols(); ++i )
      {
        y[ i ] = 0;
        for( size_type j = 0; j < rows(); ++j )
          y[ i ] += (*this)[ j ][ i ] * x[ j ];
      }
    }

    //! y += A x
    template<class X, class Y>
    void umv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M())
        DUNE_THROW(FMatrixError,"y += A x -- index out of range (sizes: x: " << x.N() << ", y: " << y.N() << ", A: " << this->N() << " x " << this->M() << ")" << std::endl);
      if (y.N()!=N())
        DUNE_THROW(FMatrixError,"y += A x -- index out of range (sizes: x: " << x.N() << ", y: " << y.N() << ", A: " << this->N() << " x " << this->M() << ")" << std::endl);
#endif
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[i] += (*this)[i][j] * x[j];
    }

    //! y += A^T x
    template<class X, class Y>
    void umtv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += (*this)[i][j]*x[i];
    }

    //! y += A^H x
    template<class X, class Y>
    void umhv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += conjugateComplex((*this)[i][j])*x[i];
    }

    //! y -= A x
    template<class X, class Y>
    void mmv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
#endif
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[i] -= (*this)[i][j] * x[j];
    }

    //! y -= A^T x
    template<class X, class Y>
    void mmtv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] -= (*this)[i][j]*x[i];
    }

    //! y -= A^H x
    template<class X, class Y>
    void mmhv (const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] -= conjugateComplex((*this)[i][j])*x[i];
    }

    //! y += alpha A x
    template<class X, class Y>
    void usmv (const field_type& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
#endif
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[i] += alpha * (*this)[i][j] * x[j];
    }

    //! y += alpha A^T x
    template<class X, class Y>
    void usmtv (const field_type& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += alpha*(*this)[i][j]*x[i];
    }

    //! y += alpha A^H x
    template<class X, class Y>
    void usmhv (const field_type& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(FMatrixError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(FMatrixError,"index out of range");
#endif

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += alpha*conjugateComplex((*this)[i][j])*x[i];
    }

    //===== norms

    //! frobenius norm: sqrt(sum over squared values of entries)
    typename FieldTraits<value_type>::real_type frobenius_norm () const
    {
      typename FieldTraits<value_type>::real_type sum=(0.0);
      for (size_type i=0; i<rows(); ++i) sum += (*this)[i].two_norm2();
      return fvmeta::sqrt(sum);
    }

    //! square of frobenius norm, need for block recursion
    typename FieldTraits<value_type>::real_type frobenius_norm2 () const
    {
      typename FieldTraits<value_type>::real_type sum=(0.0);
      for (size_type i=0; i<rows(); ++i) sum += (*this)[i].two_norm2();
      return sum;
    }

    //! infinity norm (row sum norm, how to generalize for blocks?)
    typename FieldTraits<value_type>::real_type infinity_norm () const
    {
      if (size() == 0)
        return 0.0;

      ConstIterator it = begin();
      typename remove_const< typename FieldTraits<value_type>::real_type >::type max = it->one_norm();
      for (it = it + 1; it != end(); ++it)
        max = std::max(max, it->one_norm());

      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    typename FieldTraits<value_type>::real_type infinity_norm_real () const
    {
      if (size() == 0)
        return 0.0;

      ConstIterator it = begin();
      typename remove_const< typename FieldTraits<value_type>::real_type >::type max = it->one_norm_real();
      for (it = it + 1; it != end(); ++it)
        max = std::max(max, it->one_norm_real());

      return max;
    }

    //===== solve

    /** \brief Solve system A x = b
     *
     * \exception FMatrixError if the matrix is singular
     */
    template <class V>
    void solve (V& x, const V& b) const;

    /** \brief Compute inverse
     *
     * \exception FMatrixError if the matrix is singular
     */
    void invert();

    //! calculates the determinant of this matrix
    field_type determinant () const;

    //! Multiplies M from the left to this matrix
    template<typename M2>
    MAT& leftmultiply (const DenseMatrix<M2>& M)
    {
      assert(M.rows() == M.cols() && M.rows() == rows());
      MAT C(asImp());

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++) {
          (*this)[i][j] = 0;
          for (size_type k=0; k<rows(); k++)
            (*this)[i][j] += M[i][k]*C[k][j];
        }

      return asImp();
    }

    //! Multiplies M from the right to this matrix
    template<typename M2>
    MAT& rightmultiply (const DenseMatrix<M2>& M)
    {
      assert(M.rows() == M.cols() && M.cols() == cols());
      MAT C(asImp());

      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++) {
          (*this)[i][j] = 0;
          for (size_type k=0; k<cols(); k++)
            (*this)[i][j] += C[i][k]*M[k][j];
        }
      return asImp();
    }

#if 0
    //! Multiplies M from the left to this matrix, this matrix is not modified
    template<int l>
    DenseMatrix<K,l,cols> leftmultiplyany (const FieldMatrix<K,l,rows>& M) const
    {
      FieldMatrix<K,l,cols> C;

      for (size_type i=0; i<l; i++) {
        for (size_type j=0; j<cols(); j++) {
          C[i][j] = 0;
          for (size_type k=0; k<rows(); k++)
            C[i][j] += M[i][k]*(*this)[k][j];
        }
      }
      return C;
    }

    //! Multiplies M from the right to this matrix, this matrix is not modified
    template<int l>
    FieldMatrix<K,rows,l> rightmultiplyany (const FieldMatrix<K,cols,l>& M) const
    {
      FieldMatrix<K,rows,l> C;

      for (size_type i=0; i<rows(); i++) {
        for (size_type j=0; j<l; j++) {
          C[i][j] = 0;
          for (size_type k=0; k<cols(); k++)
            C[i][j] += (*this)[i][k]*M[k][j];
        }
      }
      return C;
    }
#endif

    //===== sizes

    //! number of rows
    size_type N () const
    {
      return rows();
    }

    //! number of columns
    size_type M () const
    {
      return cols();
    }

    //! number of rows
    size_type rows() const
    {
      return asImp().mat_rows();
    }

    //! number of columns
    size_type cols() const
    {
      return asImp().mat_cols();
    }

    //===== query

    //! return true when (i,j) is in pattern
    bool exists (size_type i, size_type j) const
    {
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (i<0 || i>=rows()) DUNE_THROW(FMatrixError,"row index out of range");
      if (j<0 || j>=cols()) DUNE_THROW(FMatrixError,"column index out of range");
#endif
      return true;
    }

  private:

#ifndef DOXYGEN
    struct ElimPivot
    {
      ElimPivot(std::vector<size_type> & pivot);

      void swap(int i, int j);

      template<typename T>
      void operator()(const T&, int k, int i)
      {}

      std::vector<size_type> & pivot_;
    };

    template<typename V>
    struct Elim
    {
      Elim(V& rhs);

      void swap(int i, int j);

      void operator()(const typename V::field_type& factor, int k, int i);

      V* rhs_;
    };

    struct ElimDet
    {
      ElimDet(field_type& sign) : sign_(sign)
      { sign_ = 1; }

      void swap(int i, int j)
      { sign_ *= -1; }

      void operator()(const field_type&, int k, int i)
      {}

      field_type& sign_;
    };
#endif // DOXYGEN

    template<class Func>
    void luDecomposition(DenseMatrix<MAT>& A, Func func) const;
  };

#ifndef DOXYGEN
  template<typename MAT>
  DenseMatrix<MAT>::ElimPivot::ElimPivot(std::vector<size_type> & pivot)
    : pivot_(pivot)
  {
    typedef typename std::vector<size_type>::size_type size_type;
    for(size_type i=0; i < pivot_.size(); ++i) pivot_[i]=i;
  }

  template<typename MAT>
  void DenseMatrix<MAT>::ElimPivot::swap(int i, int j)
  {
    pivot_[i]=j;
  }

  template<typename MAT>
  template<typename V>
  DenseMatrix<MAT>::Elim<V>::Elim(V& rhs)
    : rhs_(&rhs)
  {}

  template<typename MAT>
  template<typename V>
  void DenseMatrix<MAT>::Elim<V>::swap(int i, int j)
  {
    std::swap((*rhs_)[i], (*rhs_)[j]);
  }

  template<typename MAT>
  template<typename V>
  void DenseMatrix<MAT>::
  Elim<V>::operator()(const typename V::field_type& factor, int k, int i)
  {
    (*rhs_)[k] -= factor*(*rhs_)[i];
  }
  template<typename MAT>
  template<typename Func>
  inline void DenseMatrix<MAT>::luDecomposition(DenseMatrix<MAT>& A, Func func) const
  {
    typedef typename FieldTraits<value_type>::real_type
    real_type;
    real_type norm = A.infinity_norm_real(); // for relative thresholds
    real_type pivthres = std::max( FMatrixPrecision< real_type >::absolute_limit(), norm * FMatrixPrecision< real_type >::pivoting_limit() );
    real_type singthres = std::max( FMatrixPrecision< real_type >::absolute_limit(), norm * FMatrixPrecision< real_type >::singular_limit() );

    // LU decomposition of A in A
    for (size_type i=0; i<rows(); i++)  // loop over all rows
    {
      typename FieldTraits<value_type>::real_type pivmax=fvmeta::absreal(A[i][i]);

      // pivoting ?
      if (pivmax<pivthres)
      {
        // compute maximum of column
        size_type imax=i;
        typename FieldTraits<value_type>::real_type abs(0.0);
        for (size_type k=i+1; k<rows(); k++)
          if ((abs=fvmeta::absreal(A[k][i]))>pivmax)
          {
            pivmax = abs; imax = k;
          }
        // swap rows
        if (imax!=i) {
          for (size_type j=0; j<rows(); j++)
            std::swap(A[i][j],A[imax][j]);
          func.swap(i, imax); // swap the pivot or rhs
        }
      }

      // singular ?
      if (pivmax<singthres)
        DUNE_THROW(FMatrixError,"matrix is singular");

      // eliminate
      for (size_type k=i+1; k<rows(); k++)
      {
        field_type factor = A[k][i]/A[i][i];
        A[k][i] = factor;
        for (size_type j=i+1; j<rows(); j++)
          A[k][j] -= factor*A[i][j];
        func(factor, k, i);
      }
    }
  }

  template<typename MAT>
  template <class V>
  inline void DenseMatrix<MAT>::solve(V& x, const V& b) const
  {
    // never mind those ifs, because they get optimized away
    if (rows()!=cols())
      DUNE_THROW(FMatrixError, "Can't solve for a " << rows() << "x" << cols() << " matrix!");

    if (rows()==1) {

#ifdef DUNE_FMatrix_WITH_CHECKING
      if (fvmeta::absreal((*this)[0][0])<FMatrixPrecision<>::absolute_limit())
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      x[0] = b[0]/(*this)[0][0];

    }
    else if (rows()==2) {

      field_type detinv = (*this)[0][0]*(*this)[1][1]-(*this)[0][1]*(*this)[1][0];
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (fvmeta::absreal(detinv)<FMatrixPrecision<>::absolute_limit())
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      detinv = 1.0/detinv;

      x[0] = detinv*((*this)[1][1]*b[0]-(*this)[0][1]*b[1]);
      x[1] = detinv*((*this)[0][0]*b[1]-(*this)[1][0]*b[0]);

    }
    else if (rows()==3) {

      field_type d = determinant();
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (fvmeta::absreal(d)<FMatrixPrecision<>::absolute_limit())
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif

      x[0] = (b[0]*(*this)[1][1]*(*this)[2][2] - b[0]*(*this)[2][1]*(*this)[1][2]
              - b[1] *(*this)[0][1]*(*this)[2][2] + b[1]*(*this)[2][1]*(*this)[0][2]
              + b[2] *(*this)[0][1]*(*this)[1][2] - b[2]*(*this)[1][1]*(*this)[0][2]) / d;

      x[1] = ((*this)[0][0]*b[1]*(*this)[2][2] - (*this)[0][0]*b[2]*(*this)[1][2]
              - (*this)[1][0] *b[0]*(*this)[2][2] + (*this)[1][0]*b[2]*(*this)[0][2]
              + (*this)[2][0] *b[0]*(*this)[1][2] - (*this)[2][0]*b[1]*(*this)[0][2]) / d;

      x[2] = ((*this)[0][0]*(*this)[1][1]*b[2] - (*this)[0][0]*(*this)[2][1]*b[1]
              - (*this)[1][0] *(*this)[0][1]*b[2] + (*this)[1][0]*(*this)[2][1]*b[0]
              + (*this)[2][0] *(*this)[0][1]*b[1] - (*this)[2][0]*(*this)[1][1]*b[0]) / d;

    }
    else {

      V& rhs = x; // use x to store rhs
      rhs = b; // copy data
      Elim<V> elim(rhs);
      MAT A(asImp());

      luDecomposition(A, elim);

      // backsolve
      for(int i=rows()-1; i>=0; i--) {
        for (size_type j=i+1; j<rows(); j++)
          rhs[i] -= A[i][j]*x[j];
        x[i] = rhs[i]/A[i][i];
      }
    }
  }

  template<typename MAT>
  inline void DenseMatrix<MAT>::invert()
  {
    // never mind those ifs, because they get optimized away
    if (rows()!=cols())
      DUNE_THROW(FMatrixError, "Can't invert a " << rows() << "x" << cols() << " matrix!");

    if (rows()==1) {

#ifdef DUNE_FMatrix_WITH_CHECKING
      if (fvmeta::absreal((*this)[0][0])<FMatrixPrecision<>::absolute_limit())
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      (*this)[0][0] = field_type( 1 ) / (*this)[0][0];

    }
    else if (rows()==2) {

      field_type detinv = (*this)[0][0]*(*this)[1][1]-(*this)[0][1]*(*this)[1][0];
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (fvmeta::absreal(detinv)<FMatrixPrecision<>::absolute_limit())
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      detinv = field_type( 1 ) / detinv;

      field_type temp=(*this)[0][0];
      (*this)[0][0] =  (*this)[1][1]*detinv;
      (*this)[0][1] = -(*this)[0][1]*detinv;
      (*this)[1][0] = -(*this)[1][0]*detinv;
      (*this)[1][1] =  temp*detinv;

    }
    else {

      MAT A(asImp());
      std::vector<size_type> pivot(rows());
      luDecomposition(A, ElimPivot(pivot));
      DenseMatrix<MAT>& L=A;
      DenseMatrix<MAT>& U=A;

      // initialize inverse
      *this=field_type();

      for(size_type i=0; i<rows(); ++i)
        (*this)[i][i]=1;

      // L Y = I; multiple right hand sides
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<i; j++)
          for (size_type k=0; k<rows(); k++)
            (*this)[i][k] -= L[i][j]*(*this)[j][k];

      // U A^{-1} = Y
      for (size_type i=rows(); i>0;) {
        --i;
        for (size_type k=0; k<rows(); k++) {
          for (size_type j=i+1; j<rows(); j++)
            (*this)[i][k] -= U[i][j]*(*this)[j][k];
          (*this)[i][k] /= U[i][i];
        }
      }

      for(size_type i=rows(); i>0; ) {
        --i;
        if(i!=pivot[i])
          for(size_type j=0; j<rows(); ++j)
            std::swap((*this)[j][pivot[i]], (*this)[j][i]);
      }
    }
  }

  // implementation of the determinant
  template<typename MAT>
  inline typename DenseMatrix<MAT>::field_type
  DenseMatrix<MAT>::determinant() const
  {
    // never mind those ifs, because they get optimized away
    if (rows()!=cols())
      DUNE_THROW(FMatrixError, "There is no determinant for a " << rows() << "x" << cols() << " matrix!");

    if (rows()==1)
      return (*this)[0][0];

    if (rows()==2)
      return (*this)[0][0]*(*this)[1][1] - (*this)[0][1]*(*this)[1][0];

    if (rows()==3) {
      // code generated by maple
      field_type t4  = (*this)[0][0] * (*this)[1][1];
      field_type t6  = (*this)[0][0] * (*this)[1][2];
      field_type t8  = (*this)[0][1] * (*this)[1][0];
      field_type t10 = (*this)[0][2] * (*this)[1][0];
      field_type t12 = (*this)[0][1] * (*this)[2][0];
      field_type t14 = (*this)[0][2] * (*this)[2][0];

      return (t4*(*this)[2][2]-t6*(*this)[2][1]-t8*(*this)[2][2]+
              t10*(*this)[2][1]+t12*(*this)[1][2]-t14*(*this)[1][1]);

    }

    MAT A(asImp());
    field_type det;
    try
    {
      luDecomposition(A, ElimDet(det));
    }
    catch (FMatrixError&)
    {
      return 0;
    }
    for (size_type i = 0; i < rows(); ++i)
      det *= A[i][i];
    return det;
  }

#endif // DOXYGEN

  namespace DenseMatrixHelp {
#if 0
    //! invert scalar without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,1,1> &matrix, FieldMatrix<K,1,1> &inverse)
    {
      inverse[0][0] = 1.0/matrix[0][0];
      return matrix[0][0];
    }

    //! invert scalar without changing the original matrix
    template <typename K>
    static inline K invertMatrix_retTransposed (const FieldMatrix<K,1,1> &matrix, FieldMatrix<K,1,1> &inverse)
    {
      return invertMatrix(matrix,inverse);
    }


    //! invert 2x2 Matrix without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,2,2> &matrix, FieldMatrix<K,2,2> &inverse)
    {
      // code generated by maple
      field_type det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      field_type det_1 = 1.0/det;
      inverse[0][0] =   matrix[1][1] * det_1;
      inverse[0][1] = - matrix[0][1] * det_1;
      inverse[1][0] = - matrix[1][0] * det_1;
      inverse[1][1] =   matrix[0][0] * det_1;
      return det;
    }

    //! invert 2x2 Matrix without changing the original matrix
    //! return transposed matrix
    template <typename K>
    static inline K invertMatrix_retTransposed (const FieldMatrix<K,2,2> &matrix, FieldMatrix<K,2,2> &inverse)
    {
      // code generated by maple
      field_type det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      field_type det_1 = 1.0/det;
      inverse[0][0] =   matrix[1][1] * det_1;
      inverse[1][0] = - matrix[0][1] * det_1;
      inverse[0][1] = - matrix[1][0] * det_1;
      inverse[1][1] =   matrix[0][0] * det_1;
      return det;
    }

    //! invert 3x3 Matrix without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
    {
      // code generated by maple
      field_type t4  = matrix[0][0] * matrix[1][1];
      field_type t6  = matrix[0][0] * matrix[1][2];
      field_type t8  = matrix[0][1] * matrix[1][0];
      field_type t10 = matrix[0][2] * matrix[1][0];
      field_type t12 = matrix[0][1] * matrix[2][0];
      field_type t14 = matrix[0][2] * matrix[2][0];

      field_type det = (t4*matrix[2][2]-t6*matrix[2][1]-t8*matrix[2][2]+
                        t10*matrix[2][1]+t12*matrix[1][2]-t14*matrix[1][1]);
      field_type t17 = 1.0/det;

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

    //! invert 3x3 Matrix without changing the original matrix
    template <typename K>
    static inline K invertMatrix_retTransposed (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
    {
      // code generated by maple
      field_type t4  = matrix[0][0] * matrix[1][1];
      field_type t6  = matrix[0][0] * matrix[1][2];
      field_type t8  = matrix[0][1] * matrix[1][0];
      field_type t10 = matrix[0][2] * matrix[1][0];
      field_type t12 = matrix[0][1] * matrix[2][0];
      field_type t14 = matrix[0][2] * matrix[2][0];

      field_type det = (t4*matrix[2][2]-t6*matrix[2][1]-t8*matrix[2][2]+
                        t10*matrix[2][1]+t12*matrix[1][2]-t14*matrix[1][1]);
      field_type t17 = 1.0/det;

      inverse[0][0] =  (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1])*t17;
      inverse[1][0] = -(matrix[0][1] * matrix[2][2] - matrix[0][2] * matrix[2][1])*t17;
      inverse[2][0] =  (matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1])*t17;
      inverse[0][1] = -(matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0])*t17;
      inverse[1][1] =  (matrix[0][0] * matrix[2][2] - t14) * t17;
      inverse[2][1] = -(t6-t10) * t17;
      inverse[0][2] =  (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]) * t17;
      inverse[1][2] = -(matrix[0][0] * matrix[2][1] - t12) * t17;
      inverse[2][2] =  (t4-t8) * t17;

      return det;
    }

    //! calculates ret = A * B
    template< class K, int m, int n, int p >
    static inline void multMatrix ( const FieldMatrix< K, m, n > &A,
                                    const FieldMatrix< K, n, p > &B,
                                    FieldMatrix< K, m, p > &ret )
    {
      typedef typename FieldMatrix< K, m, p > :: size_type size_type;

      for( size_type i = 0; i < m; ++i )
      {
        for( size_type j = 0; j < p; ++j )
        {
          ret[ i ][ j ] = K( 0 );
          for( size_type k = 0; k < n; ++k )
            ret[ i ][ j ] += A[ i ][ k ] * B[ k ][ j ];
        }
      }
    }

    //! calculates ret= A_t*A
    template <typename K, int rows, int cols>
    static inline void multTransposedMatrix(const FieldMatrix<K,rows,cols> &matrix, FieldMatrix<K,cols,cols>& ret)
    {
      typedef typename FieldMatrix<K,rows,cols>::size_type size_type;

      for(size_type i=0; i<cols(); i++)
        for(size_type j=0; j<cols(); j++)
        {
          ret[i][j]=0.0;
          for(size_type k=0; k<rows(); k++)
            ret[i][j]+=matrix[k][i]*matrix[k][j];
        }
    }
#endif

    //! calculates ret = matrix * x
    template <typename MAT, typename V1, typename V2>
    static inline void multAssign(const DenseMatrix<MAT> &matrix, const DenseVector<V1> & x, DenseVector<V2> & ret)
    {
      assert(x.size() == matrix.cols());
      assert(ret.size() == matrix.rows());
      typedef typename DenseMatrix<MAT>::size_type size_type;

      for(size_type i=0; i<matrix.rows(); ++i)
      {
        ret[i] = 0.0;
        for(size_type j=0; j<matrix.cols(); ++j)
        {
          ret[i] += matrix[i][j]*x[j];
        }
      }
    }

#if 0
    //! calculates ret = matrix^T * x
    template <typename K, int rows, int cols>
    static inline void multAssignTransposed( const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,rows> & x, FieldVector<K,cols> & ret)
    {
      typedef typename FieldMatrix<K,rows,cols>::size_type size_type;

      for(size_type i=0; i<cols(); ++i)
      {
        ret[i] = 0.0;
        for(size_type j=0; j<rows(); ++j)
          ret[i] += matrix[j][i]*x[j];
      }
    }

    //! calculates ret = matrix * x
    template <typename K, int rows, int cols>
    static inline FieldVector<K,rows> mult(const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,cols> & x)
    {
      FieldVector<K,rows> ret;
      multAssign(matrix,x,ret);
      return ret;
    }

    //! calculates ret = matrix^T * x
    template <typename K, int rows, int cols>
    static inline FieldVector<K,cols> multTransposed(const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,rows> & x)
    {
      FieldVector<K,cols> ret;
      multAssignTransposed( matrix, x, ret );
      return ret;
    }
#endif

  } // end namespace DenseMatrixHelp

  /** \brief Sends the matrix to an output stream */
  template<typename MAT>
  std::ostream& operator<< (std::ostream& s, const DenseMatrix<MAT>& a)
  {
    for (typename DenseMatrix<MAT>::size_type i=0; i<a.rows(); i++)
      s << a[i] << std::endl;
    return s;
  }

  /** @} end documentation */

} // end namespace Dune

#endif
