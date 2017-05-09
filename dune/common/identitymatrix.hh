#ifndef DUNE_COMMON_IDENTITYMATRIX_HH
#define DUNE_COMMON_IDENTITYMATRIX_HH

#warning Deprecated since dune-common 2.5: If you really do need an identity matrix, use DiagonalMatrix or ScalarIdentityMatrix (from dune-istl) instead!

#include <dune/common/boundschecking.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/math.hh>

/**
 * \file
 * \ingroup DenseMatVec
 * \brief Implementation of an identity matrix that does not store
 *        any data.
 * \author Christoph Gersbacher
 */

namespace Dune
{

  // IdentityMatrix
  // --------------

  /** \class IdentityMatrix
   *
   *  \ingroup DenseMatVec
   *
   *  \brief Read-only identity matrix.
   *
   *  Implementation of an identity matrix that does not store any data.
   *
   *  \tparam  K  field type
   *  \tparam  N  dimension
   */
  template< class K, int N >
  struct IdentityMatrix
  {
    /** \brief field type */
    typedef K field_type;
    /** \brief size type */
    typedef std::size_t size_type;

    /** \brief return number of rows */
    constexpr size_type rows () const { return N; }
    /** \brief return number of columns */
    constexpr size_type cols () const { return N; }

    /** \copydoc Dune::DenseMatrix::mv */
    template< class X, class Y >
    void mv ( const X &x, Y &y ) const
    {
      y = x;
    }

    /** \copydoc Dune::DenseMatrix::mtv */
    template< class X, class Y >
    void mtv ( const X &x, Y &y ) const
    {
      y = x;
    }

    /** \copydoc Dune::DenseMatrix::umv */
    template< class X, class Y >
    void umv ( const X &x, Y &y ) const
    {
      y += x;
    }

    /** \copydoc Dune::DenseMatrix::umtv */
    template< class X, class Y >
    void umtv ( const X &x, Y &y ) const
    {
      y += x;
    }

    /** \copydoc Dune::DenseMatrix::umhv */
    template< class X, class Y >
    void umhv ( const X &x, Y &y ) const
    {
      y += x;
    }

    /** \copydoc Dune::DenseMatrix::mmv */
    template< class X, class Y >
    void mmv ( const X &x, Y &y ) const
    {
      y -= x;
    }

    /** \copydoc Dune::DenseMatrix::mmtv */
    template< class X, class Y >
    void mmtv ( const X &x, Y &y ) const
    {
      y -= x;
    }

    /** \copydoc Dune::DenseMatrix::mmhv */
    template< class X, class Y >
    void mmhv ( const X &x, Y &y ) const
    {
      y -= x;
    }

    /** \copydoc Dune::DenseMatrix::usmv */
    template< class X, class Y >
    void usmv (const typename FieldTraits<Y>::field_type & alpha,
      const X& x, Y& y) const
    {
      y.axpy( alpha, x );
    }

    /** \copydoc Dune::DenseMatrix::usmtv */
    template< class X, class Y >
    void usmtv (const typename FieldTraits<Y>::field_type & alpha,
      const X& x, Y& y) const
    {
      y.axpy( alpha, x );
    }

    /** \copydoc Dune::DenseMatrix::usmhv */
    template< class X, class Y >
    void usmhv (const typename FieldTraits<Y>::field_type & alpha,
      const X& x, Y& y) const
    {
      y.axpy( alpha, x );
    }

    /** \copydoc Dune::DenseMatrix::frobenius_norm */
    typename FieldTraits< field_type >::real_type frobenius_norm () const
    {
      return std::sqrt( frobenius_norm2() );
    }

    /** \copydoc Dune::DenseMatrix::frobenius_norm2 */
    typename FieldTraits< field_type >::real_type frobenius_norm2 () const
    {
      return FieldTraits< field_type >::real_type( N );
    }

    /** \copydoc Dune::DenseMatrix::infinity_norm */
    typename FieldTraits< field_type >::real_type infinity_norm () const
    {
      return FieldTraits< field_type >::real_type( 1 );
    }

    /** \copydoc Dune::DenseMatrix::infinity_norm_real */
    typename FieldTraits< field_type >::real_type infinity_norm_real () const
    {
      return FieldTraits< field_type >::real_type( 1 );
    }
  };

  template <class DenseMatrix, class field, int N>
  struct DenseMatrixAssigner<DenseMatrix, IdentityMatrix<field, N>> {
    static void apply(DenseMatrix &denseMatrix, IdentityMatrix<field, N> const &rhs) {
      DUNE_ASSERT_BOUNDS(denseMatrix.M() == N);
      DUNE_ASSERT_BOUNDS(denseMatrix.N() == N);
      denseMatrix = field(0);
      for (int i = 0; i < N; ++i)
        denseMatrix[i][i] = field(1);
    }
  };
} // namespace Dune

#endif // #ifndef DUNE_COMMON_IDENTITYMATRIX_HH
