// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*																			*/
/* File:	  defaultmmgr.cc				                                                                */
/*																			*/
/* Purpose:   default implementation of memory manager classes				*/
/*																			*/
/* Author:	  Peter Bastian                                                                                 */
/*			  email: peter@ica3.uni-stuttgart.de							*/
/*																			*/
/* History:   26 Feb 1999, begin							                */
/*																			*/
/* Remarks:                                                                                                                             */
/*																			*/
/****************************************************************************/

/* $Header$ */

/****************************************************************************/
/* include files								                                                        */
/****************************************************************************/

#if 0
#include <stdlib.h>
#include <iostream.h>

#include "defaultmmgr.h"
#endif

/****************************************************************************/
/* preprocessor definitions									                                */
/****************************************************************************/

#undef _Debug_

/****************************************************************************/
/* definitions									                                                        */
/****************************************************************************/

/****************************************************************************/
/*																			*/
/* DefaultGHMM									                                                        */
/*																			*/
/****************************************************************************/

DefaultDSMM::DefaultDSMM (unsigned long n_par, int align_par)
{
  // store parameters
  n = n_par;
  align = align_par;
  status = 0;

  // make size a multiple of alignment
  if (n%align!=0) n = (n/align+1)*align;

  // allocate big array, assume that char is one byte
  p = (char *) malloc(n);

  if (p==NULL)
  {
    std::cout << "Could not allocate " << n << "bytes\n";
    status = 1;             // error !
    return;
  }

  base = (unsigned long) p;       // store base address

  //align base
  if (base%align!=0)
  {
    base = (base/align+1)*align;
    n -= align;
  }

  // initialize stacks
  mark_ptr_lo = 0;
  mark_stack_lo[mark_ptr_lo] = 0;

  mark_ptr_hi = 0;
  mark_stack_hi[mark_ptr_hi] = n;

#ifdef _Debug_
  std::cout << "DefaultDSMM: created size: " << n << "\n";
#endif

  return;
}

DefaultDSMM::~DefaultDSMM ()
{
  free(p);       // free all storage
  return;
}

int DefaultDSMM::get_status ()
{
  return status;       // return error variable
}

void *DefaultDSMM::MallocLo (unsigned long n)
{
  unsigned long adr;

  // make n a multiple of alignment
  if (n%align!=0) n=(n/align+1)*align;

  // check if sufficient memory and allocate
  if (mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo]>=n)
  {
    adr = base+mark_stack_lo[mark_ptr_lo];
    mark_stack_lo[mark_ptr_lo] += n;
#ifdef _Debug_
    std::cout << "MallocLo: allocated: " << n << " free: " <<
    mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo] << " pos " << mark_ptr_lo << "\n";
#endif
    return (void *) adr;
  }
  else
  {
    status = 1;
    std::cout << "MallocLo: could not allocate " << n << " bytes\n";
    return NULL;
  }
}

void *DefaultDSMM::MallocHi (unsigned long n)
{
  unsigned long adr;

  // make n a multiple of alignment
  if (n%align!=0) n=(n/align+1)*align;

  // check if sufficient memory and allocate
  if (mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo]>=n)
  {
    mark_stack_hi[mark_ptr_hi] -= n;
    adr = base+mark_stack_hi[mark_ptr_hi];
#ifdef _Debug_
    std::cout << "MallocHi: allocated: " << n << " free: " << (mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo]) << " pos " << mark_ptr_hi << "\n";
#endif
    return (void *) adr;
  }
  else
  {
    status = 1;
    std::cout << "MallocHi: could not allocate " << n << " bytes\n";
    abort();
    return NULL;
  }
}

int DefaultDSMM::MarkLo (int *key)
{
  if (mark_ptr_lo+1<=DEFAULT_MMGR_STACKSIZE)
  {
    mark_ptr_lo++;
    mark_stack_lo[mark_ptr_lo] = mark_stack_lo[mark_ptr_lo-1];
#ifdef _Debug_
    std::cout << "MarkLo: free: " <<
    mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo] << " pos: " << mark_ptr_lo << "\n";
#endif
    *key = mark_ptr_lo;
    return 0;
  }
  else
  {
    status = 1;
    return 1;
  }
}

int DefaultDSMM::MarkHi (int *key)
{
  if (mark_ptr_hi+1<=DEFAULT_MMGR_STACKSIZE)
  {
    mark_ptr_hi++;
    mark_stack_hi[mark_ptr_hi] = mark_stack_hi[mark_ptr_hi-1];
#ifdef _Debug_
    std::cout << "MarkHi: free: " <<
    mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo] << " pos: " << mark_ptr_hi << "\n";
#endif
    *key = mark_ptr_hi;
    return 0;
  }
  else
  {
    status = 1;
    return 1;
  }
}

int DefaultDSMM::ReleaseLo (int key)
{
  /* check key: key and stack ptr must coincide */
  if (mark_ptr_lo != key)
  {
    std::cout << "ReleaseLo: key mismatch\n";
    status = 1;
  }

  if (mark_ptr_lo>0)
  {
    mark_ptr_lo--;
#ifdef _Debug_
    std::cout << "ReleaseLo: free: " <<
    mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo] << " pos: " << mark_ptr_lo << "\n";
#endif
  }
  else status = 1;
  return status;
}

int DefaultDSMM::ReleaseHi (int key)
{
  /* check key: key and stack ptr must coincide */
  if (mark_ptr_hi != key)
  {
    std::cout << "ReleaseHi: key mismatch\n";
    status = 1;
  }

  if (mark_ptr_hi>0)
  {
    mark_ptr_hi--;
#ifdef _Debug_
    std::cout << "ReleaseHi: free: "<<
    mark_stack_hi[mark_ptr_hi]-mark_stack_lo[mark_ptr_lo] << " pos: " << mark_ptr_hi << "\n";
#endif
  }
  else
  {
    status = 1;
  }
  return status;
}

/****************************************************************************/
/*																			*/
/* DefaultGHMM									                                                        */
/*																			*/
/****************************************************************************/


void *DefaultGHMM::Malloc (unsigned long n)
{
  void *p;
  p =  malloc((size_t) n);
  if (p==NULL) std::cout << "Malloc: could not allocate " << n << " bytes\n";
  return p;
}

void DefaultGHMM::Free (void *p)
{
  free(p);
  return;
}
