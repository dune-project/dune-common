// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FMATRIXEIGENVALUES_EXT_CC
#define DUNE_FMATRIXEIGENVALUES_EXT_CC

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <cmath>
#include <cassert>

#include <dune/common/exceptions.hh>
#include <dune/common/unused.hh>

#if HAVE_LAPACK

// nonsymmetric matrices
#define DGEEV_FORTRAN FC_FUNC (dgeev, DGEEV)

// dsyev declaration (in liblapack)
extern "C" {

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
                            const long int* lwork, const long int* info);

} // end extern C
#endif

namespace Dune {

  namespace DynamicMatrixHelp {

    void eigenValuesNonsymLapackCall(
      const char* jobvl, const char* jobvr, const long
      int* n, double* a, const long int* lda, double* wr, double* wi, double* vl,
      const long int* ldvl, double* vr, const long int* ldvr, double* work,
      const long int* lwork, const long int* info)
    {
#if HAVE_LAPACK
      // call LAPACK dgeev
      DGEEV_FORTRAN(jobvl, jobvr, n, a, lda, wr, wi, vl, ldvl, vr, ldvr,
                    work, lwork, info);
#else
      // silence unused variable warnings
      DUNE_UNUSED_PARAMETER(jobvl), DUNE_UNUSED_PARAMETER(jobvr), DUNE_UNUSED_PARAMETER(n), DUNE_UNUSED_PARAMETER(a);
      DUNE_UNUSED_PARAMETER(lda), DUNE_UNUSED_PARAMETER(wr), DUNE_UNUSED_PARAMETER(wi);
      DUNE_UNUSED_PARAMETER(vl), DUNE_UNUSED_PARAMETER(ldvl), DUNE_UNUSED_PARAMETER(vr), DUNE_UNUSED_PARAMETER(ldvr);
      DUNE_UNUSED_PARAMETER(work), DUNE_UNUSED_PARAMETER(lwork), DUNE_UNUSED_PARAMETER(info);
      DUNE_THROW(NotImplemented,"eigenValuesNonsymLapackCall: LAPACK not found!");
#endif
    }

  } // end namespace FMatrixHelp

} // end namespace Dune
#endif
