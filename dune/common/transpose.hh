// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_TRANSPOSE_HH
#define DUNE_COMMON_TRANSPOSE_HH

#include <cstddef>
#include <functional>

#include <dune/common/std/type_traits.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/promotiontraits.hh>
#include <dune/common/referencehelper.hh>
#include <dune/common/dynmatrix.hh>
#include <dune/common/matrixconcepts.hh>

namespace Dune {

namespace Impl {



  template<class M, bool = IsStaticSizeMatrix<M>::value>
  struct TransposedDenseMatrixTraits
  {
    using type = Dune::FieldMatrix<typename FieldTraits<M>::field_type, M::cols, M::rows>;
  };

  template<class M>
  struct TransposedDenseMatrixTraits<M, false>
  {
    using type = Dune::DynamicMatrix<typename FieldTraits<M>::field_type>;
  };

  template<class M>
  using TransposedDenseMatrixTraits_t = typename TransposedDenseMatrixTraits<M>::type;



  // CRTP mixin class to provide the static part of the interface,
  // namely enums rows and cols.
  template<class WM, bool staticSize = IsStaticSizeMatrix<WM>::value>
  class TransposedMatrixWrapperMixin {};

  template<class WM>
  class TransposedMatrixWrapperMixin<WM, true>
  {
  public:

    //! The number of rows.
    constexpr static int rows = WM::cols;
    //! The number of columns.
    constexpr static int cols = WM::rows;
  };



  template<class M>
  class TransposedMatrixWrapper;

} // namespace Impl

// Specialization of FieldTraits needs to be in namespace Dune::
template<class M>
struct FieldTraits< Impl::TransposedMatrixWrapper<M> >
{
  using field_type = typename FieldTraits<ResolveRef_t<M>>::field_type;
  using real_type = typename FieldTraits<ResolveRef_t<M>>::real_type;
};

namespace Impl {

  // Wrapper representing the transposed of a matrix.
  // Creating the wrapper does not compute anything
  // but only serves for tagging the wrapped matrix
  // for transposition. This class will store M by value.
  // To support reference-semantic, it supports using
  // M=std::reference_wrapper<OriginalMatrixType>.
  template<class M>
  class TransposedMatrixWrapper :
    public TransposedMatrixWrapperMixin<ResolveRef_t<M>>
  {
    constexpr static bool hasStaticSize = IsStaticSizeMatrix<ResolveRef_t<M>>::value;
  public:
    using WrappedMatrix = ResolveRef_t<M>;

    const WrappedMatrix& wrappedMatrix() const {
      return resolveRef(matrix_);
    }


    using value_type = typename WrappedMatrix::value_type;
    using field_type = typename FieldTraits<WrappedMatrix>::field_type;

    TransposedMatrixWrapper(M&& matrix) : matrix_(std::move(matrix)) {}
    TransposedMatrixWrapper(const M& matrix) : matrix_(matrix) {}

    template<class MatrixA,
      std::enable_if_t<
        ((not Impl::IsFieldMatrix<MatrixA>::value) or (not hasStaticSize))
        and Impl::IsDenseMatrix_v<MatrixA>, int> = 0>
    friend auto operator* (const MatrixA& matrixA, const TransposedMatrixWrapper& matrixB)
    {
      using FieldA = typename FieldTraits<MatrixA>::field_type;
      using FieldB = typename FieldTraits<TransposedMatrixWrapper>::field_type;
      using Field = typename PromotionTraits<FieldA, FieldB>::PromotedType;

      // We exploit that the rows of AB^T are the columns of (AB^T)^T = BA^T.
      // Hence we get the row-vectors of AB^T by mutiplying B to the row-vectors
      // of A.
      if constexpr(IsStaticSizeMatrix_v<MatrixA> and IsStaticSizeMatrix_v<WrappedMatrix>)
      {
        FieldMatrix<Field, MatrixA::rows, WrappedMatrix::rows> result;
        for (std::size_t j=0; j<MatrixA::rows; ++j)
          matrixB.wrappedMatrix().mv(matrixA[j], result[j]);
        return result;
      }
      else
      {
        DynamicMatrix<Field> result(matrixA.N(), matrixB.wrappedMatrix().N());
        for (std::size_t j=0; j<matrixA.N(); ++j)
          matrixB.wrappedMatrix().mv(matrixA[j], result[j]);
        return result;
      }
    }

    template<class X, class Y>
    void mv (const X& x, Y& y) const
    {
      wrappedMatrix().mtv(x,y);
    }

    template<class X, class Y>
    void mtv (const X& x, Y& y) const
    {
      wrappedMatrix().mv(x,y);
    }

    // Return a classical representation of the matrix.
    // Since we do not know the internals of the wrapped
    // matrix, this will always be a dense matrix. Depending
    // on whether the matrix has static size or not, this
    // will be either a FieldMatrix or a DynamicMatrix.
    TransposedDenseMatrixTraits_t<WrappedMatrix> asDense() const
    {
      TransposedDenseMatrixTraits_t<WrappedMatrix> MT;
      if constexpr(not IsStaticSizeMatrix<WrappedMatrix>::value)
      {
        MT.resize(wrappedMatrix().M(), wrappedMatrix().N(), 0);
      }
      for(auto&& [M_i, i] : Dune::sparseRange(wrappedMatrix()))
        for(auto&& [M_ij, j] : Dune::sparseRange(M_i))
          MT[j][i] = M_ij;
      return MT;
    }

  private:

    M matrix_;
  };

  template<class M>
  using MemberFunctionTransposedConcept = std::void_t<decltype(std::declval<M>().transposed())>;

  template<class M>
  struct HasMemberFunctionTransposed : public Dune::Std::is_detected<MemberFunctionTransposedConcept, M> {};

} // namespace Impl



/**
 * \brief Return the transposed of the given matrix
 *
 * \param matrix The matrix to be transposed.
 *
 * This overload is selected if the given matrix supports \code matrix.transposed() \endcode.
 * It will return the result of \code matrix.transposed() \endcode.
 */
template<class Matrix,
  std::enable_if_t<Impl::HasMemberFunctionTransposed<Matrix>::value, int> = 0>
auto transpose(const Matrix& matrix) {
  return matrix.transposed();
}



/**
 * \brief Create a wrapper modelling the transposed matrix
 *
 * \param matrix The matrix to be transposed.
 *
 * This overload is selected if the given matrix does not support \code matrix.transposed() \endcode.
 * It will return a wrapper storing a copy of the given matrix.
 *
 * Currently the wrapper only implements
 * \code
 * auto c = a*transpose(b);
 * \endcode
 * if a is a FieldMatrix of appropriate size. This is
 * optimal even for sparse b because it only relies on
 * calling \code b.mv(a[i], c[i]) \endcode for the rows of a.
 * Furthermore the wrapper can be converted to a suitable
 * dense FieldMatrix using the \code asDense() \endcode method
 * if the wrapped matrix allows to iterate over its entries
 * and matrix-vector multiplication using \code transpose(b).mv(x,y) \endcode
 * if the wrapped matrix provides the \code b.mtv(x,y) \endcode.
 */
template<class Matrix,
  std::enable_if_t<not Impl::HasMemberFunctionTransposed<std::decay_t<Matrix>>::value, int> = 0>
auto transpose(Matrix&& matrix) {
  return Impl::TransposedMatrixWrapper(std::forward<Matrix>(matrix));
}



/**
 * \brief Create a wrapper modelling the transposed matrix
 *
 * \param matrix The matrix to be transposed.
 *
 * This overload is selected if the given value is a \code std::reference_wrapper \endcode
 * of a matrix.
 * It will return a wrapper storing a reference of the given matrix.
 *
 * Currently the wrapper only implements
 * \code
 * auto c = a*transpose(b);
 * \endcode
 * if a is a FieldMatrix of appropriate size. This is
 * optimal even for sparse b because it only relies on
 * calling b.mv(a[i], c[i]) for the rows of a.
 * Furthermore the wrapper can be converted to a suitable
 * dense FieldMatrix using the \code adDense() \endcode method
 * if the wrapped matrix allows to iterate over its entries
 * and matrix-vector multiplication using \code transpose(b).mv(x,y) \endcode
 * if the wrapped matrix provides the \code b.mtv(x,y) \endcode.
 *
 * This specialization allows to pass a \code std::reference_wrapper \endcode
 * of a matrix to explicitly request, that the latter is stored by
 * reference in the wrapper.
 */
template<class Matrix>
auto transpose(const std::reference_wrapper<Matrix>& matrix) {
  return Impl::TransposedMatrixWrapper(matrix);
}



/**
 * \brief Create a view modelling the transposed matrix
 *
 * \param matrix The matrix to be transposed.
 *
 * The returned view stores a reference of the given matrix.
 * Calling \code transposedView(matrix) \endcode is equivalent to
 * \code transpose(std::cref(matrix)) \endcode.
 */
template<class Matrix>
auto transposedView(const Matrix& matrix) {
  return transpose(std::cref(matrix));
}





} // namespace Dune

#endif // DUNE_COMMON_TRANSPOSE_HH
