// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ISTLIO_HH__
#define __DUNE_ISTLIO_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>

#include "istlexception.hh"
#include "dune/common/fvector.hh"
#include "fmatrix.hh"

/*! \file __FILE__

   Some generic functions for pretty printing vectors and matrices
 */

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */


  //==== pretty printing of vectors

  // recursively print all the blocks
  template<class V>
  void recursive_printvector (std::ostream& s, const V& v, std::string rowtext, int& counter,
                              int columns, int width, int precision)
  {
    for (typename V::ConstIterator i=v.begin(); i!=v.end(); ++i)
      recursive_printvector(s,*i,rowtext,counter,columns,width,precision);
  }

  // specialization for FieldVector
  template<class K, int n>
  void recursive_printvector (std::ostream& s, const FieldVector<K,n>& v, std::string rowtext, int& counter,
                              int columns, int width, int precision)
  {
    // we now can print n numbers
    for (int i=0; i<n; i++)
    {
      if (counter%columns==0)
      {
        s << rowtext;                 // start a new row
        s << " ";                     // space in front of each entry
        s.width(4);                   // set width for counter
        s << counter;                 // number of first entry in a line
      }
      s << " ";                   // space in front of each entry
      s.width(width);             // set width for each entry anew
      s << v[i];                  // yeah, the number !
      counter++;                  // increment the counter
      if (counter%columns==0)
        s << std::endl;           // start a new line
    }
  }


  template<class V>
  void printvector (std::ostream& s, const V& v, std::string title, std::string rowtext,
                    int columns=1, int width=10, int precision=2)
  {
    // count the numbers printed to make columns
    int counter=0;

    // set the output format
    s.setf(std::ios_base::scientific, std::ios_base::floatfield);
    int oldprec = s.precision();
    s.precision(precision);

    // print title
    s << title << " [blocks=" << v.N() << ",dimension=" << v.dim() << "]" << std::endl;

    // print data from all blocks
    recursive_printvector(s,v,rowtext,counter,columns,width,precision);

    // check if new line is required
    if (counter%columns!=0)
      s << std::endl;

    // reset the output format
    s.precision(oldprec);
    s.setf(std::ios_base::fixed, std::ios_base::floatfield);
  }


  //==== pretty printing of matrices


  //! print a row of zeros for a non-existing block
  void fill_row (std::ostream& s, int m, int width, int precision)
  {
    for (int j=0; j<m; j++)
    {
      s << " ";                   // space in front of each entry
      s.width(width);             // set width for each entry anew
      s << "0";                   // yeah, the number !
    }
  }

  //! print one row of a matrix
  template<class M>
  void print_row (std::ostream& s, const M& A, int I, int J, int therow, int width, int precision)
  {
    int i0=I;
    for (int i=0; i<A.N(); i++)
    {
      if (therow>=i0 && therow<i0+A.rowdim(i))
      {
        // the row is in this block row !
        int j0=J;
        for (int j=0; j<A.M(); j++)
        {
          // find this block
          typename M::ConstColIterator it = A[i].find(j);

          // print row or filler
          if (it!=A[i].end())
            print_row(s,*it,i0,j0,therow,width,precision);
          else
            fill_row(s,A.coldim(j),width,precision);

          // advance columns
          j0 += A.coldim(j);
        }
      }
      // advance rows
      i0 += A.rowdim(i);
    }
  }

  //! print one row of a matrix, specialization for FieldMatrix
  template<class K, int n, int m>
  void print_row (std::ostream& s, const FieldMatrix<K,n,m>& A, int I, int J, int therow, int width, int precision)
  {
    for (int i=0; i<n; i++)
      if (I+i==therow)
        for (int j=0; j<m; j++)
        {
          s << " ";                         // space in front of each entry
          s.width(width);                   // set width for each entry anew
          s << A[i][j];                     // yeah, the number !
        }
  }

  //! print one row of a matrix, specialization for K11Matrix
  template<class K>
  void print_row (std::ostream& s, const K11Matrix<K>& A, int I, int J, int therow, int width, int precision)
  {
    if (I==therow)
    {
      s << " ";                   // space in front of each entry
      s.width(width);             // set width for each entry anew
      s << A();                   // yeah, the number !
    }
  }

  template<class M>
  void printmatrix (std::ostream& s, const M& A, std::string title, std::string rowtext,
                    int width=10, int precision=2)
  {
    // set the output format
    s.setf(std::ios_base::scientific, std::ios_base::floatfield);
    int oldprec = s.precision();
    s.precision(precision);

    // print title
    s << title
      << " [n=" << A.N()
      << ",m=" << A.M()
      << ",rowdim=" << A.rowdim()
      << ",coldim=" << A.coldim()
      << "]" << std::endl;

    // print all rows
    for (int i=0; i<A.rowdim(); i++)
    {
      s << rowtext;            // start a new row
      s << " ";                // space in front of each entry
      s.width(4);              // set width for counter
      s << i;                  // number of first entry in a line
      print_row(s,A,0,0,i,width,precision);           // generic print
      s << std::endl;          // start a new line
    }

    // reset the output format
    s.precision(oldprec);
    s.setf(std::ios_base::fixed, std::ios_base::floatfield);
  }


  /** @} end documentation */

} // end namespace

#endif
