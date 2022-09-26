// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_FMATRIXEIGENVALUES_CC
#define DUNE_FMATRIXEIGENVALUES_CC

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <cmath>
#include <cassert>

#include <dune/common/exceptions.hh>

#if HAVE_LAPACK

#ifdef LAPACK_NEEDS_UNDERLINE
  #define LAPACK_MANGLE(name,NAME) name##_
#else
  #define LAPACK_MANGLE(name,NAME) name
#endif

#define FC_FUNC LAPACK_MANGLE

// symmetric matrices
#define DSYEV_FORTRAN FC_FUNC (dsyev, DSYEV)
#define SSYEV_FORTRAN FC_FUNC (ssyev, SSYEV)

// nonsymmetric matrices
#define DGEEV_FORTRAN FC_FUNC (dgeev, DGEEV)
#define SGEEV_FORTRAN FC_FUNC (sgeev, SGEEV)

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
  extern void DSYEV_FORTRAN(const char* jobz, const char* uplo, const long
                            int* n, double* a, const long int* lda, double* w,
                            double* work, const long int* lwork, long int* info);
  extern void SSYEV_FORTRAN(const char* jobz, const char* uplo, const long
                            int* n, float* a, const long int* lda, float* w,
                            float* work, const long int* lwork, long int* info);

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

  extern void DGEEV_FORTRAN(const char* jobvl, const char* jobvr, const long
                            int* n, double* a, const long int* lda, double* wr, double* wi, double* vl,
                            const long int* ldvl, double* vr, const long int* ldvr, double* work,
                            const long int* lwork, long int* info);
  extern void SGEEV_FORTRAN(const char* jobvl, const char* jobvr, const long
                            int* n, float* a, const long int* lda, float* wr, float* wi, float* vl,
                            const long int* ldvl, float* vr, const long int* ldvr, float* work,
                            const long int* lwork, long int* info);

} // end extern C

namespace Dune {

  namespace FMatrixHelp {

    void eigenValuesLapackCall(
      const char* jobz, const char* uplo, const long
      int* n, double* a, const long int* lda, double* w,
      double* work, const long int* lwork, long int* info)
    {
      // call LAPACK dsyev
      DSYEV_FORTRAN(jobz, uplo, n, a, lda, w, work, lwork, info);
    }

    void eigenValuesLapackCall(
      const char* jobz, const char* uplo, const long
      int* n, float* a, const long int* lda, float* w,
      float* work, const long int* lwork, long int* info)
    {
      // call LAPACK dsyev
      SSYEV_FORTRAN(jobz, uplo, n, a, lda, w, work, lwork, info);
    }

    void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, double* a, const long int* lda, double* wr, double* wi, double* vl,
      const long int* ldvl, double* vr, const long int* ldvr, double* work,
      const long int* lwork, long int* info)
    {
      // call LAPACK dgeev
      DGEEV_FORTRAN(jobvl, jobvr, n, a, lda, wr, wi, vl, ldvl, vr, ldvr,
                    work, lwork, info);
    }

    void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, float* a, const long int* lda, float* wr, float* wi, float* vl,
      const long int* ldvl, float* vr, const long int* ldvr, float* work,
      const long int* lwork, long int* info)
    {
      // call LAPACK dgeev
      SGEEV_FORTRAN(jobvl, jobvr, n, a, lda, wr, wi, vl, ldvl, vr, ldvr,
                    work, lwork, info);
    }
  } // end namespace FMatrixHelp
} // end namespace Dune

#endif // #if HAVE_LAPACK
#endif // #ifndef DUNE_FMATRIXEIGENVALUES_CC
