// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*																			*/
/* File:      defaultmmgr.h                                                                                             */
/*																			*/
/* Purpose:   default implementation of memory manager classes				*/
/*																			*/
/* Author:    Peter Bastian                                                                             */
/*  email:    peter@ica3.uni-stuttgart.de									*/
/*																			*/
/* History:   26 Feb 1999, begin                                                                        */
/*																			*/
/* Remarks:                                                                                                                             */
/*																			*/
/****************************************************************************/

/* $Header$ */

/****************************************************************************/
/* auto include mechanism and other include files							*/
/****************************************************************************/

#ifndef __DEFAULTMMGR__
#define __DEFAULTMMGR__

#include <stdlib.h>
#include <iostream.h>

#include "mmgr.hh"

/****************************************************************************/
/* preprocessor definitions                                                                                             */
/****************************************************************************/

#define DEFAULT_MMGR_STACKSIZE  512

/****************************************************************************/
/* class definitions                                                                    */
/****************************************************************************/

/* @LATEXON
   \subsection{Klasse DefaultDSMM}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   konkret, lokal

   \subsubsection*{Klassenbaum}

   DefaultDSMM $\to$ DoubleStackMemoryManager

   \subsection*{Assozierte Klassen}

   Keine

   \subsubsection*{Beschreibung}

   DefaultDSMM implementiert das Interface von DoubleStackMemoryManager.

   \subsubsection*{C++ Implementierung}

   @LATEXOFF @VERBON*/
class DefaultDSMM : public DoubleStackMemoryManager {
public:
  // constructor/destructor
  DefaultDSMM (unsigned long n_par, int align_par);
  ~DefaultDSMM();
  int get_status();

  // functions for the lo end used for temporary objects
  void *MallocLo  (unsigned long n);
  int   MarkLo    (int *key);
  int   ReleaseLo (int key);

  // functions for the hi end used for dynamic objects
  void *MallocHi  (unsigned long n);
  int   MarkHi    (int *key);
  int   ReleaseHi (int key);

private:
  char *p;
  unsigned long base;
  unsigned long n;
  int align;
  int status;

  unsigned long mark_stack_hi[DEFAULT_MMGR_STACKSIZE];
  int mark_ptr_hi;

  unsigned long mark_stack_lo[DEFAULT_MMGR_STACKSIZE];
  int mark_ptr_lo;
} ;
/* @VERBOFF */

/* @LATEXON
   \subsubsection*{Konstruktor}

   \begin{hangpar}
   DefaultDSMM($n\_par$,$align\_par$) allokiert $n\_par$ bytes mit malloc, die dann als
   freier Speicher zur Verf"ugung stehen. Alle Speicheranforderungen mit MallocLo/MallocHi
   liefern Speicheradressen, die ein Vielfaches von $align\_par$ sind.
   \end{hangpar}

   \subsubsection*{Destruktor}

   \begin{hangpar}
   $\sim$DefaultDSMM() gibt den Speicherblock der vom Konstruktor allokiert wurde wieder frei.
   \end{hangpar}

   @LATEXOFF */


/* @LATEXON
   \subsection{Klasse DefaultGHMM}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   konkret, lokal

   \subsubsection*{Klassenbaum}

   DefaultGHMM $\to$ GeneralHeapMemoryManager

   \subsection*{Assozierte Klassen}

   Keine

   \subsubsection*{Beschreibung}

   DefaultGHMM implementiert das Interface von GeneralHeapMemoryManager.

   \subsubsection*{C++ Implementierung}

   @LATEXOFF @VERBON*/
class DefaultGHMM : public GeneralHeapMemoryManager {

public:
  void *Malloc   (unsigned long n);
  void  Free     (void *p);
} ;
/* @VERBOFF */

#include "defaultmmgr.cc"


#endif
