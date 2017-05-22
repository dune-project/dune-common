// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DENSEMATRIX_HH
#define DUNE_DENSEMATRIX_HH

#include <cmath>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

#include <dune/common/boundschecking.hh>
#include <dune/common/classname.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/math.hh>
#include <dune/common/precision.hh>
#include <dune/common/simd.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/unused.hh>

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
      static typename V::size_type size(const V & v)
      {
        DUNE_UNUSED_PARAMETER(v);
        return N;
      }
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
     \brief you have to specialize this structure for any type that should be assignable to a DenseMatrix
     \tparam DenseMatrix Some type implementing the dense matrix interface
     \tparam RHS Right hand side type
   */
  template< class DenseMatrix, class RHS >
  struct DenseMatrixAssigner;

#ifndef DOXYGEN
  namespace Impl
  {

    template< class DenseMatrix, class RHS, class = void >
    class DenseMatrixAssigner
    {};

    template< class DenseMatrix, class RHS >
    class DenseMatrixAssigner< DenseMatrix, RHS, std::enable_if_t< Dune::IsNumber< RHS >::value > >
    {
    public:
      static void apply ( DenseMatrix &denseMatrix, const RHS &rhs )
      {
        typedef typename DenseMatrix::field_type field_type;
        std::fill( denseMatrix.begin(), denseMatrix.end(), static_cast< field_type >( rhs ) );
      }
    };

    template< class DenseMatrix, class RHS >
    class DenseMatrixAssigner< DenseMatrix, RHS, std::enable_if_t< !std::is_same< typename RHS::const_iterator, void >::value
        && std::is_convertible< typename RHS::const_iterator::value_type, typename DenseMatrix::iterator::value_type >::value > >
    {
    public:
      static void apply ( DenseMatrix &denseMatrix, const RHS &rhs )
      {
        DUNE_ASSERT_BOUNDS(rhs.N() == denseMatrix.N());
        DUNE_ASSERT_BOUNDS(rhs.M() == denseMatrix.M());
        typename DenseMatrix::iterator tIt = std::begin(denseMatrix);
        typename RHS::const_iterator sIt = std::begin(rhs);
        for(; sIt != std::end(rhs); ++tIt, ++sIt)
          std::copy(std::begin(*sIt), std::end(*sIt), std::begin(*tIt));
      }
    };

  } // namespace Impl



  template< class DenseMatrix, class RHS >
  struct DenseMatrixAssigner
    : public Impl::DenseMatrixAssigner< DenseMatrix, RHS >
  {};


  namespace Impl
  {

    template< class DenseMatrix, class RHS >
    std::true_type hasDenseMatrixAssigner ( DenseMatrix &, const RHS &, decltype( Dune::DenseMatrixAssigner< DenseMatrix, RHS >::apply( std::declval< DenseMatrix & >(), std::declval< const RHS & >() ) ) * = nullptr );

    std::false_type hasDenseMatrixAssigner ( ... );

  } // namespace Impl

  template< class DenseMatrix, class RHS >
  struct HasDenseMatrixAssigner
    : public decltype( Impl::hasDenseMatrixAssigner( std::declval< DenseMatrix & >(), std::declval< const RHS & >() ) )
  {};

#endif // #ifndef DOXYGEN



  /** @brief Error thrown if operations of a FieldMatrix fail. */
  class FMatrixError : public MathError {};

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

  private:
    //! \brief if value_type is a simd vector, then this is a simd vector of
    //!        the same length that can be used for indices.
    /**
     * Just pray that the fundamental type is actually large enough...
     */
    using simd_index_type = SimdIndex<value_type>;

  public:
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
    typedef DenseIterator<DenseMatrix,row_type,row_reference> Iterator;
    //! typedef for stl compliant access
    typedef Iterator iterator;
    //! rename the iterators for easier access
    typedef Iterator RowIterator;
    //! rename the iterators for easier access
    typedef typename std::remove_reference<row_reference>::type::Iterator ColIterator;

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
    typedef DenseIterator<const DenseMatrix,const row_type,const_row_reference> ConstIterator;
    //! typedef for stl compliant access
    typedef ConstIterator const_iterator;
    //! rename the iterators for easier access
    typedef ConstIterator ConstRowIterator;
    //! rename the iterators for easier access
    typedef typename std::remove_reference<const_row_reference>::type::ConstIterator ConstColIterator;

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

    //===== assignment

    template< class RHS, class = std::enable_if_t< HasDenseMatrixAssigner< MAT, RHS >::value > >
    derived_type &operator= ( const RHS &rhs )
    {
      DenseMatrixAssigner< MAT, RHS >::apply( asImp(), rhs );
      return asImp();
    }

    //===== vector space arithmetic

    //! vector space addition
    template <class Other>
    derived_type &operator+= (const DenseMatrix<Other>& y)
    {
      DUNE_ASSERT_BOUNDS(rows() == y.rows());
      for (size_type i=0; i<rows(); i++)
        (*this)[i] += y[i];
      return asImp();
    }

    //! vector space subtraction
    template <class Other>
    derived_type &operator-= (const DenseMatrix<Other>& y)
    {
      DUNE_ASSERT_BOUNDS(rows() == y.rows());
      for (size_type i=0; i<rows(); i++)
        (*this)[i] -= y[i];
      return asImp();
    }

    //! vector space multiplication with scalar
    derived_type &operator*= (const field_type& k)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] *= k;
      return asImp();
    }

    //! vector space division by scalar
    derived_type &operator/= (const field_type& k)
    {
      for (size_type i=0; i<rows(); i++)
        (*this)[i] /= k;
      return asImp();
    }

    //! vector space axpy operation (*this += k y)
    template <class Other>
    derived_type &axpy (const field_type &k, const DenseMatrix<Other> &y )
    {
      DUNE_ASSERT_BOUNDS(rows() == y.rows());
      for( size_type i = 0; i < rows(); ++i )
        (*this)[ i ].axpy( k, y[ i ] );
      return asImp();
    }

    //! Binary matrix comparison
    template <class Other>
    bool operator== (const DenseMatrix<Other>& y) const
    {
      DUNE_ASSERT_BOUNDS(rows() == y.rows());
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
      DUNE_ASSERT_BOUNDS((void*)(&x) != (void*)(&y));
      DUNE_ASSERT_BOUNDS(x.N() == M());
      DUNE_ASSERT_BOUNDS(y.N() == N());

      using field_type = typename FieldTraits<Y>::field_type;
      for (size_type i=0; i<rows(); ++i)
      {
        y[i] = field_type(0);
        for (size_type j=0; j<cols(); j++)
          y[i] += (*this)[i][j] * x[j];
      }
    }

    //! y = A^T x
    template< class X, class Y >
    void mtv ( const X &x, Y &y ) const
    {
      DUNE_ASSERT_BOUNDS((void*)(&x) != (void*)(&y));
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());

      using field_type = typename FieldTraits<Y>::field_type;
      for(size_type i = 0; i < cols(); ++i)
      {
        y[i] = field_type(0);
        for(size_type j = 0; j < rows(); ++j)
          y[i] += (*this)[j][i] * x[j];
      }
    }

    //! y += A x
    template<class X, class Y>
    void umv (const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == M());
      DUNE_ASSERT_BOUNDS(y.N() == N());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[i] += (*this)[i][j] * x[j];
    }

    //! y += A^T x
    template<class X, class Y>
    void umtv (const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += (*this)[i][j]*x[i];
    }

    //! y += A^H x
    template<class X, class Y>
    void umhv (const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += conjugateComplex((*this)[i][j])*x[i];
    }

    //! y -= A x
    template<class X, class Y>
    void mmv (const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == M());
      DUNE_ASSERT_BOUNDS(y.N() == N());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[i] -= (*this)[i][j] * x[j];
    }

    //! y -= A^T x
    template<class X, class Y>
    void mmtv (const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] -= (*this)[i][j]*x[i];
    }

    //! y -= A^H x
    template<class X, class Y>
    void mmhv (const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] -= conjugateComplex((*this)[i][j])*x[i];
    }

    //! y += alpha A x
    template<class X, class Y>
    void usmv (const typename FieldTraits<Y>::field_type & alpha,
      const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == M());
      DUNE_ASSERT_BOUNDS(y.N() == N());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[i] += alpha * (*this)[i][j] * x[j];
    }

    //! y += alpha A^T x
    template<class X, class Y>
    void usmtv (const typename FieldTraits<Y>::field_type & alpha,
      const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());
      for (size_type i=0; i<rows(); i++)
        for (size_type j=0; j<cols(); j++)
          y[j] += alpha*(*this)[i][j]*x[i];
    }

    //! y += alpha A^H x
    template<class X, class Y>
    void usmhv (const typename FieldTraits<Y>::field_type & alpha,
      const X& x, Y& y) const
    {
      DUNE_ASSERT_BOUNDS(x.N() == N());
      DUNE_ASSERT_BOUNDS(y.N() == M());
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
    template <typename vt = value_type,
              typename std::enable_if<!has_nan<vt>::value, int>::type = 0>
    typename FieldTraits<vt>::real_type infinity_norm() const {
      using real_type = typename FieldTraits<vt>::real_type;
      using std::max;

      real_type norm = 0;
      for (auto const &x : *this) {
        real_type const a = x.one_norm();
        norm = max(a, norm);
      }
      return norm;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    template <typename vt = value_type,
              typename std::enable_if<!has_nan<vt>::value, int>::type = 0>
    typename FieldTraits<vt>::real_type infinity_norm_real() const {
      using real_type = typename FieldTraits<vt>::real_type;
      using std::max;

      real_type norm = 0;
      for (auto const &x : *this) {
        real_type const a = x.one_norm_real();
        norm = max(a, norm);
      }
      return norm;
    }

    //! infinity norm (row sum norm, how to generalize for blocks?)
    template <typename vt = value_type,
              typename std::enable_if<has_nan<vt>::value, int>::type = 0>
    typename FieldTraits<vt>::real_type infinity_norm() const {
      using real_type = typename FieldTraits<vt>::real_type;
      using std::max;

      real_type norm = 0;
      real_type isNaN = 1;
      for (auto const &x : *this) {
        real_type const a = x.one_norm();
        norm = max(a, norm);
        isNaN += a;
      }
      isNaN /= isNaN;
      return norm * isNaN;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    template <typename vt = value_type,
              typename std::enable_if<has_nan<vt>::value, int>::type = 0>
    typename FieldTraits<vt>::real_type infinity_norm_real() const {
      using real_type = typename FieldTraits<vt>::real_type;
      using std::max;

      real_type norm = 0;
      real_type isNaN = 1;
      for (auto const &x : *this) {
        real_type const a = x.one_norm_real();
        norm = max(a, norm);
        isNaN += a;
      }
      isNaN /= isNaN;
      return norm * isNaN;
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
      DUNE_ASSERT_BOUNDS(M.rows() == M.cols());
      DUNE_ASSERT_BOUNDS(M.rows() == rows());
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
      DUNE_ASSERT_BOUNDS(M.rows() == M.cols());
      DUNE_ASSERT_BOUNDS(M.cols() == cols());
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
      DUNE_UNUSED_PARAMETER(i);
      DUNE_UNUSED_PARAMETER(j);
      DUNE_ASSERT_BOUNDS(i >= 0 && i < rows());
      DUNE_ASSERT_BOUNDS(j >= 0 && j < cols());
      return true;
    }

  private:

#ifndef DOXYGEN
    struct ElimPivot
    {
      ElimPivot(std::vector<simd_index_type> & pivot);

      void swap(std::size_t i, simd_index_type j);

      template<typename T>
      void operator()(const T&, int, int)
      {}

      std::vector<simd_index_type> & pivot_;
    };

    template<typename V>
    struct Elim
    {
      Elim(V& rhs);

      void swap(std::size_t i, simd_index_type j);

      void operator()(const typename V::field_type& factor, int k, int i);

      V* rhs_;
    };

    struct ElimDet
    {
      ElimDet(field_type& sign) : sign_(sign)
      { sign_ = 1; }

      void swap(std::size_t i, simd_index_type j)
      {
        sign_ *= cond(simd_index_type(i) == j, field_type(1), field_type(-1));
      }

      void operator()(const field_type&, int, int)
      {}

      field_type& sign_;
    };
#endif // DOXYGEN

    //! do an LU-Decomposition on matrix A
    /**
     * \param A                The matrix to decompose, and to store the
     *                         result in.
     * \param func             Functor used for swapping lanes and to conduct
     *                         the elimination.  Depending on the functor, \c
     *                         luDecomposition() can be used for solving, for
     *                         inverting, or to compute the determinant.
     * \param nonsingularLanes SimdMask of lanes that are nonsingular.
     * \param throwEarly       Whether to throw an \c FMatrixError immediately
     *                         as soon as one lane is discovered to be
     *                         singular.  If \c false, do not throw, instead
     *                         continue until finished or all lanes are
     *                         singular, and exit via return in both cases.
     *
     * There are two modes of operation:
     * <ul>
     * <li>Terminate as soon as one lane is discovered to be singular.  Early
     *     termination is done by throwing an \c FMatrixError.  On entry, \c
     *     all_true(nonsingularLanes) and \c throwEarly==true should hold.
     *     After early termination, the contents of \c A should be considered
     *     bogus, and \c nonsingularLanes has the lane(s) that triggered the
     *     early termination unset.  There may be more singular lanes than the
     *     one reported in \c nonsingularLanes, which just havent been
     *     discovered yet; so the value of \c nonsingularLanes is mostly
     *     useful for diagnostics.</li>
     * <li>Terminate only when all lanes are discovered to be singular.  Use
     *     this when you want to apply special postprocessing in singular
     *     lines (e.g. setting the determinant of singular lanes to 0 in \c
     *     determinant()).  On entry, \c nonsingularLanes may have any value
     *     and \c throwEarly==false should hold.  The function will not throw
     *     an exception if some lanes are discovered to be singular, instead
     *     it will continue running until all lanes are singular or until
     *     finished, and terminate only via normal return.  On exit, \c
     *     nonsingularLanes contains the map of lanes that are valid in \c
     *     A.</li>
     * </ul>
     */
    template<class Func, class Mask>
    void luDecomposition(DenseMatrix<MAT>& A, Func func,
                         Mask &nonsingularLanes, bool throwEarly) const;
  };

#ifndef DOXYGEN
  template<typename MAT>
  DenseMatrix<MAT>::ElimPivot::ElimPivot(std::vector<simd_index_type> & pivot)
    : pivot_(pivot)
  {
    typedef typename std::vector<size_type>::size_type size_type;
    for(size_type i=0; i < pivot_.size(); ++i) pivot_[i]=i;
  }

  template<typename MAT>
  void DenseMatrix<MAT>::ElimPivot::swap(std::size_t i, simd_index_type j)
  {
    assign(pivot_[i], j, SimdScalar<simd_index_type>(i) != j);
  }

  template<typename MAT>
  template<typename V>
  DenseMatrix<MAT>::Elim<V>::Elim(V& rhs)
    : rhs_(&rhs)
  {}

  template<typename MAT>
  template<typename V>
  void DenseMatrix<MAT>::Elim<V>::swap(std::size_t i, simd_index_type j)
  {
    using std::swap;

    // see the comment in luDecomposition()
    for(std::size_t l = 0; l < lanes(j); ++l)
      swap(lane(l, (*rhs_)[        i ]),
           lane(l, (*rhs_)[lane(l, j)]));
  }

  template<typename MAT>
  template<typename V>
  void DenseMatrix<MAT>::
  Elim<V>::operator()(const typename V::field_type& factor, int k, int i)
  {
    (*rhs_)[k] -= factor*(*rhs_)[i];
  }

  template<typename MAT>
  template<typename Func, class Mask>
  inline void DenseMatrix<MAT>::
  luDecomposition(DenseMatrix<MAT>& A, Func func, Mask &nonsingularLanes,
                  bool throwEarly) const
  {
    using std::swap;

    typedef typename FieldTraits<value_type>::real_type real_type;

    real_type norm = A.infinity_norm_real(); // for relative thresholds
    real_type pivthres =
      std::max( FMatrixPrecision< real_type >::absolute_limit(),
                norm * FMatrixPrecision< real_type >::pivoting_limit() );
    real_type singthres =
      std::max( FMatrixPrecision< real_type >::absolute_limit(),
                norm * FMatrixPrecision< real_type >::singular_limit() );

    // LU decomposition of A in A
    for (size_type i=0; i<rows(); i++)  // loop over all rows
    {
      real_type pivmax = fvmeta::absreal(A[i][i]);
      auto do_pivot = pivmax<pivthres;

      // pivoting ?
      if (any_true(do_pivot && nonsingularLanes))
      {
        // compute maximum of column
        simd_index_type imax=i;
        for (size_type k=i+1; k<rows(); k++)
        {
          auto abs = fvmeta::absreal(A[k][i]);
          auto mask = abs > pivmax && do_pivot;
          pivmax = cond(mask, abs, pivmax);
          imax   = cond(mask, simd_index_type(k), imax);
        }
        // swap rows
        if (any_true(imax != i && nonsingularLanes)) {
          for (size_type j=0; j<rows(); j++)
          {
            // This is a swap operation where the second operand is scattered,
            // and on top of that is also extracted from deep within a
            // moderately complicated data structure (a DenseMatrix), where we
            // can't assume much on the memory layout.  On intel processors,
            // the only instruction that might help us here is vgather, but it
            // is unclear whether that is even faster than a software
            // implementation, and we would also need vscatter which does not
            // exist.  So break vectorization here and do it manually.
            for(std::size_t l = 0; l < lanes(A[i][j]); ++l)
              swap(lane(l, A[i][j]), lane(l, A[lane(l, imax)][j]));
          }
          func.swap(i, imax); // swap the pivot or rhs
        }
      }

      // singular ?
      nonsingularLanes = nonsingularLanes && !(pivmax<singthres);
      if (throwEarly) {
        if(!all_true(nonsingularLanes))
          DUNE_THROW(FMatrixError, "matrix is singular");
      }
      else { // !throwEarly
        if(!any_true(nonsingularLanes))
          return;
      }

      // eliminate
      for (size_type k=i+1; k<rows(); k++)
      {
        // in the simd case, A[i][i] may be close to zero in some lanes.  Pray
        // that the result is no worse than a quiet NaN.
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
      if (any_true(fvmeta::absreal((*this)[0][0])
                   < FMatrixPrecision<>::absolute_limit()))
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      x[0] = b[0]/(*this)[0][0];

    }
    else if (rows()==2) {

      field_type detinv = (*this)[0][0]*(*this)[1][1]-(*this)[0][1]*(*this)[1][0];
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (any_true(fvmeta::absreal(detinv)
                   < FMatrixPrecision<>::absolute_limit()))
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      detinv = 1.0/detinv;

      x[0] = detinv*((*this)[1][1]*b[0]-(*this)[0][1]*b[1]);
      x[1] = detinv*((*this)[0][0]*b[1]-(*this)[1][0]*b[0]);

    }
    else if (rows()==3) {

      field_type d = determinant();
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (any_true(fvmeta::absreal(d) < FMatrixPrecision<>::absolute_limit()))
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
      SimdMask<typename FieldTraits<value_type>::real_type>
        nonsingularLanes(true);

      luDecomposition(A, elim, nonsingularLanes, true);

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
      if (any_true(fvmeta::absreal((*this)[0][0])
                   < FMatrixPrecision<>::absolute_limit()))
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      (*this)[0][0] = field_type( 1 ) / (*this)[0][0];

    }
    else if (rows()==2) {

      field_type detinv = (*this)[0][0]*(*this)[1][1]-(*this)[0][1]*(*this)[1][0];
#ifdef DUNE_FMatrix_WITH_CHECKING
      if (any_true(fvmeta::absreal(detinv)
                   < FMatrixPrecision<>::absolute_limit()))
        DUNE_THROW(FMatrixError,"matrix is singular");
#endif
      detinv = field_type( 1 ) / detinv;

      field_type temp=(*this)[0][0];
      (*this)[0][0] =  (*this)[1][1]*detinv;
      (*this)[0][1] = -(*this)[0][1]*detinv;
      (*this)[1][0] = -(*this)[1][0]*detinv;
      (*this)[1][1] =  temp*detinv;

    }
    else if (rows()==3)
    {
      using K = field_type;
      // code generated by maple
      K t4  = (*this)[0][0] * (*this)[1][1];
      K t6  = (*this)[0][0] * (*this)[1][2];
      K t8  = (*this)[0][1] * (*this)[1][0];
      K t10 = (*this)[0][2] * (*this)[1][0];
      K t12 = (*this)[0][1] * (*this)[2][0];
      K t14 = (*this)[0][2] * (*this)[2][0];

      K det = (t4*(*this)[2][2]-t6*(*this)[2][1]-t8*(*this)[2][2]+
               t10*(*this)[2][1]+t12*(*this)[1][2]-t14*(*this)[1][1]);
      K t17 = K(1.0)/det;

      K matrix01 = (*this)[0][1];
      K matrix00 = (*this)[0][0];
      K matrix10 = (*this)[1][0];
      K matrix11 = (*this)[1][1];

      (*this)[0][0] =  ((*this)[1][1] * (*this)[2][2] - (*this)[1][2] * (*this)[2][1])*t17;
      (*this)[0][1] = -((*this)[0][1] * (*this)[2][2] - (*this)[0][2] * (*this)[2][1])*t17;
      (*this)[0][2] =  (matrix01 * (*this)[1][2] - (*this)[0][2] * (*this)[1][1])*t17;
      (*this)[1][0] = -((*this)[1][0] * (*this)[2][2] - (*this)[1][2] * (*this)[2][0])*t17;
      (*this)[1][1] =  (matrix00 * (*this)[2][2] - t14) * t17;
      (*this)[1][2] = -(t6-t10) * t17;
      (*this)[2][0] =  (matrix10 * (*this)[2][1] - matrix11 * (*this)[2][0]) * t17;
      (*this)[2][1] = -(matrix00 * (*this)[2][1] - t12) * t17;
      (*this)[2][2] =  (t4-t8) * t17;
    }
    else {

      MAT A(asImp());
      std::vector<simd_index_type> pivot(rows());
      SimdMask<typename FieldTraits<value_type>::real_type>
        nonsingularLanes(true);
      luDecomposition(A, ElimPivot(pivot), nonsingularLanes, true);
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
        for(std::size_t l = 0; l < lanes((*this)[0][0]); ++l)
        {
          std::size_t pi = lane(l, pivot[i]);
          if(i!=pi)
            for(size_type j=0; j<rows(); ++j)
              std::swap(lane(l, (*this)[j][pi]),
                        lane(l, (*this)[j][ i]));
        }
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
    SimdMask<typename FieldTraits<value_type>::real_type>
      nonsingularLanes(true);

    luDecomposition(A, ElimDet(det), nonsingularLanes, false);
    assign(det, field_type(0), !nonsingularLanes);

    for (size_type i = 0; i < rows(); ++i)
      det *= A[i][i];
    return det;
  }

#endif // DOXYGEN

  namespace DenseMatrixHelp {

    //! calculates ret = matrix * x
    template <typename MAT, typename V1, typename V2>
    static inline void multAssign(const DenseMatrix<MAT> &matrix, const DenseVector<V1> & x, DenseVector<V2> & ret)
    {
      DUNE_ASSERT_BOUNDS(x.size() == matrix.cols());
      DUNE_ASSERT_BOUNDS(ret.size() == matrix.rows());
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
