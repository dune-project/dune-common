// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_FMATRIX_HH
#define DUNE_FMATRIX_HH

#include <cmath>
#include <cstddef>
#include <iostream>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/densematrix.hh>
#include <dune/common/precision.hh>
#include <dune/common/static_assert.hh>
#include <dune/common/std/constexpr.hh>

namespace Dune
{

  /**
      @addtogroup DenseMatVec
      @{
   */

  /*! \file

     \brief  Implements a matrix constructed from a given type
     representing a field and compile-time given number of rows and columns.
   */

  template< class K, int ROWS, int COLS > class FieldMatrix;

  template< class K, int ROWS, int COLS >
  struct DenseMatVecTraits< FieldMatrix<K,ROWS,COLS> >
  {
    typedef FieldMatrix<K,ROWS,COLS> derived_type;

    // each row is implemented by a field vector
    typedef FieldVector<K,COLS> row_type;

    typedef row_type &row_reference;
    typedef const row_type &const_row_reference;

    typedef Dune::array<row_type,ROWS> container_type;
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
    Dune::array< FieldVector<K,COLS>, ROWS > _data;
    typedef DenseMatrix< FieldMatrix<K,ROWS,COLS> > Base;
  public:

    //! export size
    enum {
      //! The number of rows.
      rows = ROWS,
      //! The number of columns.
      cols = COLS
    };

    typedef typename Base::size_type size_type;
    typedef typename Base::row_type row_type;

    typedef typename Base::row_reference row_reference;
    typedef typename Base::const_row_reference const_row_reference;

    //===== constructors
    /** \brief Default constructor
     */
    FieldMatrix () {}

    /** \brief Constructor initializing the whole matrix with a scalar
     */
    explicit FieldMatrix (const K& k)
    {
      for (size_type i=0; i<rows; i++) _data[i] = k;
    }

    template<typename T>
    explicit FieldMatrix (const T& t)
    {
      DenseMatrixAssigner<Conversion<T,K>::exists>::assign(*this, t);
    }

    //===== assignment
    using Base::operator=;

    // To be removed!
#if 0
    //! Multiplies M from the left to this matrix
    FieldMatrix& leftmultiply (const FieldMatrix<K,rows,rows>& M)
    {
      FieldMatrix<K,rows,cols> C(*this);

      for (size_type i=0; i<rows; i++)
        for (size_type j=0; j<cols; j++) {
          (*this)[i][j] = 0;
          for (size_type k=0; k<rows; k++)
            (*this)[i][j] += M[i][k]*C[k][j];
        }

      return *this;
    }
#endif

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

    //! Multiplies M from the right to this matrix
    FieldMatrix& rightmultiply (const FieldMatrix<K,cols,cols>& M)
    {
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
    DUNE_CONSTEXPR size_type mat_rows() const { return ROWS; }
    DUNE_CONSTEXPR size_type mat_cols() const { return COLS; }

    row_reference mat_access ( size_type i )
    {
      assert(i < ROWS);
      return _data[i];
    }

    const_row_reference mat_access ( size_type i ) const
    {
      assert(i < ROWS);
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

    //! We are at the leaf of the block recursion
    enum {
      //! The number of block levels we contain.
      //! This is always one for this type.
      blocklevel = 1
    };

    typedef typename Base::row_type row_type;

    typedef typename Base::row_reference row_reference;
    typedef typename Base::const_row_reference const_row_reference;

    //! export size
    enum {
      //! \brief The number of rows.
      //! This is always one for this type.
      rows = 1,
      //! \brief The number of columns.
      //! This is always one for this type.
      cols = 1
    };

    //===== constructors
    /** \brief Default constructor
     */
    FieldMatrix () {}

    /** \brief Constructor initializing the whole matrix with a scalar
     */
    FieldMatrix (const K& k)
    {
      _data[0] = k;
    }
    template<typename T>
    FieldMatrix(const T& t)
    {
      DenseMatrixAssigner<Conversion<T,K>::exists>::assign(*this, t);
    }

    //===== solve

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
    DUNE_CONSTEXPR size_type mat_rows() const { return 1; }
    DUNE_CONSTEXPR size_type mat_cols() const { return 1; }

    row_reference mat_access ( size_type i )
    {
      assert(i == 0);
      return _data;
    }

    const_row_reference mat_access ( size_type i ) const
    {
      assert(i == 0);
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

    operator K () const { return _data[0]; }

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
      K det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      K det_1 = 1.0/det;
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
      K det = (matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0]);
      K det_1 = 1.0/det;
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

    //! invert 3x3 Matrix without changing the original matrix
    template <typename K>
    static inline K invertMatrix_retTransposed (const FieldMatrix<K,3,3> &matrix, FieldMatrix<K,3,3> &inverse)
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

#if 0
    //! calculates ret = matrix * x
    template <typename K, int rows, int cols>
    static inline void multAssign(const FieldMatrix<K,rows,cols> &matrix, const FieldVector<K,cols> & x, FieldVector<K,rows> & ret)
    {
      typedef typename FieldMatrix<K,rows,cols>::size_type size_type;

      for(size_type i=0; i<rows; ++i)
      {
        ret[i] = 0.0;
        for(size_type j=0; j<cols; ++j)
        {
          ret[i] += matrix[i][j]*x[j];
        }
      }
    }
#else
    using Dune::DenseMatrixHelp::multAssign;
#endif

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
