// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_GSETC_HH__
#define __DUNE_GSETC_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>

#include "istlexception.hh"
#include "fvector.hh"
#include "fmatrix.hh"

/*! \file

    Simple iterative methods like Jacobi, Gauss-Seidel, SOR, SSOR, etc.
    in a generic way
 */

namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */

  //! return x = x + w D(A)^{-1} (b-Ax)
  template<class M, class X, class Y, class K>
  void bjac (M& A, X& x, Y& b, K w)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;

    // cast to field type
    typename X::field_type omega=w;

    // compute defect
    Y d(b);                     // d=b
    A.mmv(x,d);                 // d=d-Ax

    // invert block diagonal, overwrite d with update
    for (rowiterator i=A.begin(); i!=A.end(); ++i)
    {
      coliterator j = (*i).find(i.index());
      (*j).solve(d[i.index()],d[i.index()]);
    }

    // update
    x.axpy(omega,d);
  }

  //! return v =  D(A)^{-1} d
  template<class M, class X, class Y>
  void bjac_update (M& A, X& v, Y& d)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;

    // invert block diagonal, overwrite d with update
    rowiterator endi=A.end();
    for (rowiterator i=A.begin(); i!=endi; ++i)
    {
      coliterator j = (*i).find(i.index());
      (*j).solve(v[i.index()],d[i.index()]);
    }
  }

  //! return x = x + L(A)^{-1} (b-Ax), implemented without temporary vector
  template<class M, class X, class Y>
  void bgs (M& A, X& x, Y& b)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;
    typedef typename Y::block_type bblock;

    // local solve at each block and immediate update
    coliterator diag;
    for (rowiterator i=A.begin(); i!=A.end(); ++i)
    {
      // compute right hand side
      bblock rhs(b[i.index()]);
      for (typename M::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
        if (j.index()!=i.index())
          (*j).mmv(x[j.index()],rhs);
        else
          diag = j;

      // solve locally
      (*diag).solve(x[i.index()],rhs);
    }
  }

  //! return v = L(A)^{-1} d
  template<class M, class X, class Y>
  void bgs_update (M& A, X& v, Y& d)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;
    typedef typename Y::block_type bblock;

    // local solve at each block and immediate update
    rowiterator endi=A.end();
    for (rowiterator i=A.begin(); i!=endi; ++i)
    {
      // compute right hand side
      bblock rhs(d[i.index()]);
      coliterator j;
      for (j=(*i).begin(); j.index()<i.index(); ++j)
        (*j).mmv(v[j.index()],rhs);

      // solve locally
      (*j).solve(v[i.index()],rhs);
    }
  }

  //! return x = x + w L(A)^{-1} (b-Ax), implemented without temporary vector
  template<class M, class X, class Y, class K>
  void bsor (M& A, X& x, Y& b, K w)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;
    typedef typename X::block_type xblock;
    typedef typename Y::block_type bblock;

    // cast to field type
    typename X::field_type omega=w;

    // local solve at each block and immediate update
    coliterator diag;
    for (rowiterator i=A.begin(); i!=A.end(); ++i)
    {
      // compute right hand side
      bblock rhs(b[i.index()]);
      for (typename M::ColIterator j=(*i).begin(); j!=(*i).end(); ++j)
        if (j.index()!=i.index())
          (*j).mmv(x[j.index()],rhs);
        else
          diag = j;

      // solve locally
      xblock xnew;
      (*diag).solve(xnew,rhs);

      // damped update
      x[i.index()] *= 1-omega;
      x[i.index()].axpy(omega,xnew);
    }
  }

  //! return v = w L(A)^{-1} d
  template<class M, class X, class Y, class K>
  void bsor_update (M& A, X& v, Y& d, K w)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;
    typedef typename Y::block_type bblock;

    // cast to field type
    typename X::field_type omega=w;

    // local solve at each block and immediate update
    coliterator diag;
    for (rowiterator i=A.begin(); i!=A.end(); ++i)
    {
      // compute right hand side
      bblock rhs(d[i.index()]);
      for (typename M::ColIterator j=(*i).begin(); j.index()<=i.index(); ++j)
        if (j.index()<i.index())
          (*j).mmv(v[j.index()],rhs);
        else
          diag = j;

      // solve locally
      (*diag).solve(v[i.index()],rhs);

      // damp immediately
      v[i.index] *= omega;
    }
  }


  /** @} end documentation */

} // end namespace

#endif
