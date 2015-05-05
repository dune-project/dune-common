// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FMATRIXEIGENVALUES_HH
#define DUNE_FMATRIXEIGENVALUES_HH

/** \file
 * \brief Eigenvalue computations for the FieldMatrix class
 */

#include <iostream>
#include <cmath>
#include <cassert>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

namespace Dune {

  /**
     @addtogroup DenseMatVec
     @{
   */

  namespace FMatrixHelp {

    // defined in fmatrixev.cc
    extern void eigenValuesLapackCall(
      const char* jobz, const char* uplo, const long
      int* n, double* a, const long int* lda, double* w,
      double* work, const long int* lwork, long int* info);

    extern void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, double* a, const long int* lda, double* wr, double* wi, double* vl,
      const long int* ldvl, double* vr, const long int* ldvr, double* work,
      const long int* lwork, const long int* info);

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues FieldVector that contains eigenvalues in
                    ascending order
     */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 1, 1>& matrix,
                            FieldVector<K, 1>& eigenvalues)
    {
      eigenvalues[0] = matrix[0][0];
    }

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues FieldVector that contains eigenvalues in
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

    /** \brief Calculates the eigenvalues of a symmetric 3x3 field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues Eigenvalues in ascending order

        \note If the input matrix is not symmetric the behavior of this method is undefined.

        This implementation was adapted from the pseudo-code (Python?) implementation found on
        http://en.wikipedia.org/wiki/Eigenvalue_algorithm  (retrieved late August 2014).
        Wikipedia claims to have taken it from
          Smith, Oliver K. (April 1961), Eigenvalues of a symmetric 3 × 3 matrix.,
          Communications of the ACM 4 (4): 168, doi:10.1145/355578.366316
     */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 3, 3>& matrix,
                            FieldVector<K, 3>& eigenvalues)
    {
      K p1 = matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2] + matrix[1][2]*matrix[1][2];

      if (p1 <= 1e-8)
      {
        // A is diagonal.
        eigenvalues[0] = matrix[0][0];
        eigenvalues[1] = matrix[1][1];
        eigenvalues[2] = matrix[2][2];
      }
      else
      {
        // q = trace(A)/3
        K q = 0;
        for (int i=0; i<3; i++)
          q += matrix[i][i]/3.0;

        K p2 = (matrix[0][0] - q)*(matrix[0][0] - q) + (matrix[1][1] - q)*(matrix[1][1] - q) + (matrix[2][2] - q)*(matrix[2][2] - q) + 2 * p1;
        K p = std::sqrt(p2 / 6);
        // B = (1 / p) * (A - q * I);       // I is the identity matrix
        FieldMatrix<K,3,3> B;
        for (int i=0; i<3; i++)
          for (int j=0; j<3; j++)
            B[i][j] = (1/p) * (matrix[i][j] - q*(i==j));

        K r = B.determinant() / 2.0;

        // In exact arithmetic for a symmetric matrix  -1 <= r <= 1
        // but computation error can leave it slightly outside this range.
        K phi;
        if (r <= -1)
          phi = M_PI / 3.0;
        else if (r >= 1)
          phi = 0;
        else
          phi = std::acos(r) / 3;

        // the eigenvalues satisfy eig[2] <= eig[1] <= eig[0]
        eigenvalues[2] = q + 2 * p * cos(phi);
        eigenvalues[0] = q + 2 * p * cos(phi + (2*M_PI/3));
        eigenvalues[1] = 3 * q - eigenvalues[0] - eigenvalues[2];     // since trace(matrix) = eig1 + eig2 + eig3
      }
    }

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::dsyev is used to calculate the eigenvalues
     */
    template <int dim, typename K>
    static void eigenValues(const FieldMatrix<K, dim, dim>& matrix,
                            FieldVector<K, dim>& eigenvalues)
    {
      {
        const long int N = dim ;
        const char jobz = 'n'; // only calculate eigenvalues
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

        // call LAPACK routine (see fmatrixev.cc)
        eigenValuesLapackCall(&jobz, &uplo, &N, &matrixVector[0], &N,
                              &eigenvalues[0], &workSpace[0], &w, &info);

        if( info != 0 )
        {
          std::cerr << "For matrix " << matrix << " eigenvalue calculation failed! " << std::endl;
          DUNE_THROW(InvalidStateException,"eigenValues: Eigenvalue calculation failed!");
        }
      }
    }
    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::dgeev is used to calculate the eigen values
     */
    template <int dim, typename K, class C>
    static void eigenValuesNonSym(const FieldMatrix<K, dim, dim>& matrix,
                                  FieldVector<C, dim>& eigenValues)
    {
      {
        const long int N = dim ;
        const char jobvl = 'n';
        const char jobvr = 'n';

        // matrix to put into dgeev
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
        double eigenR[dim];
        double eigenI[dim];
        double work[3*dim];

        // return value information
        long int info = 0;
        long int lwork = 3*dim;

        // call LAPACK routine (see fmatrixev_ext.cc)
        eigenValuesNonsymLapackCall(&jobvl, &jobvr, &N, &matrixVector[0], &N,
                                    &eigenR[0], &eigenI[0], 0, &N, 0, &N, &work[0],
                                    &lwork, &info);

        if( info != 0 )
        {
          std::cerr << "For matrix " << matrix << " eigenvalue calculation failed! " << std::endl;
          DUNE_THROW(InvalidStateException,"eigenValues: Eigenvalue calculation failed!");
        }
        for (int i=0; i<N; ++i) {
          eigenValues[i].real = eigenR[i];
          eigenValues[i].imag = eigenI[i];
        }
      }

    }

  } // end namespace FMatrixHelp

  /** @} end documentation */

} // end namespace Dune
#endif
