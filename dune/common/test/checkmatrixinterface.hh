// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_CHECK_MATRIX_INTERFACE_HH
#define DUNE_COMMON_CHECK_MATRIX_INTERFACE_HH

#include <algorithm>
#include <limits>

#include <dune/common/dynvector.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/fvector.hh>


/*
 * @file
 * @brief This file provides an interface check for dense matrices.
 * @author Christoph Gersbacher
 */


namespace Dune
{

  // External forward declarations for namespace Dune
  // ------------------------------------------------

  template< class, int, int > class FieldMatrix;
  template< class, int > class DiagonalMatrix;

} // namespace Dune



namespace CheckMatrixInterface
{

  namespace Capabilities
  {

    // hasStaticSizes
    // --------------

    template< class Matrix >
    struct hasStaticSizes
    {
      static const bool v = false;
      static const int rows = ~0;
      static const int cols = ~0;
    };

    template< class Matrix >
    struct hasStaticSizes< const Matrix >
    {
      static const bool v = hasStaticSizes< Matrix >::v;
      static const int rows = hasStaticSizes< Matrix >::rows;
      static const int cols = hasStaticSizes< Matrix >::cols;
    };



    // isSquare
    // --------

    template< class Matrix >
    struct isSquare
    {
      static const bool v = false;
    };

    template< class Matrix >
    struct isSquare< const Matrix >
    {
      static const bool v = isSquare< Matrix >::v;
    };



    // Template specializations for Dune::FieldMatrix
    // ----------------------------------------------

    template< class K, int r, int c >
    struct hasStaticSizes< Dune::FieldMatrix< K, r, c > >
    {
      static const bool v = true;
      static const int rows = r;
      static const int cols = c;
    };

    template< class K, int rows, int cols >
    struct isSquare< Dune::FieldMatrix< K, rows, cols > >
    {
      static const bool v = ( rows == cols );
    };



    // Template specializations for Dune::DiagonalMatrix
    // -------------------------------------------------

    template< class K, int n >
    struct hasStaticSizes< Dune::DiagonalMatrix<K,n> >
    {
      static const bool v = true;
      static const int rows = n;
      static const int cols = n;
    };

    template< class K, int n >
    struct isSquare< Dune::DiagonalMatrix<K,n> >
    {
      static const bool v = true;
    };

  } // namespace Capabilities



  // UseDynamicVector
  // ----------------

  template< class Matrix >
  struct UseDynamicVector
  {
    typedef typename Matrix::value_type value_type;

    typedef Dune::DynamicVector< value_type > domain_type;
    typedef domain_type range_type;

    static domain_type domain ( const Matrix &matrix, value_type v = value_type() )
    {
      return domain_type( matrix.M(), v );
    }

    static range_type range ( const Matrix &matrix, value_type v = value_type() )
    {
      return range_type( matrix.N(), v );
    }
  };



  // UseFieldVector
  // --------------

  template< class K, int rows, int cols >
  struct UseFieldVector
  {
    typedef K value_type;

    typedef Dune::FieldVector< K, cols > domain_type;
    typedef Dune::FieldVector< K, rows > range_type;

    template< class Matrix >
    static domain_type domain ( const Matrix &, value_type v = value_type() )
    {
      return domain_type( v );
    }

    template< class Matrix >
    static range_type range ( const Matrix &, value_type v = value_type() )
    {
      return range_type( v );
    }
  };



  // MatrixSizeHelper
  // ----------------

  template< class Matrix, bool hasStaticSizes = Capabilities::hasStaticSizes< Matrix >::v >
  struct MatrixSizeHelper;

  template< class Matrix >
  struct MatrixSizeHelper< Matrix, false >
  {
    typedef typename Matrix::size_type size_type;
    static const size_type rows ( const Matrix &matrix ) { return matrix.rows(); }
    static const size_type cols ( const Matrix &matrix ) { return matrix.cols(); }
  };

  template< class Matrix >
  struct MatrixSizeHelper< Matrix, true >
  {
    typedef typename Matrix::size_type size_type;
    static const size_type rows ( const Matrix & ) { return Matrix::rows; }
    static const size_type cols ( const Matrix & ) { return Matrix::cols; }
  };



  // CheckIfSquareMatrix
  // -------------------

  template< class Matrix, class Traits, bool isSquare = Capabilities::isSquare< Matrix >::v >
  struct CheckIfSquareMatrix;

  template< class Matrix, class Traits >
  struct CheckIfSquareMatrix< Matrix, Traits, false >
  {
    static void apply ( const Matrix &) {}

    static void apply ( Matrix &) {}
  };

  template< class Matrix, class Traits >
  struct CheckIfSquareMatrix< Matrix, Traits, true >
  {
    typedef typename Matrix::value_type value_type;

    static value_type tolerance ()
    {
      return value_type( 16 ) * std::numeric_limits< value_type >::epsilon();
    }

    static void apply ( const Matrix &matrix )
    {
      const value_type determinant = matrix.determinant();
      if( determinant > tolerance() )
      {
        typename Traits::domain_type x = Traits::domain( matrix );
        const typename Traits::range_type b = Traits::range( matrix );
        matrix.solve( x, b );
      }
    }

    static void apply ( Matrix &matrix )
    {
      apply( const_cast< const Matrix & >( matrix ) );
      if( matrix.determinant() > tolerance() )
        matrix.invert();
    }
  };



  // CheckConstMatrix
  // ----------------

  template< class Matrix, class Traits >
  struct CheckConstMatrix
  {
    // required type definitions
    typedef typename Matrix::size_type size_type;

    typedef typename Matrix::value_type value_type;
    typedef typename Matrix::field_type field_type;
    typedef typename Matrix::block_type block_type;

    typedef typename Matrix::const_row_reference const_row_reference;

    typedef typename Matrix::ConstIterator ConstIterator;

    static void apply ( const Matrix &matrix )
    {
      checkDataAccess ( matrix );
      checkIterators ( matrix );
      checkLinearAlgebra ( matrix );
      checkNorms ( matrix );
      checkSizes ( matrix );
      CheckIfSquareMatrix< Matrix, Traits >::apply( matrix );

      // TODO: check comparison
      // bool operator == ( const Matrix &other );
      // bool operator != ( const Matrix &other );
    }

    // check size methods
    static void checkSizes ( const Matrix &matrix )
    {
      [[maybe_unused]] const size_type size = matrix.size();
      const size_type rows = MatrixSizeHelper< Matrix >::rows( matrix );
      const size_type cols = MatrixSizeHelper< Matrix >::cols( matrix );
      const size_type N = matrix.N();
      const size_type M = matrix.M();

      if( N != rows || M != cols )
        DUNE_THROW( Dune::RangeError, "Returned inconsistent sizes." );
    }

    // check read-only access to data
    static void checkDataAccess ( const Matrix &matrix )
    {
      const size_type size = matrix.size();
      for( size_type i = size_type( 0 ); i < size; ++i )
        [[maybe_unused]] const_row_reference row = matrix[ i ];

      const size_type rows = MatrixSizeHelper< Matrix >::rows( matrix );
      const size_type cols = MatrixSizeHelper< Matrix >::cols( matrix );
      for( size_type i = size_type( 0 ); i < rows; ++i )
      {
        for( size_type j = size_type( 0 ); j < cols; ++j )
        {
          [[maybe_unused]] bool exists = matrix.exists( i, j );
          [[maybe_unused]] const value_type &value = matrix[ i ][ j ];
        }
      }
    }

    // check norms
    static void checkNorms ( const Matrix &matrix )
    {
      typedef typename Dune::FieldTraits< value_type >::real_type real_type;
      real_type frobenius_norm = matrix.frobenius_norm();
      real_type frobenius_norm2 = matrix.frobenius_norm2();
      real_type infinity_norm = matrix.infinity_norm() ;
      real_type infinity_norm_real = matrix.infinity_norm_real();

      if( std::min( std::min( frobenius_norm, frobenius_norm2 ),
                    std::min( infinity_norm, infinity_norm_real ) ) < real_type( 0 ) )
        DUNE_THROW( Dune::InvalidStateException, "Norms must return non-negative value." );
    }

    // check basic linear algebra methods
    static void checkLinearAlgebra ( const Matrix &matrix )
    {
      typename Traits::domain_type domain = Traits::domain( matrix );
      typename Traits::range_type range = Traits::range( matrix );
      typename Traits::value_type alpha( 1 );

      matrix.mv( domain, range );
      matrix.mtv( range, domain );
      matrix.umv( domain, range );
      matrix.umtv( range, domain );
      matrix.umhv( range, domain );
      matrix.mmv( domain, range );
      matrix.mmtv( range, domain );
      matrix.mmhv( range, domain );
      matrix.usmv( alpha, domain, range );
      matrix.usmtv( alpha, range, domain );
      matrix.usmhv( alpha, range, domain );
    }

    // check iterator methods
    static void checkIterators ( const Matrix &matrix )
    {
      const ConstIterator end = matrix.end();
      for( ConstIterator it = matrix.begin(); it != end; ++it )
        [[maybe_unused]] const_row_reference row = *it;
    }
  };



  // CheckNonConstMatrix
  // -------------------

  template< class Matrix, class Traits >
  struct CheckNonConstMatrix
  {
    // required type definitions
    typedef typename Matrix::size_type size_type;
    typedef typename Matrix::value_type value_type;
    typedef typename Matrix::row_reference row_reference;
    typedef typename Matrix::row_type row_type;
    typedef typename Matrix::Iterator Iterator;

    static void apply ( Matrix &matrix )
    {
      checkIterators( matrix );
      checkAssignment( matrix );

      CheckIfSquareMatrix< Matrix, Traits >::apply( matrix );

      // TODO: check scalar/matrix and matrix/matrix operations
      // Matrix &operator+= ( const Matrix &other );
      // Matrix &operator-= ( const Matrix &other );
      // Matrix &operator*= ( const value_type &v );
      // Matrix &operator/= ( const value_type &v );
      // Matrix &axpy += ( const value_type &v, const Matrix &other );
      // Matrix &axpy += ( const value_type &v, const Matrix &other );
      // Matrix &leftmultiply ( const Matrix &other );
      // Matrix &rightmultiply ( const Matrix &other );
    }

    // check assignment
    static void checkAssignment ( Matrix &matrix )
    {
      matrix = value_type( 1 );

      const size_type size = matrix.size();
      for( size_type i = size_type( 0 ); i < size; ++i )
      {
        row_reference row = matrix[ i ];
        row = row_type( value_type( 0 ) );
      }

      const size_type rows = MatrixSizeHelper< Matrix >::rows( matrix );
      const size_type cols = MatrixSizeHelper< Matrix >::cols( matrix );
      for( size_type i = size_type( 0 ); i < rows; ++i )
      {
        for( size_type j = size_type( 0 ); j < cols; ++j )
          matrix[ i ][ j ] = ( i == j ? value_type( 1 ) : value_type( 0 ) );
      }
    }

    // check iterator methods
    static void checkIterators ( Matrix &matrix )
    {
      const Iterator end = matrix.end();
      for( Iterator it = matrix.begin(); it != end; ++it )
      {
        row_reference row = *it;
        row = row_type( value_type( 0 ) );
      }
    }
  };

} // namespace CheckMatrixInterface



namespace Dune
{

  // checkMatrixInterface
  // --------------------

  template< class Matrix, class Traits  = CheckMatrixInterface::UseDynamicVector< Matrix > >
  void checkMatrixInterface ( const Matrix &matrix )
  {
    CheckMatrixInterface::CheckConstMatrix< Matrix, Traits >::apply( matrix );
  }

  template< class Matrix, class Traits  = CheckMatrixInterface::UseDynamicVector< Matrix > >
  void checkMatrixInterface ( Matrix &matrix )
  {
    checkMatrixInterface( const_cast< const Matrix & >( matrix ) );
    CheckMatrixInterface::CheckNonConstMatrix< Matrix, Traits >::apply( matrix );
  }

} // namespace Dune

#endif // #ifndef DUNE_COMMON_CHECK_MATRIX_INTERFACE_HH
