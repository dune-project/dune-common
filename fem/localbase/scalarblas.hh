// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/****************************************************************************/
/*																			*/
/* File:	  scalarblas.h                                                                                                  */
/*																			*/
/* Purpose:   implements sblas.h interface for scalar matrices                          */
/*            in compressed row storage format                              */
/*																			*/
/* Author:	  Peter Bastian                                                                                 */
/*			  email: peter@ica3.uni-stuttgart.de							*/
/*																			*/
/* History:   01 Mar 1999, begin							                */
/*            27 Apr 1999, eliminated virtual base class                    */
/*																			*/
/* Remarks:                                                                                                                             */
/*																			*/
/****************************************************************************/

/* $Header$ */

/****************************************************************************/
/* auto include mechanism and other include files							*/
/****************************************************************************/

#ifndef __SCALARBLAS__
#define __SCALARBLAS__


#include <iostream>
#include <cstdio>

#include "mmgr.hh"


/****************************************************************************/
/* preprocessor definitions									                                */
/****************************************************************************/

/****************************************************************************/
/* class definitions				                                        */
/****************************************************************************/

/* @LATEXON
   \clearpage
   \section{Lineare Algebra f\"ur d"unnbesetzte Matrizen}\label{Section:LineareAlgebra}

   Dieser Abschnitt beschreibt die drei Klassen ScalarVector, ScalarMatrix und
   ScalarSparseBLASManager zur Speicherung und Verarbeitung d"unnbesetzter
   Matrizen und Vektoren.

   Obwohl die hier beschriebenen Klassen nur skalare Elemente besitzen implementieren
   sie das Interface f"ur allgemeine blockstrukturierte Matrizen und Vektoren.
   Durch Parametrisierung kann man so z.~B.~ein CG--Verfahren sowohl f"ur
   skalare Gleichungen als auch f"ur Systeme von Gleichungen verwenden.

   \subsection{Klasse {\rm ScalarVector}}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   konkret

   \subsubsection*{Klassenbaum}

   Basisklasse

   \subsection*{Assozierte Klassen}

   DoubleStackMemoryManager, ScalarSparseBLASManager

   \subsubsection*{Beschreibung}

   \begin{figure}
   \centerline{\epsfig{figure=EPS/blas.eps,angle=90}}
   \caption{Klassendiagramm f"ur die Klassen im Zusammenhang mit BLAS.}
   \label{Abb:BLAS}
   \end{figure}

   Die Klasse ScalarVector implementiert das Interface einer allgemeineren
   Klasse zur Darstellung von blockstrukturierten Vektoren. Deshalb seien
   zun\"achst diese erl\"autert.

   Sei $x\in\MR^m$ ein Vektor zur
   Indexmenge $I$, $|I|=m$. Eine Blockstruktur auf dem Vektor ist gegeben
   durch eine (nicht\"uberlappende) Zerlegung der Indexmenge:

   \[
   I = \bigcup_{\kappa\in B} J_\kappa, J_\kappa\cap J_\eta=\emptyset,
   \kappa\neq\eta.
   \]

   Die Anzahl der Bl\"ocke ist $n=|B|$, die Gr\"o"se des Blockes $\kappa$
   ist $m_\kappa=|J_\kappa|$ und es gilt offensichtlich $\sum_{\kappa\in
   B} m_\kappa = m$.

   Wir bezeichnen mit $(x)_\kappa\in \MR^{m_\kappa}$ den Block $\kappa$
   von $x$. Eine einzelne Komponente $i$ von $(x)_\kappa$ erh\"alt man mittels $\left
   ( (x)_\kappa \right )_i$.

   Diverse Implementierungen von blockstrukturierten Vektoren unterscheiden sich hinsichtlich
   der Flexibilit\"at in der Blockgr\"o"se. Zumindest drei Varianten sind
   denkbar: $m_\kappa=1$ f\"ur alle $\kappa$, $m_\kappa=s$, $s\in\MN$
   f\"ur alle $\kappa$ und beliebiges  $m_\kappa$ in jedem Block.

   Die Klasse ScalarVector implementiert die Variante mit $m_\kappa=1$.

   @LATEXOFF */

class ScalarSparseBLASManager; // forward declaration

/* @LATEXON

   \subsubsection*{C++ Implementierung}

   @LATEXOFF @VERBON */
class ScalarVector
{
public:
  // allocate/deallocating space
  int Build (int n, int m);
  int Clone (ScalarVector *x);
  int Resize (int n, int m);
  int Free ();

  double* getVec() { return v; };
  double& operator [] (int i) { return v[i]; };

  // define structure/fill values
  int Assemble (int index, int size);
  int Reassemble (int index, int size);

  // inlined access functions ...
  void Put (int index, double *vals);
  void Add (int index, double *vals);
  void Mul (int index, double *vals);
  void Get (int index, double *vals);
  int BlockSize (int index);
  int Size ();

  // debug
  void print (char *s, int c);

  // constructor
  ScalarVector (ScalarSparseBLASManager *ssbm);

  friend class ScalarSparseBLASManager;

private:
  int n;           // number of doubles
  double *v;       // array of values
  bool built;      // true if space has been allocated
  int key;         // key from memory manager
  DoubleStackMemoryManager *_dsmm;       // my memory manager
  ScalarSparseBLASManager *_ssbm;       // my blas manager
};

/* @VERBOFF */

/* @LATEXON

   \subsubsection*{Konstruktoren}

   \begin{hangpar}
   ScalarVector erh\"alt als Parameter ein Objekt der Klasse ScalarSparseBLASManager.
   Dieses enth\"alt die Speicherverwaltung. Der Konstruktor erzeugt nur ein Objekt der
   Klasse ScalarVector, das im gewissen Sinne \glqq{}leer\grqq{} ist und noch keine
   Daten f\"ur den Vektor enth\"alt. Der Vektor selbst wird erst mit der Funktion Build
   angelegt.
   \end{hangpar}

   \subsubsection*{Methoden}

   \begin{hangpar}
   Build($n$,$m$) allokiert Platz f\"ur einen Vektor $x\in\MR^m$ mit $n$ Bl"ocken.
   F"ur ScalarVector ist $n=m$ zwingend. Build reserviert nur den Platz f"ur den
   Vektor, die einzelnen Komponenten sind noch undefiniert. Der Vektor wird durch ein
   Feld von $m$ double Werten implementiert.
   \end{hangpar}

   \begin{hangpar}
   Clone($x$) erzeugt Platz f"ur einen Vektor mit der selben Gr"o"se wie $x$. Der
   Inhalt des Vektors $x$ wird {\em nicht} kopiert (dies macht eine BLAS--Routine).
   \end{hangpar}

   \begin{hangpar}
   Resize($n$,$m$) wird im Moment noch nicht unterst"utzt.
   \end{hangpar}

   \begin{hangpar}
   Free() gibt den Platz f"ur den Vektor wieder frei. Mit der Build Funktion
   kann dann ein neuer Vektor angelegt werden.
   \end{hangpar}

   \begin{hangpar}
   Assemble($\kappa$,$m_\kappa$) legt die Gr"o"se des Blockes $\kappa$ fest. Diese
   Funktion macht im Falle von ScalarVector nichts ist aber f"ur allgemeine blockstrukturierte
   Vektoren notwendig.
   \end{hangpar}

   \begin{hangpar}
   Reassemble($\kappa$,$m_\kappa$) wird noch nicht unterst"utzt.
   \end{hangpar}

   \begin{hangpar}
   Put($\kappa$,$y\in\MR^{m_\kappa}$) ersetzt den Block $\kappa$ des Vektors durch $y$.
   \end{hangpar}

   \begin{hangpar}
   Add($\kappa$,$y\in\MR^{m_\kappa}$) addiert $y$ auf den Block $\kappa$ des Vektors.
   \end{hangpar}

   \begin{hangpar}
   Get($\kappa$,$y\in\MR^{m_\kappa}$) gibt den Block $\kappa$ des Vektors in $y$ zur"uck.
   \end{hangpar}

   \begin{hangpar}
   Blocksize($\kappa$) gibt $m_\kappa$ zur"uck.
   \end{hangpar}

   \begin{hangpar}
   Size($\kappa$) gibt $m$ zur"uck.
   \end{hangpar}

   \begin{bemerkung}
   Die Funktionen Assemble, Put, Add, Get, BlockSize und Size sind als inline--Funktionen
   ausgef"uhrt.
   \end{bemerkung}

   @LATEXOFF */

inline int ScalarVector::Assemble (int index, int size)
{
  if (size!=1) return 1;
  return 0;
}

inline int ScalarVector::Reassemble (int index, int size)
{
  std::cout << "Reassemble: not implemented yet\n";
  return 0;
}

inline void ScalarVector::Put (int index, double *vals)
{
  v[index] = *vals;
}

inline void ScalarVector::Add (int index, double *vals)
{
  v[index] += *vals;
}

inline void ScalarVector::Mul (int index, double *vals)
{
  v[index] *= *vals;
}

inline void ScalarVector::Get (int index, double *vals)
{
  *vals = v[index];
}

inline int ScalarVector::Size ()
{
  return n;
}

inline int ScalarVector::BlockSize (int index)
{
  return 1;
}

/* @LATEXON

   \subsection{Klasse {\rm ScalarMatrix}}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   konkret

   \subsubsection*{Klassenbaum}

   Basisklasse

   \subsection*{Assozierte Klassen}

   DoubleStackMemoryManager, ScalarSparseBLASManager

   \subsubsection*{Beschreibung}

   Die Klasse ScalarMatrix implementiert das Interface einer allgemeineren Klasse zur
   Darstellung von blockstrukturierten, d"unnbesetzten Matrizen. Deshalb seien zun"achst
   diese erl"autert.

   Es sei $A\in L(\MR^{m^c},\MR^{m^r})$, also eine Matrix mit $m^r$ Zeilen und $m^c$ Spalten.
   Es seien $I^r$, $I^c$ die Indexmengen zu den Zeilen bzw.~Spalten. Somit gilt
   $m^r=|I^r|$ und $m^c=|I^c|$. Auf $I^r$ und $I^c$ sei jeweils eine Blockzerlegung
   definiert, d.~h.~:
   %
   \[
   I^r = \bigcup_{\kappa\in B^r} J^r_\kappa, \ \ \ I^c = \bigcup_{\eta\in B^c} J^c_\eta.
   \]
   %
   Die Gr"o"se der Bl"ocke sei $m_\kappa^r = |J_\kappa^r|$, $m_\eta^c = |J_\eta^c|$ und
   die Anzahl der Bl"ocke ist $n^r=|B^r|$, $n^c=|B^c|$.
   Entsprechend ist dann $(A)_{\kappa\eta}$ eine $m_\kappa^r \times m_\eta^c$ Matrix.
   Ein Element eines Blockes wird mittels $\left ( (A)_{\kappa\eta} \right )_{ij}$
   adressiert.
   Ist die Matrix $A$ d"unnbesetzt so k"onnen sowohl ganze Bl"ocke 0 sein als auch
   einzelne Eintr"age innerhalb der Bl"ocke.

   Diverse Implementierungen unterscheiden sich hinsichtlich der Flexibilit"at
   in den Bl"ockengr"o"sen und der Ausnutzung des Besetztheitsgrades.
   Denkbare Varianten w"aren etwa: (1) $m_\kappa^r=m_\eta^c=1$, d.~h.~die
   einzelnen Bl"ocke sind Skalare und es wird die Nullstruktur der Bl"ocke ber"ucksichtigt.
   (2)  $m_\kappa^r=s^r\in\MN$, $m_\eta^c=s^c\in\MN$, d.~h.~feste
   Blockgr"o"sen, die einzelnen Bl"ocke werden als voll besetzt angenommen. (3) die
   Blockgr"o"se ist variabel und es wird die Nullstruktur innerhalb und au"serhalb der
   Bl"ocke ber"ucksichtigt.

   ScalarMatrix implementiert die M"oglichkeit (1), d.~h.~$m_\kappa^r=m_\eta^c=1$.

   \subsubsection*{C++ Implementierung}
   @LATEXOFF @VERBON */
class ScalarMatrix
{
public:
  //allocating/deallocating space
  int Build (int nnr, int nnc, int nnz);
  int Clone (ScalarMatrix *B);       // uses index arrays of B!
  int Free ();       // checks stack position !

  // define structure/fill values, diagonal element is always first
  int Assemble (int row, int nnz, int *cols, int *i, int *j);

  // define access interface here ...
  int Put (int row, int nnz, double *vals, int *cols, int *i, int *j);
  int Add (int row, int nnz, double *vals, int *cols, int *i, int *j);
  int MyAdd (int row, int nnz, double val);
  int Get (int row, int *nnz, double *vals, int *cols, int *i, int *j);
  int Size (int row);

  // debug
  void print (char *s);

  // constructor
  ScalarMatrix (ScalarSparseBLASManager *ssbm);

  friend class ScalarSparseBLASManager;

private:
  int nr;            // number of rows
  int nc;            // number of columns
  int nz;            // total size
  int ctr;           // allocation counter
  int *s;            // row size array
  int *r;            // start of row
  double *a;         // matrix entries
  int *j;        // column indices
  int *columns;

  bool built;        // true if space has been allocated
  int key;           // key from memory manager
  DoubleStackMemoryManager *_dsmm;       // my memory manager
  ScalarSparseBLASManager *_ssbm;       // my blas manager
} ;
/* @VERBOFF */

/* @LATEXON

   \subsubsection*{Konstruktoren}

   \begin{hangpar}
   ScalarMatrix erh\"alt als Parameter ein Objekt der Klasse ScalarSparseBLASManager.
   Dieses enth\"alt die Speicherverwaltung. Der Konstruktor erzeugt nur ein Objekt der
   Klasse ScalarMatrix, nicht jedoch die Matrixkomponenten selbst.
   Der Platz f"ur die Matrix wird erst mit den Funktionen Build bzw.~Clone
   angefordert.
   \end{hangpar}

   \subsubsection*{Methoden}

   \begin{hangpar}
   Build($n^r$,$n^c$,$nnz$) belegt den Speicher f"ur eine Matrix mit $n^r$
   Zeilenbl"ocken, $n^c$ Spaltenbl"ocken und maximal $nnz$ skalaren Nichtnullelementen.
   \end{hangpar}

   \begin{hangpar}
   Clone($B$) belegt den Speicher f"ur eine Matrix, wobei die Blockstruktur (hier
   trivial) als auch die Besetztheitsstruktur von der Matrix $B$ "ubernommen wird.
   Die Komponenten von $B$ werden jedoch nicht "ubernommen (dies macht eine
   BLAS--Routine).
   \end{hangpar}

   \begin{hangpar}
   Free() gibt den Platz f"ur die Matrixdaten wieder frei. Mit der Build Funktion
   kann dann eine neue Matrix  angelegt werden.
   \end{hangpar}

   \begin{hangpar}
   Assemble($\kappa$,$nnz$,$\eta:I_{nz}\to\MN$,$i:I_{nz}\to\MN$,$j:I_{nz}\to\MN$)
   Definiert die Besetztheitsstruktur aller Bl"ocke $(A)_{\kappa\eta}$ der gegebenen
   Matrix $A$, d.~h.~die gesamte Blockzeile $\kappa$.
   Die Bl"ocke d"urfen selbst d"unnbesetzt sein. Die Menge $I_{nz}=\{0,\ldots,
   nnz-1\}$ indiziert die Nichtnullelemente der Blockzeile $\kappa$. F"ur
   $\mu\in I_{nz}$ ist das Element $\left ( (A)_{\kappa,\eta(\mu)} \right )_{i(\mu),
   j(\mu)}$ in die Matrixstruktur aufzunehmen.
   \end{hangpar}

   \begin{hangpar}
   Put($\kappa$,$nnz$,$a:I_{nz}\to\MR$,$\eta:I_{nz}\to\MN$,$i:I_{nz}\to\MN$,$j:I_{nz}\to\MN$)
   schreibt einen Teile der Elemente einer Blockzeile in die Matrix. Die Struktur der
   Blockzeile mu"s vorher mit der Assemble Methode definiert worden sein. Mit den
   Bezeichnungen von oben gilt also $\left ( (A)_{\kappa,\eta(\mu)} \right )_{i(\mu),
   j(\mu)} = a(\mu)$. Beachte: Es m"ussen nicht alle Elemente einer Blockzeile
   auf einmal geschrieben werden, d.~h.~die Felder $\eta,i,j$ k"onnen einen beliebigen
   Teil der Elemente einer Blockzeile (in beliebiger Reihenfolge) adressieren.
   \end{hangpar}

   \begin{hangpar}
   Add($\kappa$,$nnz$,$a:I_{nz}\to\MR$,$\eta:I_{nz}\to\MN$,$i:I_{nz}\to\MN$,$j:I_{nz}\to\MN$).
   Wie Put nur, da"s die Werte addiert statt "uberschrieben werden.
   \end{hangpar}

   \begin{hangpar}
   Get($\kappa$,$nnz$,$a:I_{nz}\to\MR$,$\eta:I_{nz}\to\MN$,$i:I_{nz}\to\MN$,$j:I_{nz}\to\MN$)
   liest Struktur und Elemente der ganzen Blockzeile $\kappa$ aus.
   Alle Parameter (ausser $\kappa$ werden mit den entsprechenden Werten gef"ullt.
   \end{hangpar}

   \begin{hangpar}
   Size($\kappa$) liefert die Anzahl der Nichtnullelemente in der Blockzeile $\kappa$.
   Damit k"onnen also entsprechend gro"se Felder f"ur die Get--Funktion bereitgestellt
   werden.
   \end{hangpar}

   \begin{bemerkung}
   Die Funktionen Assemble, Put, Add, Get und Size sind als inline--Funktionen
   ausgef"uhrt.
   \end{bemerkung}

   @LATEXOFF */
inline int ScalarMatrix::Assemble (int row, int nnz, int *cols, int *ii, int *jj)
{
  // check
  if (row>=nr) return 1;
  if (r[row]!=-1) return 1;       // must be first definition!
  //std::cout << "assembling, row=" << row << " nnz=" << nnz << std::endl;
  //if (ctr+nnz>=nz) return 1;
  for (int k=0; k<nnz; k++)
    if (cols[k]>=nc || ii[k]!=0 || jj[k]!=0) return 1;

  //std::cout << "assembling, row=" << row << " nnz=" << nnz << std::endl;

  // move diagonal element to front (if it exists)
  for (int k=1; k<nnz; k++)
    if (cols[k]==row) {
      cols[k] = cols[0];
      cols[0] = row;
    }

  // insert
  s[row] = nnz;
  r[row] = ctr;
  for (int k=0; k<nnz; k++)
  {
    j[ctr++] = cols[k];
    //std::cout << "j["<<ctr-1<<"]="<<cols[k]<<std::endl;
  }

  return 0;
}

inline int ScalarMatrix::Put (int row, int nnz, double *vals, int *cols, int *ii, int *jj)
{
  int i,k;

  // check
  if ((row<0) || (row>=nr)) return 1;
  if (r[row]==-1) return 1;       // must be defined
  if (nnz>s[row]) return 1;
  for (int k=0; k<nnz; k++)
    if (cols[k]>=nc || ii[k]!=0 || jj[k]!=0) return 1;

  // insert
  for (i=0; i<nnz; i++)
  {
    // check identical position
    if (cols[i]==j[r[row]+i])
    {
      a[r[row]+i] = vals[i];
      //			cout << "a["<<r[row]+i<<"]="<<vals[i]<<" s="<<s[row]<<endl;
    }
    else
    {
      // search
      for (k=r[row]; k<r[row]+s[row]; k++)
        if (j[k]==cols[i])
        {
          a[k] = vals[i];
          break;
        }
    }
  }

  return 0;
}

inline int ScalarMatrix::Add (int row, int nnz, double *vals, int *cols, int *ii, int *jj)
{
  int i,k;

  // check
  if ((row<0) || (row>=nr)) return 1;
  if (r[row]==-1) return 1;       // must be defined
  if (nnz>s[row]) return 1;
  for (int k=0; k<nnz; k++)
    if (cols[k]>=nc || ii[k]!=0 || jj[k]!=0) return 1;

  // insert
  for (i=0; i<nnz; i++)
  {
    // check identical position
    if (cols[i]==j[r[row]+i])
      a[r[row]+i] += vals[i];
    else
    {
      // search
      for (k=r[row]; k<r[row]+s[row]; k++)
        if (j[k]==cols[i])
        {
          a[k] += vals[i];
          break;
        }
    }
  }

  return 0;
}

inline int ScalarMatrix::MyAdd (int row, int col,double val)
{
  int i,k;

  // nny steht fuer col

  //  printf("Add ( %d , %d )  val = %f\n",row,col,val);

  // check
  if ((row<0) || (row>=nr)) return 1;
#if 0
  if (r[row]==-1) return 1; // must be defined
  if (col>s[row]) return 1;
  for (int k=0; k<s[row]; k++)
    if (columns[k]>=nc) return 1;
#endif
  int maxcol_ = s[row];

  // search
  bool colUsed = false;
  for (int i=0; i<maxcol_; i++)
  {
    if(columns[row*maxcol_ +i] == col)
    {
      a[row*maxcol_+i] += val;
      colUsed = true;
      return 0;
    }
  }

  if(!colUsed)
  {
    // search
    for (int i=0; i<maxcol_; i++)
      if (columns[row*maxcol_+i] == -1)
      {
        a[row*maxcol_+i] += val;
        columns[row*maxcol_+i] = col;
        return 0;
      }
  }

  //std::cout << "Column to small to save value \n";
  //abort();
  return 0;
}

inline int ScalarMatrix::Get (int row, int *nnz, double *vals, int *cols, int *ii, int *jj)
{
  // check
  if ((row<0) || (row>=nr)) return 1;
  if (r[row]==-1) return 1;       // must be defined
  if (*nnz<s[row]) return 1;      // must contain size of arrays on entry

  // read
  *nnz = s[row];
  for (int i=0; i<s[row]; i++)
  {
    vals[i] = a[r[row]+i];
    cols[i] = j[r[row]+i];
    ii[i]=0; jj[i]=0;
  }

  return 0;
}

inline int ScalarMatrix::Size (int row)
{
  // check
  if ((row<0) || (row>=nr)) return -1;
  if (r[row]==-1) return -1;       // must be defined

  // read
  return s[row];
}


/* @LATEXON

   \subsection{Klasse {\rm ScalarSparseBLASManager}}

   \subsubsection*{Dateiname}

   @FILENAME

   \subsubsection*{Klassenattribute}

   konkret

   \subsubsection*{Klassenbaum}

   Basisklasse

   \subsubsection*{Assozierte Klassen}

   DoubleStackMemoryManager, GeneralHeapMemoryManager,
   ScalarVector, ScalarMatrix

   \subsubsection*{Beschreibung}

   BLAS (Basic Linear Algebra Subroutines) ist eine Sammlung elementarer
   Operationen auf vollbesetzten Matrizen. Die Operationen sind so gew"ahlt, da"s
   sich praktisch alle Operationen der linearen Algebra auf diese Grundfunktionen
   zur"uckf"uhren lassen. Die BLAS--Funktionen gibt es f"ur viele Rechner
   als speziell optimierte Bibliotheken. Man unterscheidet BLAS level 1
   (Vektor--Vektor Operationen), BLAS level 2 (Matrix--Vektor) und level
   3 (Matrix--Matrix). Seit einigen Jahren wird auch an einer Definition eines
   BLAS Standards f"ur d"unnbesetzte Matrizen gearbeitet.

   Die Klasse ScalarSparseBLASManager stellt einige Grundfunktionen im Sinne
   von BLAS auf den oben definierten Klassen ScalarVector und ScalarMatrix zur
   Verf"ugung. Dabei handelt es sich um level 1 und level 2 Funktionalit"at sowie
   um Iterationsverfahren.

   Weiterhin erledigt ScalarSparseBLASManager die Speicherverwaltung f"ur
   ScalarVector/ScalarMatrix. Jedes ScalarVector/ScalarMatrix--Objekt
   geh"ort zu genau einem ScalarSparseBLASManager--Objekt (siehe Parameter
   der Konstruktoren. Bei einem Aufruf der Build--Funktion wird speicher
   vom zugeh"origen Manager angefordert. Dies gibt dem Manager auch die
   M"oglichkeit die Reihenfolge zu vermerken in der die Vektoren allokiert
   wurden. Bei einer eventuellen dynamischen Vergr"o"serung eines oder
   mehrerer Vektoren kann dadurch das umkopieren des Speicherinhaltes effektiv
   gesteuert werden (Diese Funktionalit"at ist noch nicht vollst"andig implementiert).

   \subsubsection*{C++ Implementierung}
   @LATEXOFF @VERBON */
class ScalarSparseBLASManager
{
public:
  // level 1
  void vset (ScalarVector *x, double val);
  void vcopy (ScalarVector *x, ScalarVector *y);
  void vscale (ScalarVector *x, double val);
  void daxpy (ScalarVector *x, ScalarVector *y, double val);
  double ddot (ScalarVector *x, ScalarVector *y);
  double infnorm (ScalarVector *x);

  // level 2
  void mset (ScalarMatrix *A, double val);
  void mcopy (ScalarMatrix *A, ScalarMatrix *B);
  void matmul (ScalarVector *x, ScalarMatrix *A, ScalarVector *y);
  void matmulplus (ScalarVector *x, double val, ScalarMatrix *A, ScalarVector *y);
  void matmulplustranspose (ScalarVector *x, double val, ScalarMatrix *A, ScalarVector *y);
  void sor (int nit, ScalarMatrix *A, ScalarVector *x, ScalarVector *b, double damp);
  void ssor (int nit, ScalarMatrix *A, ScalarVector *x, ScalarVector *b, double damp);
  void jac (int nit, ScalarMatrix *A, ScalarVector *x, ScalarVector *b, double damp);

  // goodies
  int Resize ();

  // constructor
  ScalarSparseBLASManager (DoubleStackMemoryManager *dsmm,
                           GeneralHeapMemoryManager *ghmm,
                           int nv, int nm);
  ~ScalarSparseBLASManager();

  friend class ScalarVector;       // in order to access
  friend class ScalarMatrix;       // push/pop

  // export memory manager
  DoubleStackMemoryManager *mydsmm () {return _dsmm;};

private:
  // register mechanism
  int PushVector (ScalarVector *v);      // push/pop
  int PopVector (ScalarVector *v);       // must match !
  int PushMatrix (ScalarMatrix *m);      // push/pop
  int PopMatrix (ScalarMatrix *m);       // must match !

  DoubleStackMemoryManager *_dsmm;
  GeneralHeapMemoryManager *_ghmm;
  int _nv;
  int _nm;

  int builtVecCtr;
  ScalarVector **builtVecRef;

  int builtMatCtr;
  ScalarMatrix **builtMatRef;
} ;
/* @VERBOFF @LATEXON

   \subsubsection*{Konstruktoren}

   \begin{hangpar}
   ScalarSparseBLASManager(dsmm,ghmm,$nv$, $nm$) erzeugt ein Manager--Objekt.
   Als Parameter sind zwei Speicherverwaltungsobjekte sowie die maximal
   ben"otigte Zahl von Vektoren ($nv$) und Matrizen ($nm$) zu "ubergeben.
   \end{hangpar}

   \subsubsection*{Destruktoren}

   \begin{hangpar}
   $\sim$ScalarSparseBLASManager() pr"uft ob noch Vektoren oder Matrizen
   speicher belegt haben und gibt gegebenenfalls eine Meldung aus.
   \end{hangpar}

   \subsubsection*{Methoden}

   \begin{hangpar}
   vset($x$,$a\in\MR$) setzt alle Komponenten des Vektors $x$ auf den Wert $a$.
   \end{hangpar}

   \begin{hangpar}
   vcopy($x$,$y$) setzt $x=y$. Die Gr"o"sen der Vektoren m"ussen "ubereinstimmen.
   \end{hangpar}

   \begin{hangpar}
   vscale($x$,$a\in\MR$) setzt $x=ax$.
   \end{hangpar}

   \begin{hangpar}
   daxpy($x$,$y$,$a\in\MR$) setzt $x=x+ay$.
   \end{hangpar}

   \begin{hangpar}
   ddot($x$,$y$) liefert $x^Ty$ zur"uck.
   \end{hangpar}

   \begin{hangpar}
   infnorm($x$) berechnet $\|x\|_\infty = \max_i |x_i|$.
   \end{hangpar}

   \begin{hangpar}
   mset($A$,$a\in\MR$) setzt alle Elemente der Matrix $A$ auf den Wert $a$.
   \end{hangpar}

   \begin{hangpar}
   mcopy($A$,$B$) setzt $A=B$ ($A$ erh"alt die Werte von $B$).
   \end{hangpar}

   \begin{hangpar}
   matmul($x$,$A$,$y$) berechnet $x=Ay$.
   \end{hangpar}

   \begin{hangpar}
   matmulplus($x$,$a\in\MR$,$A$,$y$) berechnet $x=x+aAy$.
   \end{hangpar}

   \begin{hangpar}
   matmulplustranspose($x$,$a\in\MR$,$A$,$y$) berechnet $x=x+aA^Ty$.
   \end{hangpar}

   \begin{hangpar}
   sor($n$,$A$,$x$,$b$,$\omega\in\MR$) f"uhrt $n$ Schritte des SOR--Verfahrens mit
   Relaxationsfaktor $\omega$ auf dem Gleichungssystem $Ax=b$ durch.
   \end{hangpar}

   \begin{hangpar}
   jac($n$,$A$,$x$,$b$,$\omega\in\MR$) f"uhrt $n$ Schritte des mit $\omega$ ged"ampften
   Jacobi--Verfahrens auf dem Gleichungssystem $Ax=b$ durch.
   \end{hangpar}

   @LATEXOFF */

#include "scalarblas.cc"

#endif
