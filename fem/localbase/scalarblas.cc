// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*																			*/
/* File:	  scalarblas.cc			                                                                                */
/*																			*/
/* Purpose:   implementation of sparse blas routines for scalar matrices        */
/*																			*/
/* Author:	  Peter Bastian                                                                                 */
/*			  email: peter@ica3.uni-stuttgart.de							*/
/*																			*/
/* History:   08 Mar 1999, begin							                */
/*																			*/
/* Remarks:                                                                                                                             */
/*																			*/
/****************************************************************************/

/* $Header$ */

/****************************************************************************/
/* include files								                                                        */
/****************************************************************************/

#if 0

#include <iostream>
#include <cstdio>

#ifdef macintosh
#include <console.h>
#endif

#include "mmgr.hh"
#include "scalarblas.hh"
#endif

/****************************************************************************/
/* preprocessor definitions									                                */
/****************************************************************************/

#ifndef MAX
#define MAX(x,y)                 (((x)>(y)) ? (x) : (y))
#endif
#ifndef ABS
#define ABS(i)                   (((i)<0) ? (-(i)) : (i))
#endif

/****************************************************************************/
/* definitions									                                                        */
/****************************************************************************/

/****************************************************************************/
/*																			*/
/* ScalarSparseBLASManager						                                                        */
/*																			*/
/****************************************************************************/

ScalarSparseBLASManager::ScalarSparseBLASManager (DoubleStackMemoryManager *dsmm,
                                                  GeneralHeapMemoryManager *ghmm,
                                                  int nv, int nm)
{
  _dsmm = dsmm;
  _ghmm = ghmm;
  _nv = nv;       // number of vectors to be allocated at a time
  _nm = nm;

  // init arrays storing allocated vectors
  builtVecCtr = 0;
  builtVecRef = (ScalarVector **) ghmm->Malloc(nv*sizeof(ScalarVector *));

  // init arrays storing allocated matrices
  builtMatCtr = 0;
  builtMatRef = (ScalarMatrix **) ghmm->Malloc(nm*sizeof(ScalarMatrix *));
}

ScalarSparseBLASManager::~ScalarSparseBLASManager ()
{
  if (builtVecCtr>0)
    std::cout << "~ScalarSparseBLASManager: " << builtVecCtr << " vectors are still allocated\n";

  if (builtMatCtr>0)
    std::cout << "~ScalarSparseBLASManager: " << builtMatCtr << " matrices are still allocated\n";

  _ghmm->Free(builtVecRef);
  _ghmm->Free(builtMatRef);
}

int ScalarSparseBLASManager::PushVector (ScalarVector *v)
{
  if (builtVecCtr+1<=_nv)
  {
    builtVecRef[builtVecCtr] = v;
    builtVecCtr++;
    return 0;
  }
  else return 1;
}

int ScalarSparseBLASManager::PopVector (ScalarVector *v)
{
  if (builtVecCtr>0)
  {
    builtVecCtr--;
    if (builtVecRef[builtVecCtr]!=v) return 1;
    return 0;
  }
  else return 1;
}

int ScalarSparseBLASManager::PushMatrix (ScalarMatrix *m)
{
  if (builtMatCtr+1<=_nm)
  {
    builtMatRef[builtMatCtr] = m;
    builtMatCtr++;
    return 0;
  }
  else return 1;
}

int ScalarSparseBLASManager::PopMatrix (ScalarMatrix *m)
{
  if (builtMatCtr>0)
  {
    builtMatCtr--;
    if (builtMatRef[builtMatCtr]!=m) return 1;
    return 0;
  }
  else return 1;
}

int ScalarSparseBLASManager::Resize ()
{
  return 0;
}

void ScalarSparseBLASManager::vset (ScalarVector *x, double val)
{
  register double *v;
  register int i;

  v = x->v;
  for (i=0; i<x->n; i++)
    *v++ = val;

  return;
}

double ScalarSparseBLASManager::ddot (ScalarVector *x, ScalarVector *y)
{
  double *vx, *vy;
  int i;
  double s=0.0;

  if ((x->n != y->n) || (x->_ssbm)!=(y->_ssbm)) return 0.0;

  vx = x->v;
  vy = y->v;

  for (i=0; i<x->n; i++)
    s += (*vx++) * (*vy++);

  return s;
}

double ScalarSparseBLASManager::infnorm (ScalarVector *x)
{
  double *vx;
  int i;
  double m=-1,value;

  vx = x->v;

  for (i=0; i<x->n; i++)
  {
    value = (*vx++);
    if (value<0) value=-value;
    m = MAX(m,value);
  }

  return m;
}

void ScalarSparseBLASManager::vcopy (ScalarVector *x, ScalarVector *y)
{
  register double *vx, *vy;
  register int i;

  // check compatibility
  if ((x->n != y->n) || (x->_ssbm)!=(y->_ssbm)) return;

  vx = x->v;
  vy = y->v;

  for (i=0; i<x->n; i++)
    *vx++ =  *vy++;

  return;
}

void ScalarSparseBLASManager::vscale (ScalarVector *x, double val)
{
  register double *v;
  register int i;

  v = x->v;

  for (i=0; i<x->n; i++)
    *v++ *= val;

  return;
}

void ScalarSparseBLASManager::daxpy (ScalarVector *x, ScalarVector *y, double val)
{
  register double *vx, *vy;
  register double *vx0, *vy0;
  register double rv=val;
  register int i;
  int g,r;

  // check compatibility
  if ((x->n != y->n) || (x->_ssbm)!=(y->_ssbm)) return;

  vx = x->v;
  vy = y->v;
  g = x->n / 4;
  r = x->n % 4;

  for (i=0; i<r; i++)
    *vx++ += rv * (*vy++);

  if (g>0) {
    vx0= vx; vy0=vy;
    *vx0 += rv * (*vy0);
    vx++; vy++;
    vx0+=4; vy0+=4;

    for (i=1; i<g; i++)
    {
      *vx0 += rv * (*vy0);
      vx0+=4; vy0+=4;
      *vx++ += rv * (*vy++);
      *vx++ += rv * (*vy++);
      *vx++ += rv * (*vy++);
      vx++; vy++;
    }
    *vx++ += rv * (*vy++);
    *vx++ += rv * (*vy++);
    *vx++ += rv * (*vy++);
  }

  return;
}

void ScalarSparseBLASManager::mset (ScalarMatrix *A, double val)
{
  int i;
  double *a=A->a;

  for (i=0; i<A->ctr; i++) a[i] = val;

  return;
}

void ScalarSparseBLASManager::mcopy (ScalarMatrix *A, ScalarMatrix *B)
{
  int i;
  double *a=A->a;
  double *b=B->a;

  // assume identical structure !
  for (i=0; i<A->ctr; i++) a[i] = b[i];

  return;
}

void ScalarSparseBLASManager::matmul (ScalarVector *x, ScalarMatrix *A, ScalarVector *y)
{
  int i,k,nr=A->nr;
  int *r=A->r,*s=A->s,*j=A->j;
  const double *a=(const double *) A->a;
  double *xv=x->v;
  const double *yv=(const double *)y->v;
  double sum;

  if (x->n != A->nr) return;
  if (y->n != A->nc) return;

  for (i=0; i<nr; i++)
  {
    sum = 0;
    for (k=r[i]; k<r[i]+s[i]; k++) sum += a[k]*yv[j[k]];
    xv[i] = sum;
  }

  return;
}

void ScalarSparseBLASManager::matmulplus (ScalarVector *x, double val, ScalarMatrix *A, ScalarVector *y)
{
  int i,k,nr=A->nr;
  int *r=A->r,*s=A->s,*j=A->j;
  const double *a=(const double *) A->a;
  double *xv=x->v;
  const double *yv=(const double *)y->v;
  double sum;

  if (x->n != A->nr) return;
  if (y->n != A->nc) return;

  for (i=0; i<nr; i++)
  {
    sum = 0;
    for (k=r[i]; k<r[i]+s[i]; k++) sum += a[k]*yv[j[k]];
    xv[i] += val*sum;
  }

  return;
}

void ScalarSparseBLASManager::matmulplustranspose (ScalarVector *x, double val, ScalarMatrix *A, ScalarVector *y)
{
  int i,k,nr=A->nr;
  int *r=A->r,*s=A->s,*j=A->j;
  const double *a=(const double *) A->a;
  double *xv=x->v;
  const double *yv=(const double *)y->v;

  if (x->n != A->nc) return;
  if (y->n != A->nr) return;

  for (i=0; i<nr; i++)
    for (k=r[i]; k<r[i]+s[i]; k++)
      xv[j[k]] += val*a[k]*yv[i];

  return;
}

void ScalarSparseBLASManager::jac (int nit, ScalarMatrix *A, ScalarVector *x, ScalarVector *b, double damp)
{
  int i,k,nr=A->nr;
  int *r=A->r,*s=A->s,*j=A->j;
  const double *a=(const double *) A->a;
  double *xv=x->v;
  const double *bv=(const double *)b->v;
  double sum;

  if (x->n != A->nr) {std::cout<<"jac: incompatible operands\n"; return;}
  if (x->n != A->nc) {std::cout<<"jac: incompatible operands\n"; return;}
  if (x->n != b->n)  {std::cout<<"jac: incompatible operands\n"; return;}

  ScalarVector c(this);
  if (c.Clone(x)) {std::cout<<"jac: not enough memory\n"; return;}
  double *cv=c.v;

  for (int n=0; n<nit; n++)
  {
    for (i=0; i<nr; i++)
    {
      sum = bv[i];
      for (k=r[i]; k<r[i]+s[i]; k++) sum -= a[k]*xv[j[k]];
      cv[i] = sum/a[r[i]];
    }
    daxpy(x,&c,damp);
  }

  c.Free();

  return;
}

void ScalarSparseBLASManager::sor (int nit, ScalarMatrix *A, ScalarVector *x, ScalarVector *b, double damp)
{
  int i,k,nr=A->nr;
  int *r=A->r,*s=A->s,*j=A->j;
  const double *a=(const double *) A->a;
  double *xv=x->v;
  const double *bv=(const double *)b->v;
  double sum;

  if (x->n != A->nr) return;
  if (x->n != A->nc) return;
  if (x->n != b->n) return;

  for (int n=0; n<nit; n++)
  {
    for (i=0; i<nr; i++)
    {
      sum = bv[i];
      for (k=r[i]+1; k<r[i]+s[i]; k++) sum -= a[k]*xv[j[k]];
      xv[i] = (1-damp)*xv[i] + damp*sum/a[r[i]];
    }
  }

  return;
}

void ScalarSparseBLASManager::ssor (int nit, ScalarMatrix *A, ScalarVector *x, ScalarVector *b, double damp)
{
  int i,k,nr=A->nr;
  int *r=A->r,*s=A->s,*j=A->j;
  const double *a=(const double *) A->a;
  double *xv=x->v;
  const double *bv=(const double *)b->v;
  double sum;

  if (x->n != A->nr) return;
  if (x->n != A->nc) return;
  if (x->n != b->n) return;

  for (int n=0; n<nit; n++)
  {
    for (i=0; i<nr; i++)
    {
      sum = bv[i];
      for (k=r[i]+1; k<r[i]+s[i]; k++) sum -= a[k]*xv[j[k]];
      xv[i] = (1-damp)*xv[i] + damp*sum/a[r[i]];
    }
    for (i=nr-1; i>=0; i--)
    {
      sum = bv[i];
      for (k=r[i]+1; k<r[i]+s[i]; k++) sum -= a[k]*xv[j[k]];
      xv[i] = (1-damp)*xv[i] + damp*sum/a[r[i]];
    }
  }

  return;
}

/****************************************************************************/
/*																			*/
/* ScalarVector						                                                                    */
/*																			*/
/****************************************************************************/

int ScalarVector::Build (int nn, int ntotal)
{
  // number of blocks and total size must be identical
  if (nn!=ntotal) return 1;

  // vector must not be allocated yet
  if (built) return 1;

  // remember size
  n = ntotal;

  // allocate memory
  if (_dsmm->MarkHi(&key)) return 1;
  v = (double *) _dsmm->MallocHi(n*sizeof(double));
  if (v==NULL) return 1;

  // register vector with manager
  if (_ssbm->PushVector(this)) return 1;

  // set built flag
  built = true;

  return 0;
}

int ScalarVector::Clone (ScalarVector *x)
{
  // vector must not be allocated yet
  if (built) return 1;

  // remember size
  n = x->n;

  // allocate memory
  if (_dsmm->MarkHi(&key)) return 1;
  v = (double *) _dsmm->MallocHi(n*sizeof(double));
  if (v==NULL) return 1;

  // register vector with manager
  if (_ssbm->PushVector(this)) return 1;

  // set built flag
  built = true;

  return 0;
}

int ScalarVector::Resize (int nn, int ntotal)
{
  std::cout << "Resize: not implemented yet\n";
  return 0;
}

int ScalarVector::Free ()
{
  // Deregister with manager
  if (_ssbm->PopVector(this))
  {
    std::cout << "ScalarVector::Free: pop order mismatch\n";
    return 1;
  }

  // Release and check key
  if (_dsmm->ReleaseHi(key))
  {
    std::cout << "ScalarVector::Free: key mismatch\n";
    return 1;
  }

  // no errors
  built = false;
  return 0;
}

ScalarVector::ScalarVector (ScalarSparseBLASManager *ssbm)
{
  _dsmm = ssbm->mydsmm();
  _ssbm = ssbm;
  built = false;       // no memory allocated
}

void ScalarVector::print (char *s, int c)
{
  for (int i=0; i<n; i+=c)
  {
    printf("%s",s);
    for (int j=i; j<i+c; j++)
    {
      if (j>=n) break;
      printf("[%3d:%12.4g] ",j,v[j]);
    }
    printf("\n");
  }
  return;
}

/****************************************************************************/
/*																			*/
/* ScalarMatrix						                                                                    */
/*																			*/
/****************************************************************************/

int ScalarMatrix::Build (int nnr, int nnc, int nnz)
{
  // save parameters
  nr = nnr;
  nc = nnc;
  nz = nnz;

  // mark memory stack
  if (_dsmm->MarkLo(&key)) return 1;

  // allocate space for row access
  r = (int *) _dsmm->MallocLo(nr*sizeof(int));
  if (r==NULL) return 1;
  for (int i=0; i<nr; i++) r[i] = -1;

  // allocate space for size array
  s = (int *) _dsmm->MallocLo(nr*sizeof(int));
  if (s==NULL) return 1;
  for (int i=0; i<nr; i++) s[i] = 0;

  // allocate space for size array
  columns = (int *) _dsmm->MallocLo(nz*sizeof(int));
  if (columns==NULL) return 1;
  for (int i=0; i<nz; i++) columns[i] = -1;

  // allocate space for nonzeros
  a = (double *) _dsmm->MallocLo(nz*sizeof(double));
  if (a==NULL) return 1;

  // allocate space for column array
  j = (int *) _dsmm->MallocLo(nz*sizeof(int));
  if (j==NULL) return 1;

  // register vector with manager
  if (_ssbm->PushMatrix(this)) return 1;

  // set built flag
  built = true;
  ctr = 0;

  return 0;
}

int ScalarMatrix::Clone (ScalarMatrix *B)
{
  // store parameters
  nr = B->nr;
  nc = B->nc;
  nz = B->nz;
  ctr = B->ctr;

  // mark memory stack
  if (_dsmm->MarkLo(&key)) return 1;

  // reuse index arrays from _B
  // due to stack-based memory management _B will be
  // released later !
  r = B->r;
  s = B->s;
  j = B->j;

  // allocate space for nonzeros
  a = (double *) _dsmm->MallocLo(nz*sizeof(double));
  if (a==NULL) return 1;

  // register vector with manager
  if (_ssbm->PushMatrix(this)) return 1;

  // set built flag
  built = true;

  return 0;
}

int ScalarMatrix::Free ()
{
  // Deregister with manager
  if (_ssbm->PopMatrix(this))
  {
    std::cout << "ScalarMatrix::Free: pop order mismatch\n";
    return 1;
  }

  // Release and check key
  if (_dsmm->ReleaseLo(key))
  {
    std::cout << "ScalarMatrix::Free: key mismatch\n";
    return 1;
  }

  // no errors
  built = false;
  return 0;
}

ScalarMatrix::ScalarMatrix (ScalarSparseBLASManager *ssbm)
{
  _dsmm = ssbm->mydsmm();
  _ssbm = ssbm;
  built = false;
}

void ScalarMatrix::print (char *ss)
{
  for (int i=0; i<nr; i++)
  {
    printf("%s",ss);
    for (int k=r[i]; k<r[i]+s[i]; k++)
      printf("[%3d:%3d/%12.4g]",i,j[k],a[k]);
    printf("\n");
  }

  return;
}
