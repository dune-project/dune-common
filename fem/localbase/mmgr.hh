// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*																			*/
/* File:      mmgr.h	                                                                                                        */
/*																			*/
/* Purpose:   memory manager classes needed by sumo                                     */
/*																			*/
/* Author:    Peter Bastian                                                                             */
/*  email:    peter@ica3.uni-stuttgart.de									*/
/*																			*/
/* History:   24 Feb 1999, begin                                                                        */
/*																			*/
/* Remarks:                                                                                                                             */
/*																			*/
/****************************************************************************/

/* $Header$ */

/****************************************************************************/
/* auto include mechanism and other include files							*/
/****************************************************************************/

#ifndef __MEMORYMGR__
#define __MEMORYMGR__

/****************************************************************************/
/* preprocessor definitions                                                                                             */
/****************************************************************************/

/****************************************************************************/
/* class definitions                                                                    */
/****************************************************************************/


/* @LATEXON
   \clearpage
   \section{Speicherverwaltung}\label{Section:Speicherverwaltung}

   Speicherverwaltung ist ein wichtiges Thema f"ur effizienten Code in der Numerik. OO Puristen
   w"urden die Speicherverwaltung am liebsten vollautomatisch in Form einer
   \glqq{}garbage collection\grqq realisieren, siehe \cite{Meyer97}. In C++ wird der
   Speicher traditionell vom Programmierer verwaltet. Hierzu stellen wir hier einige
   Klassen vor.

   \subsection{Klasse DoubleStackMemoryManager}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   abstrakt, lokal

   \subsubsection*{Klassenbaum}

   Basisklasse

   \subsection*{Assozierte Klassen}

   Keine

   \subsubsection*{Beschreibung}

   \begin{figure}
   \centerline{\epsfig{figure=EPS/dsmm.eps}}
   \caption{Klassendiagramm f"ur den DoubleStackMemoryMangaer}
   \label{Abb:DSMM}
   \end{figure}

   DoubleStackMemoryManager realisiert folgende Art der Speicherverwaltung:

   \begin{itemize}
   \item Zu Beginn besitzt der DoubleStackMemoryManager einen zusammenh"angenden Speicherblock
   \item Speicherbl"ocke k"onnen jeweils vom oberen oder unteren Ende abgeknappst werden
   \item Die aktuelle Position des freien Speichers an beiden Enden kann markiert werden
   \item Speicher kann bis zur letzten Markierung freigegeben werden
   \item Markierungen k"onnen geschachtelt werden
   \end{itemize}

   Abbildung \ref{Abb:DSMM} zeigt ein Klassendiagram, die Methoden sind nachfolgend erl"autert.

   \subsubsection*{C++ Implementierung}

   @LATEXOFF @VERBON*/
class DoubleStackMemoryManager { // lets you allocate memory from hi and lo
                                 // end, mark and release are recursive
public:
  // functions for the lo end used for temporary objects
  virtual void *MallocLo  (unsigned long n) = 0;
  virtual int   MarkLo    (int *key) = 0;   // return 1 if error
  virtual int   ReleaseLo (int key) = 0;    // return 1 if error

  // functions for the hi end used for dynamic objects
  virtual void *MallocHi  (unsigned long n) = 0;
  virtual int   MarkHi    (int *key) = 0;   // etc.
  virtual int   ReleaseHi (int key) = 0;    // etc.
} ;
/* @VERBOFF */

/* @LATEXON
   \subsubsection*{Methoden}

   \begin{hangpar}
   MallocLo($n$) allokiert $n$ bytes vom unteren Ende des Speichers.
   \end{hangpar}

   \begin{hangpar}
   MarkLo($key$) Schreibt die aktuelle Position des freien Speichers am unteren Ende
   auf einen Stack (push). $key$ wird mit der Stackposition gef"ullt (so kann man
   beim freigeben "uberpr"ufen ob kein Release vergessen wurde.
   \end{hangpar}

   \begin{hangpar}
   ReleaseLo($key$) holt die Position des freien Speichers am unteren Ende vom Stack. In $key$
   "ubergibt man den Wert, den das zugeh"orige MarkLo geliefert hat. Dies dient der
   "Uberpr"ufung der richtigen Schachtelung der MarkLo/ReleaseLo Aufrufe.
   \end{hangpar}

   Mit der Endung \glqq{}Hi\grqq{} sind die entsprechenden Aufrufe f"ur das obere Ende
   bezeichnet.
   @LATEXOFF */


/* @LATEXON
   \subsection{Klasse GeneralHeapMemoryManager}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   abstrakt, lokal

   \subsubsection*{Klassenbaum}

   Basisklasse

   \subsection*{Assozierte Klassen}

   Keine

   \subsubsection*{Beschreibung}

   \begin{figure}
   \begin{center}
   \centerline{\epsfig{figure=EPS/ghmm.eps}}
   \end{center}
   \caption{Klassendiagramm f"ur den GeneralHeapMemoryManager}
   \label{Abb:GHMM}
   \end{figure}

   Der GeneralHeapMemoryManager verpackt die "ublichen Speicherverwaltungsfunktionen
   malloc/free von C in eine Klasse. Der Sinn ist, dass man damit die Speicherverwaltung
   besser in ein bereits bestehendes Programm integrieren kann.

   Abbildung \ref{Abb:GHMM} zeigt das Klassendiagramm.

   \subsubsection*{C++ Implementierung}

   @LATEXOFF @VERBON*/
class GeneralHeapMemoryManager { // allows standard malloc and free

public:
  virtual void *Malloc   (unsigned long n) = 0;
  virtual void  Free     (void *p) = 0;
} ;
/* @VERBOFF */
/* @LATEXON
   \subsubsection*{Methoden}

   \begin{hangpar}
   Malloc($n$) allokiert $n$ bytes.
   \end{hangpar}

   \begin{hangpar}
   Free($p$) gibt den Speicherblock mit Anfang $p$ wieder frei.
   \end{hangpar}

   @LATEXOFF */


#endif
