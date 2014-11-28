// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DYNMATRIXEIGENVALUES_HH
#define DUNE_DYNMATRIXEIGENVALUES_HH

#include <memory>

#include <dune/common/std/memory.hh>

#include "dynmatrix.hh"

/*!
   \file
   \brief utility functions to compute eigenvalues for
   dense matrices.
   \addtogroup DenseMatVec
    @{
 */

namespace Dune {

  namespace DynamicMatrixHelp {

    // defined in fmatrixev_ext.cpp
    extern void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, double* a, const long int* lda, double* wr, double* wi, double* vl,
      const long int* ldvl, double* vr, const long int* ldvr, double* work,
      const long int* lwork, const long int* info);

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::dgeev is used to calculate the eigen values
     */
    template <typename K, class C>
    static void eigenValuesNonSym(const DynamicMatrix<K>& matrix,
                                  DynamicVector<C>& eigenValues)
    {
      {
        const long int N = matrix.rows();
        const char jobvl = 'n';
        const char jobvr = 'n';


        // matrix to put into dgeev
        std::unique_ptr<double[]> matrixVector = Std::make_unique<double[]>(N*N);

        // copy matrix
        int row = 0;
        for(int i=0; i<N; ++i)
        {
          for(int j=0; j<N; ++j, ++row)
          {
            matrixVector[ row ] = matrix[ i ][ j ];
          }
        }

        // working memory
        std::unique_ptr<double[]> eigenR = Std::make_unique<double[]>(N);
        std::unique_ptr<double[]> eigenI = Std::make_unique<double[]>(N);
        std::unique_ptr<double[]> work = Std::make_unique<double[]>(3*N);

        // return value information
        long int info = 0;
        long int lwork = 3*N;

        // call LAPACK routine (see fmatrixev_ext.cc)
        eigenValuesNonsymLapackCall(&jobvl, &jobvr, &N, &matrixVector[0], &N,
                                    &eigenR[0], &eigenI[0], 0, &N, 0, &N, &work[0],
                                    &lwork, &info);

        if( info != 0 )
        {
          std::cerr << "For matrix " << matrix << " eigenvalue calculation failed! " << std::endl;
          DUNE_THROW(InvalidStateException,"eigenValues: Eigenvalue calculation failed!");
        }

        eigenValues.resize(N);
        for (int i=0; i<N; ++i)
          eigenValues[i] = std::complex<double>(eigenR[i], eigenI[i]);
      }
    }

  }

}
/** @} */
#endif
