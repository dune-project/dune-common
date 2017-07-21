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
#include <dune/common/math.hh>

namespace Dune {
  namespace Impl {

#if HAVE_LAPACK

// symmetric matrices
#define DSYEV_FORTRAN FC_FUNC (dsyev, DSYEV)

// generic matrices
#define DGEEV_FORTRAN FC_FUNC (dgeev, DGEEV)

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
   **  work    (workspace/output) DOUBLE PRECISION array, dimension (MAX(1,LWORK))
   **           On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
   **
   **  lwork   (input) INTEGER
   **          The length of the array WORK.  LWORK >= max(1,3*N-1).
   **          For optimal efficiency, LWORK >= (NB+2)*N,
   **          where NB is the blocksize for DSYTRD returned by ILAENV.
   **
   **          If LWORK = -1, then a workspace query is assumed; the routine
   **          only calculates the optimal size of the WORK array, returns
   **          this value as the first entry of the WORK array, and no error
   **          message related to LWORK is issued by XERBLA.
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
  extern void DSYEV_FORTRAN(const char* jobz, const char* uplo, const long int* n,
                            double* a, const long int* lda, double* w,
                            double* work, const long int* lwork, long int* info);

  /*
   *
   **  purpose
   **  =======
   **
   **  xgeev computes for an N-by-N BASE DATA TYPE nonsymmetric matrix A, the
   **  eigenvalues and, optionally, the left and/or right eigenvectors.
   **
   **  The right eigenvector v(j) of A satisfies
   **                   A * v(j) = lambda(j) * v(j)
   **  where lambda(j) is its eigenvalue.
   **  The left eigenvector u(j) of A satisfies
   **                u(j)**T * A = lambda(j) * u(j)**T
   **  where u(j)**T denotes the transpose of u(j).
   **
   **  The computed eigenvectors are normalized to have Euclidean norm
   **  equal to 1 and largest component real.
   **
   **  arguments
   **  =========
   **
   **  jobvl   (input) char
   **          = 'n': left eigenvectors of a are not computed;
   **          = 'v': left eigenvectors of a are computed.
   **
   **  jobvr   (input) char
   **          = 'n': right eigenvectors of a are not computed;
   **          = 'v': right eigenvectors of a are computed.
   **
   **  n       (input) long int
   **          the order of the matrix v. v >= 0.
   **
   **  a       (input/output) BASE DATA TYPE array, dimension (lda,n)
   **          on entry, the n-by-n matrix a.
   **          on exit, a has been overwritten.
   **
   **  lda     (input) long int
   **          the leading dimension of the array a.  lda >= max(1,n).
   **
   **  wr      (output) BASE DATA TYPE array, dimension (n)
   **  wi      (output) BASE DATA TYPE array, dimension (n)
   **          wr and wi contain the real and imaginary parts,
   **          respectively, of the computed eigenvalues.  complex
   **          conjugate pairs of eigenvalues appear consecutively
   **          with the eigenvalue having the positive imaginary part
   **          first.
   **
   **  vl      (output) COMPLEX DATA TYPE array, dimension (ldvl,n)
   **          if jobvl = 'v', the left eigenvectors u(j) are stored one
   **          after another in the columns of vl, in the same order
   **          as their eigenvalues.
   **          if jobvl = 'n', vl is not referenced.
   **          if the j-th eigenvalue is real, then u(j) = vl(:,j),
   **          the j-th column of vl.
   **          if the j-th and (j+1)-st eigenvalues form a complex
   **          conjugate pair, then u(j) = vl(:,j) + i*vl(:,j+1) and
   **          u(j+1) = vl(:,j) - i*vl(:,j+1).
   **
   **  ldvl    (input) long int
   **          the leading dimension of the array vl.  ldvl >= 1; if
   **          jobvl = 'v', ldvl >= n.
   **
   **  vr      (output) COMPLEX DATA TYPE array, dimension (ldvr,n)
   **          if jobvr = 'v', the right eigenvectors v(j) are stored one
   **          after another in the columns of vr, in the same order
   **          as their eigenvalues.
   **          if jobvr = 'n', vr is not referenced.
   **          if the j-th eigenvalue is real, then v(j) = vr(:,j),
   **          the j-th column of vr.
   **          if the j-th and (j+1)-st eigenvalues form a complex
   **          conjugate pair, then v(j) = vr(:,j) + i*vr(:,j+1) and
   **          v(j+1) = vr(:,j) - i*vr(:,j+1).
   **
   **  ldvr    (input) long int
   **          the leading dimension of the array vr.  ldvr >= 1; if
   **          jobvr = 'v', ldvr >= n.
   **
   **  work    (workspace/output) BASE DATA TYPE array, dimension (max(1,lwork))
   **          on exit, if info = 0, work(1) returns the optimal lwork.
   **
   **  lwork   (input) long int
   **          the dimension of the array work.  lwork >= max(1,3*n), and
   **          if jobvl = 'v' or jobvr = 'v', lwork >= 4*n.  for good
   **          performance, lwork must generally be larger.
   **
   **          if lwork = -1, then a workspace query is assumed; the routine
   **          only calculates the optimal size of the work array, returns
   **          this value as the first entry of the work array, and no error
   **          message related to lwork is issued by xerbla.
   **
   **  info    (output) long int
   **          = 0:  successful exit
   **          < 0:  if info = -i, the i-th argument had an illegal value.
   **          > 0:  if info = i, the qr algorithm failed to compute all the
   **                eigenvalues, and no eigenvectors have been computed;
   **                elements i+1:n of wr and wi contain eigenvalues which
   **                have converged.
   **
   **/

  extern void DGEEV_FORTRAN(const char* jobvl, const char* jobvr, const long int* n,
                            double* a, const long int* lda, double* wr, double* wi, double* vl,
                            const long int* ldvl, double* vr, const long int* ldvr, double* work,
                            const long int* lwork, const long int* info);
} // end extern C
#endif

template <typename K, int dim>
struct LapackHelper
{
  void static callXSYEV(const char* jobz, const char* uplo, const long
                        int* n, K* a, const long int* lda, K* w,
                        K* work, const long int* lwork, long int* info) {
    DUNE_THROW(Dune::NotImplemented, "callXSYEV only implemented for single and double precision");
  }
  void static callXGEEV(const char* jobvl, const char* jobvr, const long
                        int* n, K* a, const long int* lda, K* wr, K* wi, K* vl,
                        const long int* ldvl, K* vr, const long int* ldvr, K* work,
                        const long int* lwork, const long int* info) {
    DUNE_THROW(Dune::NotImplemented, "callXGEEV only implemented for single and double precision");
  }
};

template <int dim>
struct LapackHelper<double, dim>
{
  using K = double;
  void static callXSYEV(const char* jobz, const char* uplo, const long
                        int* n, K* a, const long int* lda, K* w,
                        K* work, const long int* lwork, long int* info) {
#if HAVE_LAPACK
    DSYEV_FORTRAN(jobz, uplo, n, a, lda, w, work, lwork, info);
#else
    DUNE_THROW(Dune::NotImplemented, "LAPACK not found!");
#endif
  }

  void static callXGEEV(const char* jobvl, const char* jobvr, const long
                        int* n, K* a, const long int* lda, K* wr, K* wi, K* vl,
                        const long int* ldvl, K* vr, const long int* ldvr, K* work,
                        const long int* lwork, const long int* info) {
#if HAVE_LAPACK
    DGEEV_FORTRAN(jobvl, jobvr, n, a, lda, wr, wi, vl, ldvl, vr, ldvr,
                  work, lwork, info);
#else
    DUNE_THROW(Dune::NotImplemented, "LAPACK not found!");
#endif
  }
};


template <typename K, int n>
struct DuneEigenvalueHelper
{
  void static eigenvaluesRealSymmetric(FieldMatrix<K, n, n> const &matrix,
                                   FieldVector<K, n> &eigenvalues) {
    DUNE_THROW(Dune::NotImplemented, "Only implemented for dimensions <=3");
  }
};

template <typename K>
struct DuneEigenvalueHelper<K, 1>
{
  int static const n = 1;
  void static eigenvaluesRealSymmetric(FieldMatrix<K, n, n> const &matrix,
                                   FieldVector<K, n> &eigenvalues) {
    eigenvalues[0] = matrix[0][0];
  }
};

template <typename K>
struct DuneEigenvalueHelper<K, 2>
{
  int static const n = 2;
  void static eigenvaluesRealSymmetric(FieldMatrix<K, n, n> const &matrix,
                                   FieldVector<K, n> &eigenvalues) {
    using std::sqrt;
    const K detM = matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1];
    const K p = 0.5 * (matrix[0][0] + matrix[1][1]);
    K q = p * p - detM;
    if (q < 0) {
      if (q > -1e-14) // FIXME: hardcoded tolerance
        q = 0;
      else {
        std::cout << matrix << std::endl;
        // Complex eigenvalues are either caused by non-symmetric matrices or by round-off errors
        DUNE_THROW(MathError, "Complex eigenvalue detected (which this implementation cannot handle).");
      }
    }

    // get square root
    q = sqrt(q);

    // store eigenvalues in ascending order
    eigenvalues[0] = p - q;
    eigenvalues[1] = p + q;
  }
};

/*
  This implementation was adapted from the pseudo-code (Python?) implementation found on
  http://en.wikipedia.org/wiki/Eigenvalue_algorithm  (retrieved late August 2014).
  Wikipedia claims to have taken it from
  Smith, Oliver K. (April 1961), Eigenvalues of a symmetric 3 x 3 matrix.,
  Communications of the ACM 4 (4): 168, doi:10.1145/355578.366316

  WARNING: "I did not check stability o[r] efficiency of this" -- 3e91e19881ec7a8708ba2dab17d7f8694ddb7e08
*/
template <typename K>
struct DuneEigenvalueHelper<K, 3>
{
  int static const n = 3;
  void static eigenvaluesRealSymmetric(FieldMatrix<K, n, n> const &matrix,
                                   FieldVector<K, n> &eigenvalues) {
    using std::sqrt;
    using std::acos;
    using std::cos;
    const K pi = MathematicalConstants<K>::pi();

    K p1 = matrix[0][1]*matrix[0][1] + matrix[0][2]*matrix[0][2] + matrix[1][2]*matrix[1][2];
    // q = trace(A)/3
    K q = (matrix[0][0] + matrix[1][1] + matrix[2][2]) / 3;
    K p2 = (matrix[0][0] - q)*(matrix[0][0] - q) + (matrix[1][1] - q)*(matrix[1][1] - q) + (matrix[2][2] - q)*(matrix[2][2] - q) + 2 * p1;
    K p = sqrt(p2 / 6);

    if (p == 0) {
      eigenvalues = q;
      return;
    }
    // B = (1 / p) * (A - q * I);
    FieldMatrix<K,3,3> B;
    K pinv = 1 / p;
    for (int i=0; i<3; i++) {
      B[i][i] = pinv * (matrix[i][i] - q);
      for (int j=0; j<i; j++)
        B[i][j] = B[j][i] = pinv * matrix[i][j];
    }

    K r = B.determinant() / 2;

    // In exact arithmetic for a symmetric matrix, -1 <= r <= 1
    // but computation error can leave it slightly outside this range.
    if (r < -1)
      r = -1;
    else if (r > 1)
      r = 1;

    K phi = acos(r) / 3;
    K sphi = phi + 2*pi/3;

    eigenvalues[2] = q + 2 * p * cos(phi);
    eigenvalues[0] = q + 2 * p * cos(sphi);
    eigenvalues[1] = 3 * q - eigenvalues[0] - eigenvalues[2]; // since trace(matrix) = eig1 + eig2 + eig3
  }
};

template <typename K, int n>
struct LapackEigenvalueHelper
{
  void static eigenvaluesRealGeneric(FieldMatrix<K, n, n> const &matrix,
                                     FieldVector<std::complex<K>, n> &eigenvalues) {
    const long int N = n;
    const char jobvl = 'n';
    const char jobvr = 'n';

    // matrix to put into xgeev
    K matrixVector[n * n];

    // copy matrix
    int row = 0;
    for(int i=0; i<n; ++i)
      for(int j=0; j<n; ++j)
        matrixVector[ row++ ] = matrix[ i ][ j ];

    // working memory
    K eigenR[n];
    K eigenI[n];
    K work[3*n];

    // return value information
    long int info = 0;
    long int lwork = 3*n;

    LapackHelper<K, n>::callXGEEV(&jobvl, &jobvr, &N, &matrixVector[0], &N,
                                  &eigenR[0], &eigenI[0], 0, &N, 0, &N, &work[0],
                                  &lwork, &info);

    if( info != 0 )
    {
      std::cerr << "For matrix " << matrix << " eigenvalue calculation failed! " << std::endl;
      DUNE_THROW(InvalidStateException,"eigenvalues: Eigenvalue calculation failed!");
    }
    for (int i=0; i<N; ++i)
      eigenvalues[i] = { eigenR[i], eigenI[i] };
  }

  void static eigenvaluesRealSymmetric(FieldMatrix<K, n, n> const &matrix,
                                       FieldVector<K, n> &eigenvalues) {
    const long int N = n;
    const char jobz = 'n'; // only calculate eigenvalues
    const char uplo = 'u'; // use upper triangular matrix

    // length of matrix vector, LWORK >= max(1,3*N-1)
    const long int lwork = 3*n -1 ;

    // matrix to put into dsyev
    K matrixVector[n * n];

    // copy matrix
    int row = 0;
    for(int i=0; i<n; ++i)
      for(int j=0; j<n; ++j)
        matrixVector[ row++ ] = matrix[ i ][ j ];

    // working memory
    K workSpace[lwork];

    // return value information
    long int info = 0;

    LapackHelper<K, n>::callXSYEV(&jobz, &uplo, &N, &matrixVector[0], &N,
                                  &eigenvalues[0], &workSpace[0], &lwork, &info);

    if( info != 0 )
    {
      std::cerr << "For matrix " << matrix << " eigenvalue calculation failed! " << std::endl;
      DUNE_THROW(InvalidStateException,"eigenvalues: Eigenvalue calculation failed!");
    }
  }
};
} // end namespace Impl

  // FIXME: Old interface. Consider deprecating.
  namespace FMatrixHelp {

    /** \brief Calculates the eigenvalues of a symmetric 1x1 field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues Eigenvalues in ascending order
    */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 1, 1>& matrix,
                            FieldVector<K, 1>& eigenvalues)
    {
      int const dim = 1;
      Impl::DuneEigenvalueHelper<K, dim>::eigenvaluesRealSymmetric(matrix, eigenvalues);
    }

    /** \brief Calculates the eigenvalues of a symmetric 2x2 field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues Eigenvalues in ascending order
    */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 2, 2>& matrix,
                            FieldVector<K, 2>& eigenvalues)
    {
      int const dim = 2;
      Impl::DuneEigenvalueHelper<K, dim>::eigenvaluesRealSymmetric(matrix, eigenvalues);
    }

    /** \brief Calculates the eigenvalues of a symmetric 3x3 field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues Eigenvalues in ascending order
    */
    template <typename K>
    static void eigenValues(const FieldMatrix<K, 3, 3>& matrix,
                            FieldVector<K, 3>& eigenvalues)
    {
      int const dim = 3;
      Impl::DuneEigenvalueHelper<K, dim>::eigenvaluesRealSymmetric(matrix, eigenvalues);
    }

    /** \brief calculates the eigenvalues of a symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenvalues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::xsyev is used to calculate the eigenvalues
     */
    template <int dim, typename K>
    static void eigenValues(const FieldMatrix<K, dim, dim>& matrix,
                            FieldVector<K, dim>& eigenvalues)
    {
      Impl::LapackEigenvalueHelper<K, dim>::eigenvaluesRealSymmetric(matrix, eigenvalues);
    }
    /** \brief calculates the eigenvalues of a non-symmetric field matrix
        \param[in]  matrix matrix eigenvalues are calculated for
        \param[out] eigenValues FieldVector that contains eigenvalues in
                    ascending order

        \note LAPACK::xgeev is used to calculate the eigen values
     */
    template <int dim, typename K, class C>
    static void eigenValuesNonSym(const FieldMatrix<K, dim, dim>& matrix,
                                  FieldVector<C, dim>& eigenvalues)
    {
      Impl::LapackEigenvalueHelper<K, dim>::eigenvaluesRealGeneric(matrix, eigenvalues);
    }

  }
} // end namespace Dune

#endif
