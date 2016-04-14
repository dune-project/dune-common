// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_MATRIXHELPER_HH
#define DUNE_COMMON_MATRIXHELPER_HH

#include <cassert>
#include <cmath>

namespace Dune
{
  // FieldHelper
  // -----------

  template< class Field >
  struct FieldHelper
  {
    static Field abs ( const Field &x ) {
      using std::abs;
      return abs( x );
    }
  };

  // MatrixHelper
  // ------------

  template< class Traits >
  struct MatrixHelper
  {
    typedef typename Traits::ctype FieldType;

    static FieldType abs ( const FieldType &x )
    {
      return FieldHelper< FieldType >::abs( x );
    }

    template< int rows, int cols >
    static void
    Ax ( const typename Traits :: template Matrix< rows, cols > :: type &A,
         const typename Traits :: template Vector< cols > :: type &x,
         typename Traits :: template Vector< rows > :: type &ret )
    {
      for( int i = 0; i < rows; ++i )
      {
        ret[ i ] = FieldType( 0 );
        for( int j = 0; j < cols; ++j )
          ret[ i ] += A[ i ][ j ] * x[ j ];
      }
    }

    template< int rows, int cols >
    static void
    ATx ( const typename Traits :: template Matrix< rows, cols > :: type &A,
          const typename Traits :: template Vector< rows > :: type &x,
          typename Traits :: template Vector< cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        ret[ i ] = FieldType( 0 );
        for( int j = 0; j < rows; ++j )
          ret[ i ] += A[ j ][ i ] * x[ j ];
      }
    }

    template< int rows, int cols, int p >
    static void
    AB ( const typename Traits :: template Matrix< rows, cols > :: type &A,
         const typename Traits :: template Matrix< cols, p > :: type &B,
         typename Traits :: template Matrix< rows, p > :: type &ret )
    {
      for( int i = 0; i < rows; ++i )
      {
        for( int j = 0; j < p; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = 0; k < cols; ++k )
            ret[ i ][ j ] += A[ i ][ k ] * B[ k ][ j ];
        }
      }
    }

    template< int rows, int cols, int p >
    static void
    ATBT ( const typename Traits :: template Matrix< rows, cols > :: type &A,
           const typename Traits :: template Matrix< p, rows > :: type &B,
           typename Traits :: template Matrix< cols, p > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        for( int j = 0; j < p; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = 0; k < rows; ++k )
            ret[ i ][ j ] += A[ k ][ i ] * B[ j ][ k ];
        }
      }
    }

    template< int rows, int cols >
    static void
    ATA_L ( const typename Traits :: template Matrix< rows, cols > :: type &A,
            typename Traits :: template Matrix< cols, cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        for( int j = 0; j <= i; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = 0; k < rows; ++k )
            ret[ i ][ j ] += A[ k ][ i ] * A[ k ][ j ];
        }
      }
    }

    template< int rows, int cols >
    static void
    ATA ( const typename Traits :: template Matrix< rows, cols > :: type &A,
          typename Traits :: template Matrix< cols, cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        for( int j = 0; j <= i; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = 0; k < rows; ++k )
            ret[ i ][ j ] += A[ k ][ i ] * A[ k ][ j ];
          ret[ j ][ i ] = ret[ i ][ j ];
        }

        ret[ i ][ i ] = FieldType( 0 );
        for( int k = 0; k < rows; ++k )
          ret[ i ][ i ] += A[ k ][ i ] * A[ k ][ i ];
      }
    }

    template< int rows, int cols >
    static void
    AAT_L ( const typename Traits :: template Matrix< rows, cols > :: type &A,
            typename Traits :: template Matrix< rows, rows > :: type &ret )
    {
      /*
         if (rows==2) {
         ret[0][0] = A[0]*A[0];
         ret[1][1] = A[1]*A[1];
         ret[1][0] = A[0]*A[1];
         }
         else
       */
      for( int i = 0; i < rows; ++i )
      {
        for( int j = 0; j <= i; ++j )
        {
          FieldType &retij = ret[ i ][ j ];
          retij = A[ i ][ 0 ] * A[ j ][ 0 ];
          for( int k = 1; k < cols; ++k )
            retij += A[ i ][ k ] * A[ j ][ k ];
        }
      }
    }

    template< int rows, int cols >
    static void
    AAT ( const typename Traits :: template Matrix< rows, cols > :: type &A,
          typename Traits :: template Matrix< rows, rows > :: type &ret )
    {
      for( int i = 0; i < rows; ++i )
      {
        for( int j = 0; j < i; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = 0; k < cols; ++k )
            ret[ i ][ j ] += A[ i ][ k ] * A[ j ][ k ];
          ret[ j ][ i ] = ret[ i ][ j ];
        }
        ret[ i ][ i ] = FieldType( 0 );
        for( int k = 0; k < cols; ++k )
          ret[ i ][ i ] += A[ i ][ k ] * A[ i ][ k ];
      }
    }

    template< int cols >
    static void
    Lx ( const typename Traits :: template Matrix< cols, cols > :: type &L,
         const typename Traits :: template Vector< cols > :: type &x,
         typename Traits :: template Vector< cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        ret[ i ] = FieldType( 0 );
        for( int j = 0; j <= i; ++j )
          ret[ i ] += L[ i ][ j ] * x[ j ];
      }
    }

    template< int cols >
    static void
    LTx ( const typename Traits :: template Matrix< cols, cols > :: type &L,
          const typename Traits :: template Vector< cols > :: type &x,
          typename Traits :: template Vector< cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        ret[ i ] = FieldType( 0 );
        for( int j = i; j < cols; ++j )
          ret[ i ] += L[ j ][ i ] * x[ j ];
      }
    }

    template< int cols >
    static void
    LTL ( const typename Traits :: template Matrix< cols, cols > :: type &L,
          typename Traits :: template Matrix< cols, cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        for( int j = 0; j < i; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = i; k < cols; ++k )
            ret[ i ][ j ] += L[ k ][ i ] * L[ k ][ j ];
          ret[ j ][ i ] = ret[ i ][ j ];
        }
        ret[ i ][ i ] = FieldType( 0 );
        for( int k = i; k < cols; ++k )
          ret[ i ][ i ] += L[ k ][ i ] * L[ k ][ i ];
      }
    }

    template< int cols >
    static void
    LLT ( const typename Traits :: template Matrix< cols, cols > :: type &L,
          typename Traits :: template Matrix< cols, cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        for( int j = 0; j < i; ++j )
        {
          ret[ i ][ j ] = FieldType( 0 );
          for( int k = 0; k <= j; ++k )
            ret[ i ][ j ] += L[ i ][ k ] * L[ j ][ k ];
          ret[ j ][ i ] = ret[ i ][ j ];
        }
        ret[ i ][ i ] = FieldType( 0 );
        for( int k = 0; k <= i; ++k )
          ret[ i ][ i ] += L[ i ][ k ] * L[ i ][ k ];
      }
    }

    template< int cols >
    static void
    cholesky_L ( const typename Traits :: template Matrix< cols, cols > :: type &A,
                 typename Traits :: template Matrix< cols, cols > :: type &ret )
    {
      for( int i = 0; i < cols; ++i )
      {
        FieldType &rii = ret[ i ][ i ];

        FieldType xDiag = A[ i ][ i ];
        for( int j = 0; j < i; ++j )
          xDiag -= ret[ i ][ j ] * ret[ i ][ j ];
        assert( xDiag > FieldType( 0 ) );
        rii = sqrt( xDiag );

        FieldType invrii = FieldType( 1 ) / rii;
        for( int k = i+1; k < cols; ++k )
        {
          FieldType x = A[ k ][ i ];
          for( int j = 0; j < i; ++j )
            x -= ret[ i ][ j ] * ret[ k ][ j ];
          ret[ k ][ i ] = invrii * x;
        }
      }
    }

    template< int cols >
    static FieldType
    detL ( const typename Traits :: template Matrix< cols, cols > :: type &L )
    {
      FieldType det = FieldType( 1 );
      for( int i = 0; i < cols; ++i )
        det *= L[ i ][ i ];
      return det;
    }

    template< int cols >
    static FieldType
    invL ( typename Traits :: template Matrix< cols, cols > :: type &L )
    {
      FieldType det = FieldType( 1 );
      for( int i = 0; i < cols; ++i )
      {
        FieldType &lii = L[ i ][ i ];
        det *= lii;
        lii = FieldType( 1 ) / lii;
        for( int j = 0; j < i; ++j )
        {
          FieldType &lij = L[ i ][ j ];
          FieldType x = lij * L[ j ][ j ];
          for( int k = j+1; k < i; ++k )
            x += L[ i ][ k ] * L[ k ][ j ];
          lij = (-lii) * x;
        }
      }
      return det;
    }

    // calculates x := L^{-1} x
    template< int cols >
    static void
    invLx ( typename Traits :: template Matrix< cols, cols > :: type &L,
            typename Traits :: template Vector< cols > :: type &x )
    {
      for( int i = 0; i < cols; ++i )
      {
        for( int j = 0; j < i; ++j )
          x[ i ] -= L[ i ][ j ] * x[ j ];
        x[ i ] /= L[ i ][ i ];
      }
    }

    // calculates x := L^{-T} x
    template< int cols >
    static void
    invLTx ( typename Traits :: template Matrix< cols, cols > :: type &L,
             typename Traits :: template Vector< cols > :: type &x )
    {
      for( int i = cols; i > 0; --i )
      {
        for( int j = i; j < cols; ++j )
          x[ i-1 ] -= L[ j ][ i-1 ] * x[ j ];
        x[ i-1 ] /= L[ i-1 ][ i-1 ];
      }
    }

    template< int cols >
    static FieldType
    spdDetA ( const typename Traits :: template Matrix< cols, cols > :: type &A )
    {
      // return A[0][0]*A[1][1]-A[1][0]*A[1][0];
      typename Traits :: template Matrix< cols, cols > :: type L;
      cholesky_L< cols >( A, L );
      return detL< cols >( L );
    }

    template< int cols >
    static FieldType
    spdInvA ( typename Traits :: template Matrix< cols, cols > :: type &A )
    {
      typename Traits :: template Matrix< cols, cols > :: type L;
      cholesky_L< cols >( A, L );
      const FieldType det = invL< cols >( L );
      LTL< cols >( L, A );
      return det;
    }

    // calculate x := A^{-1} x
    template< int cols >
    static void
    spdInvAx ( typename Traits :: template Matrix< cols, cols > :: type &A,
               typename Traits :: template Vector< cols > :: type &x )
    {
      typename Traits :: template Matrix< cols, cols > :: type L;
      cholesky_L< cols >( A, L );
      invLx< cols >( L, x );
      invLTx< cols >( L, x );
    }

    template< int rows, int cols >
    static FieldType
    detATA ( const typename Traits :: template Matrix< rows, cols > :: type &A )
    {
      if( rows >= cols )
      {
        typename Traits :: template Matrix< cols, cols > :: type ata;
        ATA_L< rows, cols >( A, ata );
        return spdDetA< cols >( ata );
      }
      else
        return FieldType( 0 );
    }

    /** \brief Compute the square root of the determinant of A times A transposed
     *
     *  This is the volume element for an embedded submanifold and needed to
     *  implement the method integrationElement().
     */
    template< int rows, int cols >
    static FieldType
    sqrtDetAAT ( const typename Traits::template Matrix< rows, cols >::type &A )
    {
      // These special cases are here not only for speed reasons:
      // The general implementation aborts if the matrix is almost singular,
      // and the special implementation provide a stable way to handle that case.
      if( (cols == 2) && (rows == 2) )
      {
        // Special implementation for 2x2 matrices: faster and more stable
        return abs( A[ 0 ][ 0 ]*A[ 1 ][ 1 ] - A[ 1 ][ 0 ]*A[ 0 ][ 1 ] );
      }
      else if( (cols == 3) && (rows == 3) )
      {
        // Special implementation for 3x3 matrices
        const FieldType v0 = A[ 0 ][ 1 ] * A[ 1 ][ 2 ] - A[ 1 ][ 1 ] * A[ 0 ][ 2 ];
        const FieldType v1 = A[ 0 ][ 2 ] * A[ 1 ][ 0 ] - A[ 1 ][ 2 ] * A[ 0 ][ 0 ];
        const FieldType v2 = A[ 0 ][ 0 ] * A[ 1 ][ 1 ] - A[ 1 ][ 0 ] * A[ 0 ][ 1 ];
        return abs( v0 * A[ 2 ][ 0 ] + v1 * A[ 2 ][ 1 ] + v2 * A[ 2 ][ 2 ] );
      }
      else if ( (cols == 3) && (rows == 2) )
      {
        // Special implementation for 2x3 matrices
        const FieldType v0 = A[ 0 ][ 0 ] * A[ 1 ][ 1 ] - A[ 0 ][ 1 ] * A[ 1 ][ 0 ];
        const FieldType v1 = A[ 0 ][ 0 ] * A[ 1 ][ 2 ] - A[ 1 ][ 0 ] * A[ 0 ][ 2 ];
        const FieldType v2 = A[ 0 ][ 1 ] * A[ 1 ][ 2 ] - A[ 0 ][ 2 ] * A[ 1 ][ 1 ];
        return sqrt( v0*v0 + v1*v1 + v2*v2);
      }
      else if( cols >= rows )
      {
        // General case
        typename Traits::template Matrix< rows, rows >::type aat;
        AAT_L< rows, cols >( A, aat );
        return spdDetA< rows >( aat );
      }
      else
        return FieldType( 0 );
    }

    // A^{-1}_L = (A^T A)^{-1} A^T
    // => A^{-1}_L A = I
    template< int rows, int cols >
    static FieldType
    leftInvA ( const typename Traits :: template Matrix< rows, cols > :: type &A,
               typename Traits :: template Matrix< cols, rows > :: type &ret )
    {
      static_assert((rows >= cols), "Matrix has no left inverse.");
      typename Traits :: template Matrix< cols, cols > :: type ata;
      ATA_L< rows, cols >( A, ata );
      const FieldType det = spdInvA< cols >( ata );
      ATBT< cols, cols, rows >( ata, A, ret );
      return det;
    }

    template< int rows, int cols >
    static void
    leftInvAx ( const typename Traits :: template Matrix< rows, cols > :: type &A,
                const typename Traits :: template Vector< rows > :: type &x,
                typename Traits :: template Vector< cols > :: type &y )
    {
      static_assert((rows >= cols), "Matrix has no left inverse.");
      typename Traits :: template Matrix< cols, cols > :: type ata;
      ATx< rows, cols >( A, x, y );
      ATA_L< rows, cols >( A, ata );
      spdInvAx< cols >( ata, y );
    }

    /** \brief Compute right pseudo-inverse of matrix A */
    template< int rows, int cols >
    static FieldType
    rightInvA ( const typename Traits :: template Matrix< rows, cols > :: type &A,
                typename Traits :: template Matrix< cols, rows > :: type &ret )
    {
      static_assert((cols >= rows), "Matrix has no right inverse.");
      if( (cols == 2) && (rows == 2) )
      {
        const FieldType det = (A[ 0 ][ 0 ]*A[ 1 ][ 1 ] - A[ 1 ][ 0 ]*A[ 0 ][ 1 ]);
        const FieldType detInv = FieldType( 1 ) / det;
        ret[ 0 ][ 0 ] = A[ 1 ][ 1 ] * detInv;
        ret[ 1 ][ 1 ] = A[ 0 ][ 0 ] * detInv;
        ret[ 1 ][ 0 ] = -A[ 1 ][ 0 ] * detInv;
        ret[ 0 ][ 1 ] = -A[ 0 ][ 1 ] * detInv;
        return abs( det );
      }
      else
      {
        typename Traits :: template Matrix< rows , rows > :: type aat;
        AAT_L< rows, cols >( A, aat );
        const FieldType det = spdInvA< rows >( aat );
        ATBT< rows, cols, rows >( A , aat , ret );
        return det;
      }
    }

    template< int rows, int cols >
    static void
    xTRightInvA ( const typename Traits :: template Matrix< rows, cols > :: type &A,
                  const typename Traits :: template Vector< cols > :: type &x,
                  typename Traits :: template Vector< rows > :: type &y )
    {
      static_assert((cols >= rows), "Matrix has no right inverse.");
      typename Traits :: template Matrix< rows, rows > :: type aat;
      Ax< rows, cols >( A, x, y );
      AAT_L< rows, cols >( A, aat );
      spdInvAx< rows >( aat, y );
    }
  };
} // namespace Dune

#endif
