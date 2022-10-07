// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_FMATRIXEIGENVALUES_HH
#define DUNE_FMATRIXEIGENVALUES_HH

/** \file
 * \brief Eigenvalue computations for the FieldMatrix class
 */

#include <algorithm>
#include <iostream>
#include <cmath>
#include <cassert>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/math.hh>

namespace Dune {

  /**
     @addtogroup DenseMatVec
     @{
   */

  namespace FMatrixHelp {

#if HAVE_LAPACK
    // defined in fmatrixev.cc
    extern void eigenValuesLapackCall(
      const char* jobz, const char* uplo, const long
      int* n, double* a, const long int* lda, double* w,
      double* work, const long int* lwork, long int* info);

    extern void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, double* a, const long int* lda, double* wr, double* wi, double* vl,
      const long int* ldvl, double* vr, const long int* ldvr, double* work,
      const long int* lwork, long int* info);

    extern void eigenValuesLapackCall(
      const char* jobz, const char* uplo, const long
      int* n, float* a, const long int* lda, float* w,
      float* work, const long int* lwork, long int* info);

    extern void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, float* a, const long int* lda, float* wr, float* wi, float* vl,
      const long int* ldvl, float* vr, const long int* ldvr, float* work,
      const long int* lwork, long int* info);

#endif

    namespace Impl {
      //internal tag to activate/disable code for eigenvector calculation at compile time
      enum Jobs { OnlyEigenvalues=0, EigenvaluesEigenvectors=1 };

      //internal dummy used if only eigenvalues are to be calculated
      template<typename K, int dim>
      using EVDummy = FieldMatrix<K, dim, dim>;

      //compute the cross-product of two vectors
      template<typename K>
      inline FieldVector<K,3> crossProduct(const FieldVector<K,3>& vec0, const FieldVector<K,3>& vec1) {
        return {vec0[1]*vec1[2] - vec0[2]*vec1[1], vec0[2]*vec1[0] - vec0[0]*vec1[2], vec0[0]*vec1[1] - vec0[1]*vec1[0]};
      }

      template <typename K>
      static void eigenValues2dImpl(const FieldMatrix<K, 2, 2>& matrix,
                                    FieldVector<K, 2>& eigenvalues)
      {
        using std::sqrt;
        const K p = 0.5 * (matrix[0][0] + matrix [1][1]);
        const K p2 = p - matrix[1][1];
        K q = p2 * p2 + matrix[1][0] * matrix[0][1];
        if( q < 0 && q > -1e-14 ) q = 0;
        if (q < 0)
        {
          std::cout << matrix << std::endl;
          // Complex eigenvalues are either caused by non-symmetric matrices or by round-off errors
          DUNE_THROW(MathError, "Complex eigenvalue detected (which this implementation cannot handle).");
        }

        // get square root
        q = sqrt(q);

        // store eigenvalues in ascending order
        eigenvalues[0] = p - q;
        eigenvalues[1] = p + q;
      }

      /*
        This implementation was adapted from the pseudo-code (Python?) implementation found on
        http://en.wikipedia.org/wiki/Eigenvalue_algorithm  (retrieved late August 2014).
        Wikipedia claims to have taken it from
          Smith, Oliver K. (April 1961), Eigenvalues of a symmetric 3 × 3 matrix.,
          Communications of the ACM 4 (4): 168, doi:10.1145/355578.366316
      */
      template <typename K>
      static K eigenValues3dImpl(const FieldMatrix<K, 3, 3>& matrix,
                                FieldVector<K, 3>& eigenvalues)
      {
        using std::sqrt;
        using std::acos;
        using real_type = typename FieldTraits<K>::real_type;
        const K pi = MathematicalConstants<K>::pi();
        K p1 = matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2] + matrix[1][2]*matrix[1][2];

        if (p1 <= std::numeric_limits<K>::epsilon()) {
          // A is diagonal.
          eigenvalues[0] = matrix[0][0];
          eigenvalues[1] = matrix[1][1];
          eigenvalues[2] = matrix[2][2];
          std::sort(eigenvalues.begin(), eigenvalues.end());

          return 0.0;
        }
        else
        {
          // q = trace(A)/3
          K q = 0;
          for (int i=0; i<3; i++)
            q += matrix[i][i] / 3.0;

          K p2 = (matrix[0][0] - q)*(matrix[0][0] - q) + (matrix[1][1] - q)*(matrix[1][1] - q) + (matrix[2][2] - q)*(matrix[2][2] - q) + 2.0 * p1;
          K p = sqrt(p2 / 6);
          // B = (1 / p) * (A - q * I);       // I is the identity matrix
          FieldMatrix<K,3,3> B;
          for (int i=0; i<3; i++)
            for (int j=0; j<3; j++)
              B[i][j] = (real_type(1.0)/p) * (matrix[i][j] - q*(i==j));

          K r = B.determinant() / 2.0;

          /*In exact arithmetic for a symmetric matrix  -1 <= r <= 1
          but computation error can leave it slightly outside this range.
          acos(z) function requires |z| <= 1, but will fail silently
          and return NaN if the input is larger than 1 in magnitude.
          Thus r is clamped to [-1,1].*/
          using std::clamp;
          r = clamp<K>(r, -1.0, 1.0);
          K phi = acos(r) / 3.0;

          // the eigenvalues satisfy eig[2] <= eig[1] <= eig[0]
          eigenvalues[2] = q + 2 * p * cos(phi);
          eigenvalues[0] = q + 2 * p * cos(phi + (2*pi/3));
          eigenvalues[1] = 3 * q - eigenvalues[0] - eigenvalues[2];     // since trace(matrix) = eig1 + eig2 + eig3

          return r;
        }
      }

      //see https://www.geometrictools.com/Documentation/RobustEigenSymmetric3x3.pdf
      //Robustly compute a right-handed orthonormal set {u, v, evec0}.
      template<typename K>
      void orthoComp(const FieldVector<K,3>& evec0, FieldVector<K,3>& u, FieldVector<K,3>& v) {
        using std::abs;
        if(abs(evec0[0]) > abs(evec0[1])) {
          //The component of maximum absolute value is either evec0[0] or evec0[2].
          FieldVector<K,2> temp = {evec0[0], evec0[2]};
          auto L = 1.0 / temp.two_norm();
          u = L * FieldVector<K,3>({-evec0[2], 0.0, evec0[0]});
        }
        else {
          //The component of maximum absolute value is either evec0[1] or evec0[2].
          FieldVector<K,2> temp = {evec0[1], evec0[2]};
          auto L = 1.0 / temp.two_norm();
          u = L * FieldVector<K,3>({0.0, evec0[2], -evec0[1]});
        }
        v = crossProduct(evec0, u);
      }

      //see https://www.geometrictools.com/Documentation/RobustEigenSymmetric3x3.pdf
      template<typename K>
      void eig0(const FieldMatrix<K,3,3>& matrix, K eval0, FieldVector<K,3>& evec0) {
        /* Compute a unit-length eigenvector for eigenvalue[i0].  The
        matrix is rank 2, so two of the rows are linearly independent.
        For a robust computation of the eigenvector, select the two
        rows whose cross product has largest length of all pairs of
        rows. */
        using Vector = FieldVector<K,3>;
        Vector row0 = {matrix[0][0]-eval0, matrix[0][1], matrix[0][2]};
        Vector row1 = {matrix[1][0], matrix[1][1]-eval0, matrix[1][2]};
        Vector row2 = {matrix[2][0], matrix[2][1], matrix[2][2]-eval0};

        Vector r0xr1 = crossProduct(row0, row1);
        Vector r0xr2 = crossProduct(row0, row2);
        Vector r1xr2 = crossProduct(row1, row2);
        auto d0 = r0xr1.two_norm();
        auto d1 = r0xr2.two_norm();
        auto d2 = r1xr2.two_norm();

        auto dmax = d0 ;
        int imax = 0;
        if(d1>dmax) {
          dmax = d1;
          imax = 1;
        }
        if(d2>dmax)
          imax = 2;

        if(imax == 0)
          evec0 = r0xr1 / d0;
        else if(imax == 1)
          evec0 = r0xr2 / d1;
        else
          evec0 = r1xr2 / d2;
      }

      //see https://www.geometrictools.com/Documentation/RobustEigenSymmetric3x3.pdf
      template<typename K>
      void eig1(const FieldMatrix<K,3,3>& matrix, const FieldVector<K,3>& evec0, FieldVector<K,3>& evec1, K eval1) {
        using Vector = FieldVector<K,3>;

        //Robustly compute a right-handed orthonormal set {u, v, evec0}.
        Vector u,v;
        orthoComp(evec0, u, v);

        /* Let e be eval1 and let E be a corresponding eigenvector which
        is a solution to the linear system (A - e*I)*E = 0.  The matrix
        (A - e*I) is 3x3, not invertible (so infinitely many
        solutions), and has rank 2 when eval1 and eval are different.
        It has rank 1 when eval1 and eval2 are equal.  Numerically, it
        is difficult to compute robustly the rank of a matrix.  Instead,
        the 3x3 linear system is reduced to a 2x2 system as follows.
        Define the 3x2 matrix J = [u,v] whose columns are the u and v
        computed previously.  Define the 2x1 vector X = J*E.  The 2x2
        system is 0 = M * X = (J^T * (A - e*I) * J) * X where J^T is
        the transpose of J and M = J^T * (A - e*I) * J is a 2x2 matrix.
        The system may be written as
            +-                        -++-  -+       +-  -+
            | U^T*A*U - e  U^T*A*V     || x0 | = e * | x0 |
            | V^T*A*U      V^T*A*V - e || x1 |       | x1 |
            +-                        -++   -+       +-  -+
        where X has row entries x0 and x1. */

        Vector Au, Av;
        matrix.mv(u, Au);
        matrix.mv(v, Av);

        auto m00 = u.dot(Au) - eval1;
        auto m01 = u.dot(Av);
        auto m11 = v.dot(Av) - eval1;

        /* For robustness, choose the largest-length row of M to compute
        the eigenvector.  The 2-tuple of coefficients of U and V in the
        assignments to eigenvector[1] lies on a circle, and U and V are
        unit length and perpendicular, so eigenvector[1] is unit length
        (within numerical tolerance). */
        using std::abs, std::sqrt, std::max;
        auto absM00 = abs(m00);
        auto absM01 = abs(m01);
        auto absM11 = abs(m11);
        if(absM00 >= absM11) {
          auto maxAbsComp = max(absM00, absM01);
          if(maxAbsComp > 0.0) {
            if(absM00 >= absM01) {
              m01 /= m00;
              m00 = 1.0 / sqrt(1.0 + m01*m01);
              m01 *= m00;
            }
            else {
              m00 /= m01;
              m01 = 1.0 / sqrt(1.0 + m00*m00);
              m00 *= m01;
            }
            evec1 = m01*u - m00*v;
          }
          else
            evec1 = u;
        }
        else {
          auto maxAbsComp = max(absM11, absM01);
          if(maxAbsComp > 0.0) {
            if(absM11 >= absM01) {
              m01 /= m11;
              m11 = 1.0 / sqrt(1.0 + m01*m01);
              m01 *= m11;
            }
            else {
              m11 /= m01;
              m01 = 1.0 / sqrt(1.0 + m11*m11);
              m11 *= m01;
            }
            evec1 = m11*u - m01*v;
          }
          else
            evec1 = u;
        }
      }

      // 1d specialization
      template<Jobs Tag, typename K>
      static void eigenValuesVectorsImpl(const FieldMatrix<K, 1, 1>& matrix,
                                     FieldVector<K, 1>& eigenValues,
                                     FieldMatrix<K, 1, 1>& eigenVectors)
      {
        eigenValues[0] = matrix[0][0];
        if constexpr(Tag==EigenvaluesEigenvectors)
          eigenVectors[0] = {1.0};
      }


      // 2d specialization
      template <Jobs Tag, typename K>
      static void eigenValuesVectorsImpl(const FieldMatrix<K, 2, 2>& matrix,
                                     FieldVector<K, 2>& eigenValues,
                                     FieldMatrix<K, 2, 2>& eigenVectors)
      {
        // Compute eigen values
        Impl::eigenValues2dImpl(matrix, eigenValues);

        // Compute eigenvectors by exploiting the Cayley–Hamilton theorem.
        // If λ_1, λ_2 are the eigenvalues, then (A - λ_1I )(A - λ_2I ) = (A - λ_2I )(A - λ_1I ) = 0,
        // so the columns of (A - λ_2I ) are annihilated by (A - λ_1I ) and vice versa.
        // Assuming neither matrix is zero, the columns of each must include eigenvectors
        // for the other eigenvalue. (If either matrix is zero, then A is a multiple of the
        // identity and any non-zero vector is an eigenvector.)
        // From: https://en.wikipedia.org/wiki/Eigenvalue_algorithm#2x2_matrices
        if constexpr(Tag==EigenvaluesEigenvectors) {

          // Special casing for multiples of the identity
          FieldMatrix<K,2,2> temp = matrix;
          temp[0][0] -= eigenValues[0];
          temp[1][1] -= eigenValues[0];
          if(temp.infinity_norm() <= 1e-14) {
            eigenVectors[0] = {1.0, 0.0};
            eigenVectors[1] = {0.0, 1.0};
          }
          else {
            // The columns of A - λ_2I are eigenvectors for λ_1, or zero.
            // Take the column with the larger norm to avoid zero columns.
            FieldVector<K,2> ev0 = {matrix[0][0]-eigenValues[1], matrix[1][0]};
            FieldVector<K,2> ev1 = {matrix[0][1], matrix[1][1]-eigenValues[1]};
            eigenVectors[0] = (ev0.two_norm2() >= ev1.two_norm2()) ? ev0/ev0.two_norm() : ev1/ev1.two_norm();

            // The columns of A - λ_1I are eigenvectors for λ_2, or zero.
            // Take the column with the larger norm to avoid zero columns.
            ev0 = {matrix[0][0]-eigenValues[0], matrix[1][0]};
            ev1 = {matrix[0][1], matrix[1][1]-eigenValues[0]};
            eigenVectors[1] = (ev0.two_norm2() >= ev1.two_norm2()) ? ev0/ev0.two_norm() : ev1/ev1.two_norm();
          }
        }
      }

      // 3d specialization
      template <Jobs Tag, typename K>
      static void eigenValuesVectorsImpl(const FieldMatrix<K, 3, 3>& matrix,
                                     FieldVector<K, 3>& eigenValues,
                                     FieldMatrix<K, 3, 3>& eigenVectors)
      {
        using Vector = FieldVector<K,3>;
        using Matrix = FieldMatrix<K,3,3>;

        //compute eigenvalues
        /* Precondition the matrix by factoring out the maximum absolute
        value of the components.  This guards against floating-point
        overflow when computing the eigenvalues.*/
        using std::isnormal;
        K maxAbsElement = (isnormal(matrix.infinity_norm())) ? matrix.infinity_norm() : K(1.0);
        Matrix scaledMatrix = matrix / maxAbsElement;
        K r = Impl::eigenValues3dImpl(scaledMatrix, eigenValues);

        if constexpr(Tag==EigenvaluesEigenvectors) {
          K offDiagNorm = Vector{scaledMatrix[0][1],scaledMatrix[0][2],scaledMatrix[1][2]}.two_norm2();
          if (offDiagNorm <= std::numeric_limits<K>::epsilon())
          {
            eigenValues = {scaledMatrix[0][0], scaledMatrix[1][1], scaledMatrix[2][2]};
            eigenVectors = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};

            // Use bubble sort to jointly sort eigenvalues and eigenvectors
            // such that eigenvalues are ascending
            if (eigenValues[0] > eigenValues[1])
            {
              std::swap(eigenValues[0], eigenValues[1]);
              std::swap(eigenVectors[0], eigenVectors[1]);
            }
            if (eigenValues[1] > eigenValues[2])
            {
              std::swap(eigenValues[1], eigenValues[2]);
              std::swap(eigenVectors[1], eigenVectors[2]);
            }
            if (eigenValues[0] > eigenValues[1])
            {
              std::swap(eigenValues[0], eigenValues[1]);
              std::swap(eigenVectors[0], eigenVectors[1]);
            }
          }
          else {
            /*Compute the eigenvectors so that the set
            [evec[0], evec[1], evec[2]] is right handed and
            orthonormal. */

            Matrix evec(0.0);
            Vector eval(eigenValues);
            if(r >= 0) {
              Impl::eig0(scaledMatrix, eval[2], evec[2]);
              Impl::eig1(scaledMatrix, evec[2], evec[1], eval[1]);
              evec[0] = Impl::crossProduct(evec[1], evec[2]);
            }
            else {
              Impl::eig0(scaledMatrix, eval[0], evec[0]);
              Impl::eig1(scaledMatrix, evec[0], evec[1], eval[1]);
              evec[2] = Impl::crossProduct(evec[0], evec[1]);
            }
            //sort eval/evec-pairs in ascending order
            using EVPair = std::pair<K, Vector>;
            std::vector<EVPair> pairs;
            for(std::size_t i=0; i<=2; ++i)
              pairs.push_back(EVPair(eval[i], evec[i]));
            auto comp = [](EVPair x, EVPair y){ return x.first < y.first; };
                                       std::sort(pairs.begin(), pairs.end(), comp);
            for(std::size_t i=0; i<=2; ++i){
              eigenValues[i] = pairs[i].first;
              eigenVectors[i] = pairs[i].second;
            }
          }
        }
        //The preconditioning scaled the matrix, which scales the eigenvalues. Revert the scaling.
        eigenValues *= maxAbsElement;
      }

      // forwarding to LAPACK with corresponding tag
      template <Jobs Tag, int dim, typename K>
      static void eigenValuesVectorsLapackImpl(const FieldMatrix<K, dim, dim>& matrix,
                                               FieldVector<K, dim>& eigenValues,
                                               FieldMatrix<K, dim, dim>& eigenVectors)
      {
        {
#if HAVE_LAPACK
          /*Lapack uses a proprietary tag to determine whether both eigenvalues and
            -vectors ('v') or only eigenvalues ('n') should be calculated */
          const char jobz = "nv"[Tag];

          const long int N = dim ;
          const char uplo = 'u'; // use upper triangular matrix

          // length of matrix vector, LWORK >= max(1,3*N-1)
          const long int lwork = 3*N -1 ;

          constexpr bool isKLapackType = std::is_same_v<K,double> || std::is_same_v<K,float>;
          using LapackNumType = std::conditional_t<isKLapackType, K, double>;

          // matrix to put into dsyev
          LapackNumType matrixVector[dim * dim];

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
          LapackNumType workSpace[lwork];

          // return value information
          long int info = 0;
          LapackNumType* ev;
          if constexpr (isKLapackType){
            ev = &eigenValues[0];
          }else{
            ev = new LapackNumType[dim];
          }

          // call LAPACK routine (see fmatrixev.cc)
          eigenValuesLapackCall(&jobz, &uplo, &N, &matrixVector[0], &N,
                                ev, &workSpace[0], &lwork, &info);

          if constexpr (!isKLapackType){
              for(size_t i=0;i<dim;++i)
                eigenValues[i] = ev[i];
              delete[] ev;
          }

          // restore eigenvectors matrix
          if (Tag==Jobs::EigenvaluesEigenvectors){
            row = 0;
            for(int i=0; i<dim; ++i)
            {
              for(int j=0; j<dim; ++j, ++row)
              {
                eigenVectors[ i ][ j ] = matrixVector[ row ];
              }
            }
          }

          if( info != 0 )
          {
            std::cerr << "For matrix " << matrix << " eigenvalue calculation failed! " << std::endl;
            DUNE_THROW(InvalidStateException,"eigenValues: Eigenvalue calculation failed!");
          }
#else
          DUNE_THROW(NotImplemented,"LAPACK not found!");
#endif
        }
      }

      // generic specialization
      template <Jobs Tag, int dim, typename K>
      static void eigenValuesVectorsImpl(const FieldMatrix<K, dim, dim>& matrix,
                                         FieldVector<K, dim>& eigenValues,
                                         FieldMatrix<K, dim, dim>& eigenVectors)
      {
        eigenValuesVectorsLapackImpl<Tag>(matrix,eigenValues,eigenVectors);
      }
    } //namespace Impl

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order

        \note specializations for dim=1,2,3 exist, for dim>3 LAPACK::dsyev is used
     */
    template <int dim, typename K>
    static void eigenValues(const FieldMatrix<K, dim, dim>& matrix,
                            FieldVector<K ,dim>& eigenValues)
    {
      Impl::EVDummy<K,dim> dummy;
      Impl::eigenValuesVectorsImpl<Impl::Jobs::OnlyEigenvalues>(matrix, eigenValues, dummy);
    }

    /** \brief calculates the eigenvalues and eigenvectors of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order
        \param[out] eigenVectors FieldMatrix that contains the eigenvectors

        \note specializations for dim=1,2,3 exist, for dim>3 LAPACK::dsyev is used
     */
    template <int dim, typename K>
    static void eigenValuesVectors(const FieldMatrix<K, dim, dim>& matrix,
                                   FieldVector<K ,dim>& eigenValues,
                                   FieldMatrix<K, dim, dim>& eigenVectors)
    {
      Impl::eigenValuesVectorsImpl<Impl::Jobs::EigenvaluesEigenvectors>(matrix, eigenValues, eigenVectors);
    }

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::dsyev is used to calculate the eigenvalues
     */
    template <int dim, typename K>
    static void eigenValuesLapack(const FieldMatrix<K, dim, dim>& matrix,
                                         FieldVector<K, dim>& eigenValues)
    {
      Impl::EVDummy<K,dim> dummy;
      Impl::eigenValuesVectorsLapackImpl<Impl::Jobs::EigenvaluesEigenvectors>(matrix, eigenValues, dummy);
    }

    /** \brief calculates the eigenvalues and -vectors of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order
        \param[out] eigenVectors FieldMatrix that contains the eigenvectors

        \note LAPACK::dsyev is used to calculate the eigenvalues and -vectors
     */
    template <int dim, typename K>
    static void eigenValuesVectorsLapack(const FieldMatrix<K, dim, dim>& matrix,
                                         FieldVector<K, dim>& eigenValues,
                                         FieldMatrix<K, dim, dim>& eigenVectors)
    {
      Impl::eigenValuesVectorsLapackImpl<Impl::Jobs::EigenvaluesEigenvectors>(matrix, eigenValues, eigenVectors);
    }

    /** \brief calculates the eigenvalues of a non-symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::dgeev is used to calculate the eigenvalues
     */
    template <int dim, typename K, class C>
    static void eigenValuesNonSym(const FieldMatrix<K, dim, dim>& matrix,
                                  FieldVector<C, dim>& eigenValues)
    {
#if HAVE_LAPACK
      {
        const long int N = dim ;
        const char jobvl = 'n';
        const char jobvr = 'n';

        constexpr bool isKLapackType = std::is_same_v<K,double> || std::is_same_v<K,float>;
        using LapackNumType = std::conditional_t<isKLapackType, K, double>;

        // matrix to put into dgeev
        LapackNumType matrixVector[dim * dim];

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
        LapackNumType eigenR[dim];
        LapackNumType eigenI[dim];
        LapackNumType work[3*dim];

        // return value information
        long int info = 0;
        const long int lwork = 3*dim;

        // call LAPACK routine (see fmatrixev_ext.cc)
        eigenValuesNonsymLapackCall(&jobvl, &jobvr, &N, &matrixVector[0], &N,
                                    &eigenR[0], &eigenI[0], nullptr, &N, nullptr, &N, &work[0],
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
#else
      DUNE_THROW(NotImplemented,"LAPACK not found!");
#endif
    }
  } // end namespace FMatrixHelp

  /** @} end documentation */

} // end namespace Dune
#endif
