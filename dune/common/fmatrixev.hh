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

    /** \brief calculates the eigenvalues of a symetric field matrix
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

    /** \brief calculates the eigenvalues of a symetric field matrix
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

    /** \brief calculates the eigenvalues of a symetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::dsyev is used to calculate the eigen values
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

  } // end namespace FMatrixHelp

  /** @} end documentation */

} // end namespace Dune
#endif
