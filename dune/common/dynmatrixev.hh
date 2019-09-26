// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DYNMATRIXEIGENVALUES_HH
#define DUNE_DYNMATRIXEIGENVALUES_HH

#include <memory>

#include "dynmatrix.hh"
#include "fmatrixev.hh"

/*!
   \file
   \brief utility functions to compute eigenvalues for
   dense matrices.
   \addtogroup DenseMatVec
    @{
 */

namespace Dune {

  namespace DynamicMatrixHelp {

    using Dune::FMatrixHelp::eigenValuesNonsymLapackCall;

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
        auto matrixVector = std::make_unique<double[]>(N*N);

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
        auto eigenR = std::make_unique<double[]>(N);
        auto eigenI = std::make_unique<double[]>(N);
        auto work = std::make_unique<double[]>(3*N);

        // return value information
        long int info = 0;
        const long int lwork = 3*N;

        // call LAPACK routine (see fmatrixev_ext.cc)
        eigenValuesNonsymLapackCall(&jobvl, &jobvr, &N, matrixVector.get(), &N,
                                    eigenR.get(), eigenI.get(), nullptr, &N, nullptr, &N, work.get(),
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
