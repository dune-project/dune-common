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
      @brief A dense n x m matrix.

     Matrices represent linear maps from a vector space V to a vector space W.
       This class represents such a linear map by storing a two-dimensional
       %array of numbers of a given field type K. The number of rows and
       columns is given at compile time.
   */
  template<class K, int ROWS, int COLS>
  class FieldMatrix : public DenseMatrix< FieldMatrix<K,ROWS,COLS> >
  {
    std::array< FieldVector<K,COLS>, ROWS > _data;
    typedef DenseMatrix< FieldMatrix<K,ROWS,COLS> > Base;
  public:

    //! The number of rows.
    constexpr static int rows = ROWS;
    //! The number of columns.
    constexpr static int cols = COLS;

    typedef typename Base::size_type size_type;
    typedef typename Base::row_type row_type;

    typedef typename Base::row_reference row_reference;
    typedef typename Base::const_row_reference const_row_reference;

    //===== constructors
    /** \brief Default constructor
     */
    constexpr FieldMatrix() = default;

    /** \brief Constructor initializing the matrix from a list of vector
     */
    FieldMatrix(std::initializer_list<Dune::FieldVector<K, cols> > const &l) {
      assert(l.size() == rows); // Actually, this is not needed any more!
      std::copy_n(l.begin(), std::min(static_cast<std::size_t>(ROWS),
                                      l.size()),
                 _data.begin());
    }

    template <class T,
              typename = std::enable_if_t<HasDenseMatrixAssigner<FieldMatrix, T>::value>>
    FieldMatrix(T const& rhs)
    {
      *this = rhs;
    }

    using Base::operator=;

    //! copy assignment operator
    FieldMatrix& operator=(const FieldMatrix&) = default;

    //! copy assignment from FieldMatrix over a different field
    template<typename T>
    FieldMatrix& operator=(const FieldMatrix<T, ROWS, COLS>& x)
    {
      _data = x._data;
      return *this;
    }

    //! no copy assignment from FieldMatrix of different size
    template <typename T, int rows, int cols>
    FieldMatrix& operator=(FieldMatrix<T,rows,cols> const&) = delete;

    //! Return transposed of the matrix as FieldMatrix
    FieldMatrix<K, COLS, ROWS> transposed() const
    {
      Dune::FieldMatrix<K, COLS, ROWS> AT;
      for( int i = 0; i < ROWS; ++i )
        for( int j = 0; j < COLS; ++j )
          AT[j][i] = (*this)[i][j];
      return AT;
    }

    //! vector space addition -- two-argument version
    template <class OtherScalar>
    friend auto operator+ ( const FieldMatrix& matrixA,
                            const FieldMatrix<OtherScalar,ROWS,COLS>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherScalar>::PromotedType,ROWS,COLS> result;

      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrixA[i][j] + matrixB[i][j];

      return result;
    }

    //! vector space subtraction -- two-argument version
    template <class OtherScalar>
    friend auto operator- ( const FieldMatrix& matrixA,
                            const FieldMatrix<OtherScalar,ROWS,COLS>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherScalar>::PromotedType,ROWS,COLS> result;

      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrixA[i][j] - matrixB[i][j];

      return result;
    }

    //! vector space multiplication with scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator* ( const FieldMatrix& matrix, Scalar scalar)
    {
      FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,ROWS,COLS> result;

      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrix[i][j] * scalar;

      return result;
    }

    //! vector space multiplication with scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator* ( Scalar scalar, const FieldMatrix& matrix)
    {
      FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,ROWS,COLS> result;

      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = scalar * matrix[i][j];

      return result;
    }

    //! vector space division by scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator/ ( const FieldMatrix& matrix, Scalar scalar)
    {
      FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,ROWS,COLS> result;

      for (size_type i = 0; i < ROWS; ++i)
        for (size_type j = 0; j < COLS; ++j)
          result[i][j] = matrix[i][j] / scalar;

      return result;
    }

    /** \brief Matrix-matrix multiplication
     */
    template <class OtherScalar, int otherCols>
    friend auto operator* ( const FieldMatrix& matrixA,
                            const FieldMatrix<OtherScalar, COLS, otherCols>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherScalar>::PromotedType,ROWS,otherCols> result;

      for (size_type i = 0; i < matrixA.mat_rows(); ++i)
        for (size_type j = 0; j < matrixB.mat_cols(); ++j)
        {
          result[i][j] = 0;
          for (size_type k = 0; k < matrixA.mat_cols(); ++k)
            result[i][j] += matrixA[i][k] * matrixB[k][j];
        }

      return result;
    }

    /** \brief Matrix-matrix multiplication
     *
     * This implements multiplication of a FieldMatrix with another matrix
     * of type OtherMatrix. The latter has to provide
     * OtherMatrix::field_type, OtherMatrix::cols, and OtherMatrix::mtv(x,y).
     */
    template <class OtherMatrix, std::enable_if_t<
      Impl::IsStaticSizeMatrix_v<OtherMatrix>
      and not Impl::IsFieldMatrix_v<OtherMatrix>
      , int> = 0>
    friend auto operator* ( const FieldMatrix& matrixA,
                            const OtherMatrix& matrixB)
    {
      using Field = typename PromotionTraits<K, typename OtherMatrix::field_type>::PromotedType;
      Dune::FieldMatrix<Field, rows ,OtherMatrix::cols> result;
      for (std::size_t j=0; j<rows; ++j)
        matrixB.mtv(matrixA[j], result[j]);
      return result;
    }

    /** \brief Matrix-matrix multiplication
     *
     * This implements multiplication of another matrix
     * of type OtherMatrix with a FieldMatrix. The former has to provide
     * OtherMatrix::field_type, OtherMatrix::rows, and OtherMatrix::mv(x,y).
     */
    template <class OtherMatrix, std::enable_if_t<
      Impl::IsStaticSizeMatrix_v<OtherMatrix>
      and not Impl::IsFieldMatrix_v<OtherMatrix>
      , int> = 0>
    friend auto operator* ( const OtherMatrix& matrixA,
                            const FieldMatrix& matrixB)
    {
      using Field = typename PromotionTraits<K, typename OtherMatrix::field_type>::PromotedType;
      Dune::FieldMatrix<Field, OtherMatrix::rows, cols> result;
      for (std::size_t j=0; j<cols; ++j)
      {
        auto B_j = Impl::ColumnVectorView(matrixB, j);
        auto result_j = Impl::ColumnVectorView(result, j);
        matrixA.mv(B_j, result_j);
      }
      return result;
    }

    //! Multiplies M from the left to this matrix, this matrix is not modified
    template<int l>
    FieldMatrix<K,l,cols> leftmultiplyany (const FieldMatrix<K,l,rows>& M) const
    {
      FieldMatrix<K,l,cols> C;

      for (size_type i=0; i<l; i++) {
        for (size_type j=0; j<cols; j++) {
          C[i][j] = 0;
          for (size_type k=0; k<rows; k++)
            C[i][j] += M[i][k]*(*this)[k][j];
        }
      }
      return C;
    }

    using Base::rightmultiply;

    //! Multiplies M from the right to this matrix
    template <int r, int c>
    FieldMatrix& rightmultiply (const FieldMatrix<K,r,c>& M)
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
    FieldMatrix<K,rows,l> rightmultiplyany (const FieldMatrix<K,cols,l>& M) const
    {
      FieldMatrix<K,rows,l> C;

      for (size_type i=0; i<rows; i++) {
        for (size_type j=0; j<l; j++) {
          C[i][j] = 0;
          for (size_type k=0; k<cols; k++)
            C[i][j] += (*this)[i][k]*M[k][j];
        }
      }
      return C;
    }

    // make this thing a matrix
    static constexpr size_type mat_rows() { return ROWS; }
    static constexpr size_type mat_cols() { return COLS; }

    row_reference mat_access ( size_type i )
    {
      DUNE_ASSERT_BOUNDS(i < ROWS);
      return _data[i];
    }

    const_row_reference mat_access ( size_type i ) const
    {
      DUNE_ASSERT_BOUNDS(i < ROWS);
      return _data[i];
    }
  };

#ifndef DOXYGEN // hide specialization
  /** \brief Special type for 1x1 matrices
   */
  template<class K>
  class FieldMatrix<K,1,1> : public DenseMatrix< FieldMatrix<K,1,1> >
  {
    FieldVector<K,1> _data;
    typedef DenseMatrix< FieldMatrix<K,1,1> > Base;
  public:
    // standard constructor and everything is sufficient ...

    //===== type definitions and constants

    //! The type used for index access and size operations
    typedef typename Base::size_type size_type;

    //! The number of block levels we contain.
    //! This is always one for this type.
    constexpr static int blocklevel = 1;

    typedef typename Base::row_type row_type;

    typedef typename Base::row_reference row_reference;
    typedef typename Base::const_row_reference const_row_reference;

    //! \brief The number of rows.
    //! This is always one for this type.
    constexpr static int rows = 1;
    //! \brief The number of columns.
    //! This is always one for this type.
    constexpr static int cols = 1;

    //===== constructors
    /** \brief Default constructor
     */
    constexpr FieldMatrix() = default;

    /** \brief Constructor initializing the matrix from a list of vector
     */
    FieldMatrix(std::initializer_list<Dune::FieldVector<K, 1>> const &l)
    {
      std::copy_n(l.begin(), std::min(static_cast< std::size_t >( 1 ), l.size()), &_data);
    }

    template <class T,
              typename = std::enable_if_t<HasDenseMatrixAssigner<FieldMatrix, T>::value>>
    FieldMatrix(T const& rhs)
    {
      *this = rhs;
    }

    using Base::operator=;

    //! Return transposed of the matrix as FieldMatrix
    FieldMatrix<K, 1, 1> transposed() const
    {
      return *this;
    }

    //! vector space addition -- two-argument version
    template <class OtherScalar>
    friend auto operator+ ( const FieldMatrix& matrixA,
                            const FieldMatrix<OtherScalar,1,1>& matrixB)
    {
      return FieldMatrix<typename PromotionTraits<K,OtherScalar>::PromotedType,1,1>{matrixA[0][0] + matrixB[0][0]};
    }

    //! Binary addition when treating FieldMatrix<K,1,1> like K
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator+ ( const FieldMatrix& matrix,
                            const Scalar& scalar)
    {
      return FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,1,1>{matrix[0][0] + scalar};
    }

    //! Binary addition when treating FieldMatrix<K,1,1> like K
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator+ ( const Scalar& scalar,
                            const FieldMatrix& matrix)
    {
      return FieldMatrix<typename PromotionTraits<Scalar,K>::PromotedType,1,1>{scalar + matrix[0][0]};
    }

    //! vector space subtraction -- two-argument version
    template <class OtherScalar>
    friend auto operator- ( const FieldMatrix& matrixA,
                            const FieldMatrix<OtherScalar,1,1>& matrixB)
    {
      return FieldMatrix<typename PromotionTraits<K,OtherScalar>::PromotedType,1,1>{matrixA[0][0] - matrixB[0][0]};
    }

    //! Binary subtraction when treating FieldMatrix<K,1,1> like K
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator- ( const FieldMatrix& matrix,
                            const Scalar& scalar)
    {
      return FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,1,1>{matrix[0][0] - scalar};
    }

    //! Binary subtraction when treating FieldMatrix<K,1,1> like K
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator- ( const Scalar& scalar,
                            const FieldMatrix& matrix)
    {
      return FieldMatrix<typename PromotionTraits<Scalar,K>::PromotedType,1,1>{scalar - matrix[0][0]};
    }

    //! vector space multiplication with scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator* ( const FieldMatrix& matrix, Scalar scalar)
    {
      return FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,1,1> {matrix[0][0] * scalar};
    }

    //! vector space multiplication with scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator* ( Scalar scalar, const FieldMatrix& matrix)
    {
      return FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,1,1> {scalar * matrix[0][0]};
    }

    //! vector space division by scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator/ ( const FieldMatrix& matrix, Scalar scalar)
    {
      return FieldMatrix<typename PromotionTraits<K,Scalar>::PromotedType,1,1> {matrix[0][0] / scalar};
    }

    //===== solve

        /** \brief Matrix-matrix multiplication
     */
    template <class OtherScalar, int otherCols>
    friend auto operator* ( const FieldMatrix& matrixA,
                            const FieldMatrix<OtherScalar, 1, otherCols>& matrixB)
    {
      FieldMatrix<typename PromotionTraits<K,OtherScalar>::PromotedType,1,otherCols> result;

      for (size_type j = 0; j < matrixB.mat_cols(); ++j)
        result[0][j] = matrixA[0][0] * matrixB[0][j];

      return result;
    }

    /** \brief Matrix-matrix multiplication
     *
     * This implements multiplication of a FieldMatrix with another matrix
     * of type OtherMatrix. The latter has to provide
     * OtherMatrix::field_type, OtherMatrix::cols, and OtherMatrix::mtv(x,y).
     */
    template <class OtherMatrix, std::enable_if_t<
      Impl::IsStaticSizeMatrix_v<OtherMatrix>
      and not Impl::IsFieldMatrix_v<OtherMatrix>
      and (OtherMatrix::rows==1)
      , int> = 0>
    friend auto operator* ( const FieldMatrix& matrixA,
                            const OtherMatrix& matrixB)
    {
      using Field = typename PromotionTraits<K, typename OtherMatrix::field_type>::PromotedType;
      Dune::FieldMatrix<Field, rows ,OtherMatrix::cols> result;
      for (std::size_t j=0; j<rows; ++j)
        matrixB.mtv(matrixA[j], result[j]);
      return result;
    }

    /** \brief Matrix-matrix multiplication
     *
     * This implements multiplication of another matrix
     * of type OtherMatrix with a FieldMatrix. The former has to provide
     * OtherMatrix::field_type, OtherMatrix::rows, and OtherMatrix::mv(x,y).
     */
    template <class OtherMatrix, std::enable_if_t<
      Impl::IsStaticSizeMatrix_v<OtherMatrix>
      and not Impl::IsFieldMatrix_v<OtherMatrix>
      and (OtherMatrix::cols==1)
      , int> = 0>
    friend auto operator* ( const OtherMatrix& matrixA,
                            const FieldMatrix& matrixB)
    {
      using Field = typename PromotionTraits<K, typename OtherMatrix::field_type>::PromotedType;
      Dune::FieldMatrix<Field, OtherMatrix::rows, cols> result;
      for (std::size_t j=0; j<cols; ++j)
      {
        auto B_j = Impl::ColumnVectorView(matrixB, j);
        auto result_j = Impl::ColumnVectorView(result, j);
        matrixA.mv(B_j, result_j);
      }
      return result;
    }

    //! Multiplies M from the left to this matrix, this matrix is not modified
    template<int l>
    FieldMatrix<K,l,1> leftmultiplyany (const FieldMatrix<K,l,1>& M) const
    {
      FieldMatrix<K,l,1> C;
      for (size_type j=0; j<l; j++)
        C[j][0] = M[j][0]*(*this)[0][0];
      return C;
    }

    //! left multiplication
    FieldMatrix& rightmultiply (const FieldMatrix& M)
    {
      _data[0] *= M[0][0];
      return *this;
    }

    //! Multiplies M from the right to this matrix, this matrix is not modified
    template<int l>
    FieldMatrix<K,1,l> rightmultiplyany (const FieldMatrix<K,1,l>& M) const
    {
      FieldMatrix<K,1,l> C;

      for (size_type j=0; j<l; j++)
        C[0][j] = M[0][j]*_data[0];
      return C;
    }

    // make this thing a matrix
    static constexpr size_type mat_rows() { return 1; }
    static constexpr size_type mat_cols() { return 1; }

    row_reference mat_access ([[maybe_unused]] size_type i)
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return _data;
    }

    const_row_reference mat_access ([[maybe_unused]] size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return _data;
    }

    //! add scalar
    FieldMatrix& operator+= (const K& k)
    {
      _data[0] += k;
      return (*this);
    }

    //! subtract scalar
    FieldMatrix& operator-= (const K& k)
    {
      _data[0] -= k;
      return (*this);
    }

    //! multiplication with scalar
    FieldMatrix& operator*= (const K& k)
    {
      _data[0] *= k;
      return (*this);
    }

    //! division by scalar
    FieldMatrix& operator/= (const K& k)
    {
      _data[0] /= k;
      return (*this);
    }

    //===== conversion operator

    operator const K& () const { return _data[0]; }

  };

  /** \brief Sends the matrix to an output stream */
  template<typename K>
  std::ostream& operator<< (std::ostream& s, const FieldMatrix<K,1,1>& a)
  {
    s << a[0][0];
    return s;
  }

#endif // DOXYGEN

  namespace FMatrixHelp {

    //! invert scalar without changing the original matrix
    template <typename K>
    static inline K invertMatrix (const FieldMatrix<K,1,1> &matrix, FieldMatrix<K,1,1> &inverse)
    {
      using real_type = typename FieldTraits<K>::real_type;
      inverse[0][0] = real_type(1.0)/matrix[0][0];
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
    static inline K invertMatrix_retTransposed (const FieldMatrix<K,2,2> &matrix, FieldMatrix<K,2,2> &inverse)
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
    static inline K invertMatrix (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
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
    static inline K invertMatrix_retTransposed (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
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
    static inline void multAssignTransposed( const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,rows> & x, FieldVector<K,cols> & ret)
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

  } // end namespace FMatrixHelp

  /** @} end documentation */

} // end namespace

#include "fmatrixev.hh"
#endif
