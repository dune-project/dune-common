// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_TRANSPOSE_HH
#define DUNE_COMMON_TRANSPOSE_HH

#include <cstddef>

#include <dune/common/fmatrix.hh>
#include <dune/common/promotiontraits.hh>

namespace Dune {

namespace Impl {

  // Wrapper representing the transposed of a matrix.
  // Creating the wrapper does not compute anything
  // but only serves for tagging the wrapped matrix
  // for transposition.
  template<class M>
  class TransposedMatrixWrapper
  {
  public:

    enum {
      //! The number of rows.
      rows = M::cols,
      //! The number of columns.
      cols = M::rows
    };

    TransposedMatrixWrapper(const M& matrix) : matrix_(matrix) {}
    TransposedMatrixWrapper(const TransposedMatrixWrapper&) = delete;
    TransposedMatrixWrapper(TransposedMatrixWrapper&&) = delete;

    template<class OtherField, int otherRows>
    friend auto operator* (const FieldMatrix<OtherField, otherRows, rows>& matrixA,
                            const TransposedMatrixWrapper& matrixB)
    {
      using ThisField = typename FieldTraits<M>::field_type;
      using Field = typename PromotionTraits<ThisField, OtherField>::PromotedType;
      FieldMatrix<Field, otherRows, cols> result;
      for (std::size_t j=0; j<otherRows; ++j)
        matrixB.matrix_.mv(matrixA[j], result[j]);
      return result;
    }

  private:

    const M& matrix_;
  };

} // namespace Impl

/**
 * \brief Create a wrapper modelling the transposed matrix
 *
 * Currently the wrapper only implements
 * \code
 * auto c = a*transpose(b);
 * \endcode
 * if a is a FieldMatrix of appropriate size. This is
 * optimal even for sparse b because it only relies on
 * calling b.mv(a[i], c[i]) for the rows of a.
 *
 * Since the created object only stores a reference
 * to the wrapped matrix, it cannot be modified and
 * should not be stored but used directly.
 */
template<class Matrix>
auto transpose(const Matrix& matrix) {
  return Impl::TransposedMatrixWrapper<Matrix>(matrix);
}


} // namespace Dune

#endif // DUNE_COMMON_TRANSPOSE_HH
