// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ILU_HH__
#define __DUNE_ILU_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>

#include "istlexception.hh"
#include "io.hh"

/*! \file __FILE__

    Simple iterative methods like Jacobi, Gauss-Seidel, SOR, SSOR, etc.
    in a generic way
 */

namespace Dune {

  /** @addtogroup ISTL
          @{
   */

  //! compute ILU decomposition of A. A is overwritten by its decomposition
  template<class M>
  void ilu0_decomposition (M& A)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;
    typedef typename M::block_type block;

    // implement left looking variant with stored inverse
    rowiterator endi=A.end();
    for (rowiterator i=A.begin(); i!=endi; ++i)
    {
      // coliterator is diagonal after the following loop
      coliterator endij=(*i).end();           // end of row i
      coliterator ij;

      // eliminate entries left of diagonal; store L factor
      for (ij=(*i).begin(); ij.index()<i.index(); ++ij)
      {
        // find A_jj which eliminates A_ij
        coliterator jj = A[ij.index()].find(ij.index());

        // compute L_ij = A_jj^-1 * A_ij
        (*ij).rightmultiply(*jj);

        // modify row
        coliterator endjk=A[ij.index()].end();                 // end of row j
        coliterator jk=jj; ++jk;
        coliterator ik=ij; ++ik;
        while (ik!=endij && jk!=endjk)
          if (ik.index()==jk.index())
          {
            block B(*jk);
            B.leftmultiply(*ij);
            *ik -= B;
            ++ik; ++jk;
          }
          else
          {
            if (ik.index()<jk.index())
              ++ik;
            else
              ++jk;
          }
      }

      // invert pivot and store it in A
      if (ij.index()!=i.index())
        DUNE_THROW(ISTLError,"diagonal entry missing");
      (*ij).invert();           // compute inverse of diagonal block
    }
  }

  //! LU backsolve with stored inverse
  template<class M, class X, class Y>
  void ilu_backsolve (const M& A, X& v, const Y& d)
  {
    // iterator types
    typedef typename M::ConstRowIterator rowiterator;
    typedef typename M::ConstColIterator coliterator;
    typedef typename Y::block_type dblock;
    typedef typename X::block_type vblock;

    // lower triangular solve
    rowiterator endi=A.end();
    for (rowiterator i=A.begin(); i!=endi; ++i)
    {
      dblock rhs(d[i.index()]);
      for (coliterator j=(*i).begin(); j.index()<i.index(); ++j)
        (*j).mmv(v[j.index()],rhs);
      v[i.index()] = rhs;           // Lii = I
    }

    // upper triangular solve
    rowiterator rendi=A.rend();
    for (rowiterator i=A.rbegin(); i!=rendi; --i)
    {
      vblock rhs(v[i.index()]);
      coliterator j;
      for (j=(*i).rbegin(); j.index()>i.index(); --j)
        (*j).mmv(v[j.index()],rhs);
      v[i.index()] = 0;
      (*j).umv(rhs,v[i.index()]);           // diagonal stores inverse!
    }
  }


  /** @} end documentation */

} // end namespace

#endif
