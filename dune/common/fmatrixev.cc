// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
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

// symetric matrices
#define DSYEV_FORTRAN FC_FUNC (dsyev, DSYEV)

// nonsymetric matrices
#define DGEEV_FORTRAN FC_FUNC (dgeev, DGEEV)

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
  extern void DSYEV_FORTRAN(const char* jobz, const char* uplo, const long
                            int* n, double* a, const long int* lda, double* w,
                            double* work, const long int* lwork, long int* info);

} // end extern C
#endif

namespace Dune {

  namespace FMatrixHelp {

    void eigenValuesLapackCall(
      const char* jobz, const char* uplo, const long
      int* n, double* a, const long int* lda, double* w,
      double* work, const long int* lwork, long int* info)
    {
#if HAVE_LAPACK
      // call LAPACK dsyev
      DSYEV_FORTRAN(jobz, uplo, n, a, lda, w, work, lwork, info);
#else
      DUNE_THROW(NotImplemented,"eigenValuesLapackCall: LAPACK not found!");
#endif
    }

  } // end namespace FMatrixHelp

} // end namespace Dune
#endif
