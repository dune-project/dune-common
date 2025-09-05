// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_FMATRIX_HH
#define DUNE_FMATRIX_HH

#include <cmath>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <type_traits>

#include <dune/common/boundschecking.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/densematrix.hh>
#include <dune/common/precision.hh>
#include <dune/common/promotiontraits.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/matrixconcepts.hh>

namespace Dune
{

  namespace Impl
  {

    template<class M>
    class ColumnVectorView
    {
    public:

      using value_type = typename M::value_type;
      using size_type = typename M::size_type;

      constexpr ColumnVectorView(M& matrix, size_type col) :
        matrix_(matrix),
        col_(col)
      {}

      constexpr size_type N () const {
        return matrix_.N();
      }

      template<class M_ = M,
        std::enable_if_t<std::is_same_v<M_,M> and not std::is_const_v<M_>, int> = 0>
      constexpr value_type& operator[] (size_type row) {
        return matrix_[row][col_];
      }

      constexpr const value_type& operator[] (size_type row) const {
        return matrix_[row][col_];
      }

    protected:
      M& matrix_;
      const size_type col_;
    };

  }

  template<typename M>
  struct FieldTraits< Impl::ColumnVectorView<M> >
  {
    using field_type = typename FieldTraits<M>::field_type;
    using real_type = typename FieldTraits<M>::real_type;
  };

  /**
      @addtogroup DenseMatVec
      @{
   */

  /*! \file

     \brief  Implements a matrix constructed from a given type
     representing a field and compile-time given number of rows and columns.
   */

  template< class K, int ROWS, int COLS = ROWS > class FieldMatrix;


  template< class K, int ROWS, int COLS >
  struct DenseMatVecTraits< FieldMatrix<K,ROWS,COLS> >
  {
    typedef FieldMatrix<K,ROWS,COLS> derived_type;

    // each row is implemented by a field vector
    typedef FieldVector<K,COLS> row_type;

    typedef row_type &row_reference;
    typedef const row_type &const_row_reference;

    typedef std::array<row_type,ROWS> container_type;
    typedef K value_type;
    typedef typename container_type::size_type size_type;
  };

  template< class K, int ROWS, int COLS >
  struct FieldTraits< FieldMatrix<K,ROWS,COLS> >
  {
    typedef typename FieldTraits<K>::field_type field_type;
    typedef typename FieldTraits<K>::real_type real_type;
  };


  /**
   * @brief TMP to check the shape of a DenseMatrix statically, if possible.
   *
   * If the implementation type of M is a FieldMatrix, we statically check
   * whether its shape is ROWSxCOLS.
   *
   * @tparam M The implementation of the other DenseMatrix.
   * @tparam ROWS The number of rows we need assume.
   * @tparam COLS The number of columns we need assume.
   */
  template<class M, int ROWS, int COLS>
  struct IsFieldMatrixShapeCorrect
    : std::true_type {};

  template<class K, int ROWS, int COLS>
  struct IsFieldMatrixShapeCorrect<FieldMatrix<K,ROWS,COLS>,ROWS,COLS>
    : std::true_type {};

  template<class K, int ROWS, int COLS, int ROWS1, int COLS1>
  struct IsFieldMatrixShapeCorrect<FieldMatrix<K,ROWS1,COLS1>,ROWS,COLS>
    : std::false_type {};


  /**
      @brief A dense n x m matrix.

     Matrices represent linear maps from a vector space V to a vector space W.
       This class represents such a linear map by storing a two-dimensional
       %array of numbers of a given field type K. The number of rows and
       columns is given at compile time.
   */
  template<class K, int ROWS, int COLS>
  class FieldMatrix : public DenseMatrix< FieldMatrix<K,ROWS,COLS> >
  {
    template<class,int,int> friend class FieldMatrix;
    using Base = DenseMatrix< FieldMatrix<K,ROWS,COLS> >;

    //! The container storage
    std::array< FieldVector<K,COLS>, ROWS > _data;

  public:

    //! The number of rows.
    static constexpr std::integral_constant<int, ROWS> rows = {};

    //! The number of columns.
    static constexpr std::integral_constant<int, COLS> cols = {};

    //! The type used for the index access and size operation
    using size_type = typename Base::size_type;

    //! The type of the elements stored in the matrix
    using value_type = typename Base::value_type;

    //! The type used for references to the matrix entries
    using reference = value_type&;

    //! The type used for const references to the matrix entries
    using const_reference = const value_type&;

    //! The type the rows of the matrix are represented by
    using row_type = typename Base::row_type;

    //! The type used for references to the rows of the matrix
    using row_reference = typename Base::row_reference;

    //! The type used for const references to the rows of the matrix
    using const_row_reference = typename Base::const_row_reference;

  public:

    /// \name Constructors
    /// @{

    //! Default constructor, making value-initialized matrix with all components set to zero
    constexpr FieldMatrix ()
        noexcept(std::is_nothrow_default_constructible_v<K>)
      : _data{}
    {}

    //! Constructor with a given value initializing all entries to this value
    explicit(ROWS*COLS != 1)
    constexpr FieldMatrix (const value_type& value) noexcept
      : _data{filledArray<ROWS>(row_type(value))}
    {}

    //! Constructor with a given scalar initializing all entries to this value
    template<Concept::Number S>
      requires (std::constructible_from<K,S>)
    explicit(ROWS*COLS != 1)
    constexpr FieldMatrix (const S& scalar)
        noexcept(std::is_nothrow_constructible_v<K,S>)
      : _data{filledArray<ROWS>(row_type(scalar))}
    {}

    //! Constructor initializing the matrix from a nested list of values
    constexpr FieldMatrix(std::initializer_list<Dune::FieldVector<K, cols> > const &l)
      : _data{}
    {
      assert(l.size() == rows);
      for (size_type i = 0; i < rows; ++i)
        _data[i] = std::data(l)[i];
    }

    //! copy constructor from assignable type OtherMatrix
    template <class OtherMatrix>
      requires (not Concept::Number<OtherMatrix> && HasDenseMatrixAssigner<FieldMatrix, OtherMatrix>::value)
    constexpr FieldMatrix(const OtherMatrix& rhs)
        noexcept(std::is_nothrow_assignable_v<FieldMatrix&, const OtherMatrix&>)
      : _data{}
    {
      *this = rhs;
    }

    //! copy constructor
    constexpr FieldMatrix (const FieldMatrix&) = default;

    /// @}

    //! Assignment from another dense matrix
    template<class M>
      requires (IsFieldMatrixShapeCorrect<M,ROWS,COLS>::value &&
        std::is_assignable_v<K&, decltype(std::declval<const M&>()[0][0])>)
    constexpr FieldMatrix& operator= (const DenseMatrix<M>& x)
    {
      assert(x.rows() == rows);
      for (size_type i = 0; i < rows; ++i)
        _data[i] = x[i];
      return *this;
    }

    //! copy assignment from FieldMatrix over a different field
    template <class OtherK>
    constexpr FieldMatrix& operator= (const FieldMatrix<OtherK, ROWS, COLS>& x)
    {
      // The copy must be done element-by-element since a std::array does not have
      // a converting assignment operator.
      for (std::size_t i = 0; i < _data.size(); ++i)
        _data[i] = x._data[i];
      return *this;
    }

    //! Assignment operator from scalar
    template<Concept::Number S>
      requires std::constructible_from<K,S>
    constexpr FieldMatrix& operator= (const S& scalar)
        noexcept(std::is_nothrow_constructible_v<K,S>)
    {
      for (std::size_t i = 0; i < _data.size(); ++i)
        _data[i] = scalar;
      return *this;
    }

    //! copy assignment operator
    constexpr FieldMatrix& operator= (const FieldMatrix&) = default;

    //! no copy assignment from FieldMatrix of different size
    template <class OtherK, int OtherRows, int OtherCols>
    FieldMatrix& operator=(FieldMatrix<OtherK,OtherRows,OtherCols> const&) = delete;

    //! Return transposed of the matrix as FieldMatrix
    constexpr FieldMatrix<K, COLS, ROWS> transposed() const
    {
      Dune::FieldMatrix<K, COLS, ROWS> AT;
      for( int i = 0; i < ROWS; ++i )
        for( int j = 0; j < COLS; ++j )
          AT[j][i] = (*this)[i][j];
      return AT;
    }


    /// \name Vector space operations
    /// @{

    //! vector space addition
    template <class OtherK>
    friend constexpr auto operator+ (const FieldMatrix& matrixA,
                                     const FieldMatrix<OtherK,ROWS,COLS>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherK>::PromotedType,ROWS,COLS> result;
      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrixA[i][j] + matrixB[i][j];
      return result;
    }

    //! Binary addition, when using FieldVector<K,1,1> like K
    template <Concept::Number S>
    friend constexpr auto operator+ (const FieldMatrix& a, const S& b) noexcept
        requires(ROWS*COLS == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldMatrix<ResultValueType,1,1>{a[0][0] + b};
    }

    //! Binary addition, when using FieldMatrix<K,1,1> like K
    template <Concept::Number S>
    friend constexpr auto operator+ (const S& a, const FieldMatrix& b) noexcept
        requires(ROWS*COLS == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldMatrix<ResultValueType,1,1>{a + b[0][0]};
    }

    //! add scalar
    template <Concept::Number S>
    constexpr FieldMatrix& operator+= (const S& scalar)
      requires(ROWS*COLS == 1)
    {
      _data[0][0] += scalar;
      return *this;
    }

    using Base::operator+=;

    //! vector space subtraction
    template <class OtherK>
    friend constexpr auto operator- (const FieldMatrix& matrixA,
                                     const FieldMatrix<OtherK,ROWS,COLS>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherK>::PromotedType,ROWS,COLS> result;
      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrixA[i][j] - matrixB[i][j];
      return result;
    }

    //! Binary subtraction, when using FieldMatrix<K,1,1> like K
    template<Concept::Number S>
    friend constexpr auto operator- (const FieldMatrix& a, const S& b) noexcept
        requires(ROWS*COLS == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldMatrix<ResultValueType,1,1>{a[0][0] - b};
    }

    //! Binary subtraction, when using FieldMatrix<K,1,1> like K
    template<Concept::Number S>
    friend constexpr auto operator- (const S& a, const FieldMatrix& b) noexcept
        requires(ROWS*COLS == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldMatrix<ResultValueType,1,1>{a - b[0][0]};
    }

    //! subtract scalar
    template <Concept::Number S>
    constexpr FieldMatrix& operator-= (const S& scalar)
      requires(ROWS*COLS == 1)
    {
      _data[0][0] -= scalar;
      return *this;
    }

    using Base::operator-=;

    //! vector space multiplication with scalar
    template <Concept::Number S>
    friend constexpr auto operator* (const FieldMatrix& matrix, const S& scalar)
    {
      FieldMatrix<typename PromotionTraits<K,S>::PromotedType,ROWS,COLS> result;
      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrix[i][j] * scalar;
      return result;
    }

    //! vector space multiplication with scalar
    template <Concept::Number S>
    friend constexpr auto operator* (const S& scalar, const FieldMatrix& matrix)
    {
      FieldMatrix<typename PromotionTraits<K,S>::PromotedType,ROWS,COLS> result;
      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = scalar * matrix[i][j];
      return result;
    }

    //! multiplication with scalar
    template <Concept::Number S>
    constexpr FieldMatrix& operator*= (const S& scalar)
      requires(ROWS*COLS == 1)
    {
      _data[0][0] *= scalar;
      return *this;
    }

    using Base::operator*=;

    //! vector space division by scalar
    template <Concept::Number S>
    friend constexpr auto operator/ (const FieldMatrix& matrix, const S& scalar)
    {
      FieldMatrix<typename PromotionTraits<K,S>::PromotedType,ROWS,COLS> result;
      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrix[i][j] / scalar;
      return result;
    }

    //! Binary division, when using FieldMatrix<K,1,1> like K
    template<Concept::Number S>
    friend constexpr FieldMatrix operator/ (const S& a, const FieldMatrix& b) noexcept
        requires(ROWS*COLS == 1)
    {
      return FieldMatrix{a / b[0][0]};
    }

    //! division by scalar
    template <Concept::Number S>
    constexpr FieldMatrix& operator/= (const S& scalar)
      requires(ROWS*COLS == 1)
    {
      _data[0][0] /= scalar;
      return *this;
    }

    using Base::operator/=;

    /// @}


    /// \name Matrix-matrix multiplication
    /// @{

    //! Matrix-matrix multiplication of FieldMatrix types
    template <class OtherK, int otherCols>
    friend constexpr auto operator* (const FieldMatrix& matrixA,
                                     const FieldMatrix<OtherK, COLS, otherCols>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherK>::PromotedType,ROWS,otherCols> result;

      for (size_type i = 0; i < matrixA.mat_rows(); ++i)
        for (size_type j = 0; j < matrixB.mat_cols(); ++j)
        {
          result[i][j] = 0;
          for (size_type k = 0; k < matrixA.mat_cols(); ++k)
            result[i][j] += matrixA[i][k] * matrixB[k][j];
        }

      return result;
    }

    /** \brief Matrix-matrix multiplication of FieldMatrix with other matrix type
     *
     * This implements multiplication of a FieldMatrix with another matrix
     * of type OtherMatrix. The latter has to provide
     * OtherMatrix::field_type, OtherMatrix::cols, and OtherMatrix::mtv(x,y).
     */
    template <class OtherMatrix>
      requires (Impl::IsStaticSizeMatrix_v<OtherMatrix> and not Impl::IsFieldMatrix_v<OtherMatrix>)
    friend constexpr auto operator* (const FieldMatrix& matrixA,
                                     const OtherMatrix& matrixB)
    {
      using Field = typename PromotionTraits<K, typename OtherMatrix::field_type>::PromotedType;
      Dune::FieldMatrix<Field, rows ,OtherMatrix::cols> result;
      for (size_type j = 0; j < rows; ++j)
        matrixB.mtv(matrixA[j], result[j]);
      return result;
    }

    /** \brief Matrix-matrix multiplication
     *
     * This implements multiplication of another matrix
     * of type OtherMatrix with a FieldMatrix. The former has to provide
     * OtherMatrix::field_type, OtherMatrix::rows, and OtherMatrix::mv(x,y).
     */
    template <class OtherMatrix>
      requires (Impl::IsStaticSizeMatrix_v<OtherMatrix> and not Impl::IsFieldMatrix_v<OtherMatrix>)
    friend constexpr auto operator* (const OtherMatrix& matrixA,
                                     const FieldMatrix& matrixB)
    {
      using Field = typename PromotionTraits<K, typename OtherMatrix::field_type>::PromotedType;
      Dune::FieldMatrix<Field, OtherMatrix::rows, cols> result;
      for (size_type j = 0; j < cols; ++j)
      {
        auto B_j = Impl::ColumnVectorView(matrixB, j);
        auto result_j = Impl::ColumnVectorView(result, j);
        matrixA.mv(B_j, result_j);
      }
      return result;
    }

    //! Multiplies M from the left to this matrix, this matrix is not modified
    template<int l>
    constexpr FieldMatrix<K,l,cols> leftmultiplyany (const FieldMatrix<K,l,rows>& M) const
    {
      return M * (*this);
    }

    using Base::rightmultiply;

    //! Multiplies M from the right to this matrix
    template <int r, int c>
    constexpr FieldMatrix& rightmultiply (const FieldMatrix<K,r,c>& M)
    {
      static_assert(r == c, "Cannot rightmultiply with non-square matrix");
      static_assert(r == cols, "Size mismatch");
      FieldMatrix<K,rows,cols> C(*this);

      for (size_type i=0; i<rows; i++)
        for (size_type j=0; j<cols; j++) {
          (*this)[i][j] = 0;
          for (size_type k=0; k<cols; k++)
            (*this)[i][j] += C[i][k]*M[k][j];
        }
      return *this;
    }

    //! Multiplies M from the right to this matrix, this matrix is not modified
    template<int l>
    constexpr FieldMatrix<K,rows,l> rightmultiplyany (const FieldMatrix<K,cols,l>& M) const
    {
      return (*this) * M;
    }

    /// @}


    /// \name Capacity
    /// @{

    // internal: return the number of rows
    static constexpr size_type mat_rows() { return ROWS; }

    // internal: return the number of columns
    static constexpr size_type mat_cols() { return COLS; }

    /// @}


    /// \name Element access
    /// @{

    // internal: return a reference to the ith row
    constexpr row_reference mat_access (size_type i)
    {
      DUNE_ASSERT_BOUNDS(i < ROWS);
      return _data[i];
    }

    // internal: return a const reference to the ith row
    constexpr const_row_reference mat_access (size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i < ROWS);
      return _data[i];
    }

    //! Conversion operator
    constexpr operator const_reference () const noexcept
        requires(ROWS*COLS == 1)
    {
      return _data[0][0];
    }

    //! Conversion operator
    constexpr operator reference () noexcept
        requires(ROWS*COLS == 1)
    {
      return _data[0][0];
    }

    /// @}


    /// \name Comparison operators
    /// @{

    //! comparing FieldMatrix<1,1> with scalar for equality
    template<Concept::Number S>
    friend constexpr bool operator== (const FieldMatrix& a, const S& b) noexcept
        requires(ROWS*COLS == 1)
    {
      return a._data[0] == b;
    }

    //! comparing FieldMatrix<1,1> with scalar for equality
    template<Concept::Number S>
    friend constexpr bool operator== (const S& a, const FieldMatrix& b) noexcept
        requires(ROWS*COLS == 1)
    {
      return a == b._data[0];
    }

    //! three-way comparison of FieldMatrix
    template<class OtherK>
      requires (Std::three_way_comparable_with<K,OtherK>)
    friend constexpr auto operator<=> (const FieldMatrix& a, const FieldMatrix<OtherK,ROWS,COLS>& b) noexcept
    {
#if __cpp_lib_three_way_comparison
      return a._data <=> b._data;
#else
      return Std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end());
#endif
    }

    //! three-way comparison of FieldMatrix<1,1> with scalar
    template<Concept::Number S>
    friend constexpr auto operator<=> (const FieldMatrix& a, const S& b) noexcept
        requires(ROWS*COLS == 1)
    {
      return a._data[0] <=> b;
    }

    //! three-way comparison of FieldMatrix<1,1> with scalar
    template<Concept::Number S>
    friend constexpr auto operator<=> (const S& a, const FieldMatrix& b) noexcept
        requires(ROWS*COLS == 1)
    {
      return a <=> b._data[0];
    }

    /// @}
  };

  /** \brief Sends the matrix to an output stream */
  template<typename K>
  std::ostream& operator<< (std::ostream& s, const FieldMatrix<K,1,1>& a)
  {
    s << a[0][0];
    return s;
  }

  namespace FMatrixHelp {

    //! invert scalar without changing the original matrix
    template <typename K>
    static constexpr K invertMatrix (const FieldMatrix<K,1,1> &matrix, FieldMatrix<K,1,1> &inverse)
    {
      using real_type = typename FieldTraits<K>::real_type;
      inverse[0][0] = real_type(1.0)/matrix[0][0];
      return matrix[0][0];
    }

    //! invert scalar without changing the original matrix
    template <typename K>
    static constexpr K invertMatrix_retTransposed (const FieldMatrix<K,1,1> &matrix, FieldMatrix<K,1,1> &inverse)
    {
      return invertMatrix(matrix,inverse);
    }


    //! invert 2x2 Matrix without changing the original matrix
    template <typename K>
    static constexpr K invertMatrix (const FieldMatrix<K,2,2> &matrix, FieldMatrix<K,2,2> &inverse)
    {
      using real_type = typename FieldTraits<K>::real_type;
      // code generated by maple
      K det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      K det_1 = real_type(1.0)/det;
      inverse[0][0] =   matrix[1][1] * det_1;
      inverse[0][1] = - matrix[0][1] * det_1;
      inverse[1][0] = - matrix[1][0] * det_1;
      inverse[1][1] =   matrix[0][0] * det_1;
      return det;
    }

    //! invert 2x2 Matrix without changing the original matrix
    //! return transposed matrix
    template <typename K>
    static constexpr K invertMatrix_retTransposed (const FieldMatrix<K,2,2> &matrix, FieldMatrix<K,2,2> &inverse)
    {
      using real_type = typename FieldTraits<K>::real_type;
      // code generated by maple
      K det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      K det_1 = real_type(1.0)/det;
      inverse[0][0] =   matrix[1][1] * det_1;
      inverse[1][0] = - matrix[0][1] * det_1;
      inverse[0][1] = - matrix[1][0] * det_1;
      inverse[1][1] =   matrix[0][0] * det_1;
      return det;
    }

    //! invert 3x3 Matrix without changing the original matrix
    template <typename K>
    static constexpr K invertMatrix (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
    {
      using real_type = typename FieldTraits<K>::real_type;
      // code generated by maple
      K t4  = matrix[0][0] * matrix[1][1];
      K t6  = matrix[0][0] * matrix[1][2];
      K t8  = matrix[0][1] * matrix[1][0];
      K t10 = matrix[0][2] * matrix[1][0];
      K t12 = matrix[0][1] * matrix[2][0];
      K t14 = matrix[0][2] * matrix[2][0];

      K det = (t4*matrix[2][2]-t6*matrix[2][1]-t8*matrix[2][2]+
               t10*matrix[2][1]+t12*matrix[1][2]-t14*matrix[1][1]);
      K t17 = real_type(1.0)/det;

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
    static constexpr K invertMatrix_retTransposed (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
    {
      using real_type = typename FieldTraits<K>::real_type;
      // code generated by maple
      K t4  = matrix[0][0] * matrix[1][1];
      K t6  = matrix[0][0] * matrix[1][2];
      K t8  = matrix[0][1] * matrix[1][0];
      K t10 = matrix[0][2] * matrix[1][0];
      K t12 = matrix[0][1] * matrix[2][0];
      K t14 = matrix[0][2] * matrix[2][0];

      K det = (t4*matrix[2][2]-t6*matrix[2][1]-t8*matrix[2][2]+
               t10*matrix[2][1]+t12*matrix[1][2]-t14*matrix[1][1]);
      K t17 = real_type(1.0)/det;

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
    static constexpr void multMatrix ( const FieldMatrix< K, m, n > &A,
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
    static constexpr void multTransposedMatrix(const FieldMatrix<K,rows,cols> &matrix, FieldMatrix<K,cols,cols>& ret)
    {
      typedef typename FieldMatrix<K,rows,cols>::size_type size_type;

      for(size_type i=0; i<cols; i++)
        for(size_type j=0; j<cols; j++)
        {
          ret[i][j]=0.0;
          for(size_type k=0; k<rows; k++)
            ret[i][j]+=matrix[k][i]*matrix[k][j];
        }
    }

    using Dune::DenseMatrixHelp::multAssign;

    //! calculates ret = matrix^T * x
    template <typename K, int rows, int cols>
    static constexpr void multAssignTransposed( const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,rows> & x, FieldVector<K,cols> & ret)
    {
      typedef typename FieldMatrix<K,rows,cols>::size_type size_type;

      for(size_type i=0; i<cols; ++i)
      {
        ret[i] = 0.0;
        for(size_type j=0; j<rows; ++j)
          ret[i] += matrix[j][i]*x[j];
      }
    }

    //! calculates ret = matrix * x
    template <typename K, int rows, int cols>
    static constexpr FieldVector<K,rows> mult(const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,cols> & x)
    {
      FieldVector<K,rows> ret;
      multAssign(matrix,x,ret);
      return ret;
    }

    //! calculates ret = matrix^T * x
    template <typename K, int rows, int cols>
    static constexpr FieldVector<K,cols> multTransposed(const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,rows> & x)
    {
      FieldVector<K,cols> ret;
      multAssignTransposed( matrix, x, ret );
      return ret;
    }

  } // end namespace FMatrixHelp

  /** @} end documentation */

} // end namespace

#include "fmatrixev.hh"
#endif
