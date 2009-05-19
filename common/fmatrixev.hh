// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FMATRIXEIGENVALUES_HH
#define DUNE_FMATRIXEIGENVALUES_HH

#include <iostream>
#include <cmath>
#include <cassert>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

#if HAVE_LAPACK
// dsyev declaration (in liblapack)
extern "C" {

  /*
   *
   **  purpose
   **  =======
   **
   **  xsyev computes all eigenvalues and, optionally, eigenvectors of a
   **  BASE DATA TYPE symmetric matrix a.
   **
   **  arguments
   **  =========
   **
   **  jobz    (input) char
   **          = 'n':  compute eigenvalues only;
   **          = 'v':  compute eigenvalues and eigenvectors.
   **
   **  uplo    (input) char
   **          = 'u':  upper triangle of a is stored;
   **          = 'l':  lower triangle of a is stored.
   **
   **  n       (input) long int
   **          the order of the matrix a.  n >= 0.
   **
   **  a       (input/output) BASE DATA TYPE array, dimension (lda, n)
   **          on entry, the symmetric matrix a.  if uplo = 'u', the
   **          leading n-by-n upper triangular part of a contains the
   **          upper triangular part of the matrix a.  if uplo = 'l',
   **          the leading n-by-n lower triangular part of a contains
   **          the lower triangular part of the matrix a.
   **          on exit, if jobz = 'v', then if info = 0, a contains the
   **          orthonormal eigenvectors of the matrix a.
   **          if jobz = 'n', then on exit the lower triangle (if uplo='l')
   **          or the upper triangle (if uplo='u') of a, including the
   **          diagonal, is destroyed.
   **
   **  lda     (input) long int
   **          the leading dimension of the array a.  lda >= max(1,n).
   **
   **  w       (output) BASE DATA TYPE array, dimension (n)
   **          if info = 0, the eigenvalues in ascending order.
   **
   **
   **
   **  info    (output) long int
   **          = 0:  successful exit
   **          < 0:  if info = -i, the i-th argument had an illegal value
   **          > 0:  if info = i, the algorithm failed to converge; i
   **                off-diagonal elements of an intermediate tridiagonal
   **                form did not converge to zero.
   **
   **/
  extern void dsyev_(const char* jobz, const char* uplo, const long
                     int* n, double* a, const long int* lda, double* w,
                     double* work, const long int* lwork, long int* info);
} // end extern C
#endif

namespace Dune {

  /**
     @addtogroup DenseMatVec
     @{
   */

  namespace FMatrixHelp {

    /** \brief calculates the eigen values of a field matrix
        \param[in]  matrix matrix eigen values are calculated for
        \param[out] eigenvalues FieldVector that contains eigen values in
                    ascending order
     */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 1, 1>& matrix,
                            FieldVector<K, 1>& eigenvalues)
    {
      eigenvalues[0] = matrix[0][0];
    }

    /** \brief calculates the eigen values of a field matrix
        \param[in]  matrix matrix eigen values are calculated for
        \param[out] eigenvalues FieldVector that contains eigen values in
                    ascending order
     */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 2, 2>& matrix,
                            FieldVector<K, 2>& eigenvalues)
    {
      const K detM = matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1];
      const K p = 0.5 * (matrix[0][0] + matrix [1][1]);
      K q = p * p - detM;
      if( q < 0 && q > -1e-14 ) q = 0;
      if (p < 0 || q < 0)
      {
        std::cout << p << " p | q " << q << "\n";
        std::cout << matrix << std::endl;
        std::cout << "something went wrong in Eigenvalues for matrix!" << std::endl;
        assert(false);
        abort();
      }

      // get square root
      q = std :: sqrt(q);

      // store eigenvalues in ascending order
      eigenvalues[0] = p - q;
      eigenvalues[1] = p + q;
    }

    /** \brief calculates the eigen values of a field matrix
        \param[in]  matrix matrix eigen values are calculated for
        \param[out] eigenvalues FieldVector that contains eigen values in
                    ascending order

        \note LAPACK::dsyev is used to calculate the eigen values
     */
    template <int dim, typename K>
    static void eigenValues(const FieldMatrix<K, dim, dim>& matrix,
                            FieldVector<K, dim>& eigenvalues)
    {
#if HAVE_LAPACK
      {
        const long int N = dim ;
        const char jobz = 'n'; // only calculate eigen values
        const char uplo = 'u'; // use upper triangular matrix

        // length of matrix vector
        const long int w = N * N ;

        // matrix to put into dsyev
        double matrixVector[dim * dim];

        // copy matrix
        int row = 0;
        for(int i=0; i<dim; ++i)
        {
          for(int j=0; j<dim; ++j, ++row)
          {
            matrixVector[ row ] = matrix[ i ][ j ];
          }
        }

        // working memory
        double workSpace[dim * dim];

        // return value information
        long int info = 0;

        // call LAPACK dsyev
        dsyev_(&jobz, &uplo, &N, &matrixVector[0], &N,
               &eigenvalues[0], &workSpace[0], &w, &info);

        if( info != 0 )
        {
          std::cerr << "For matrix " << matrix << " eigen value calculation falied! " << std::endl;
          DUNE_THROW(InvalidStateException,"eigenValues: Eigenvalue calculation failed!");
        }
      }
#else
      DUNE_THROW(NotImplemented,"LAPACK is not availble, therefore no eigen value calculation");
#endif
    }

  } // end namespace FMatrixHelp

  /** @} end documentation */

} // end namespace Dune
#endif
