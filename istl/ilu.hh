// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ILU_HH__
#define __DUNE_ILU_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <map>

#include "istlexception.hh"
#include "io.hh"

/** \file
 * \brief  ???
 */

namespace Dune {

  /** @addtogroup ISTL
          @{
   */

  //! compute ILU decomposition of A. A is overwritten by its decomposition
  template<class M>
  void bilu0_decomposition (M& A)
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
  void bilu_backsolve (const M& A, X& v, const Y& d)
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



  // recursive function template to access first entry of a matrix
  template<class M>
  typename M::field_type& firstmatrixelement (M& A)
  {
    return firstmatrixelement(*(A.begin()->begin()));
  }

  template<class K, int n, int m>
  K& firstmatrixelement (FieldMatrix<K,n,m>& A)
  {
    return A[0][0];
  }

  template<class K>
  K& firstmatrixelement (FieldMatrix<K,1,1>& A)
  {
    return A[0][0];
  }


  /*! ILU decomposition of order n
          Computes ILU decomposition of order n. The matrix ILU should
      be an empty matrix in row_wise creation mode. This allows the user
      to either specify the number of nonzero elements or to
          determine it automatically at run-time.
   */
  template<class M>
  void bilu_decomposition (const M& A, int n, M& ILU)
  {
    // iterator types
    typedef typename M::RowIterator rowiterator;
    typedef typename M::ColIterator coliterator;
    typedef typename M::ConstRowIterator crowiterator;
    typedef typename M::ConstColIterator ccoliterator;
    typedef typename M::CreateIterator createiterator;
    typedef typename M::block_type block;
    typedef typename M::field_type K;
    typedef typename std::map<int,int> map;
    typedef typename std::map<int,int>::iterator mapiterator;

    // symbolic factorization phase, store generation number in first matrix element
    crowiterator endi=A.end();
    createiterator ci=ILU.createbegin();
    for (crowiterator i=A.begin(); i!=endi; ++i)
    {
      //		std::cout << "in row " << i.index() << std::endl;
      map rowpattern;           // maps column index to generation

      // initialize pattern with row of A
      for (ccoliterator j=(*i).begin(); j!=(*i).end(); ++j)
        rowpattern[j.index()] = 0;

      // eliminate entries in row which are to the left of the diagonal
      for (mapiterator ik=rowpattern.begin(); (*ik).first<i.index(); ++ik)
      {
        if ((*ik).second<n)
        {
          //                            std::cout << "  eliminating " << i.index() << "," << (*ik).first
          //                                              << " level " << (*ik).second << std::endl;

          coliterator endk = ILU[(*ik).first].end();                       // end of row k
          coliterator kj = ILU[(*ik).first].find((*ik).first);                       // diagonal in k
          for (++kj; kj!=endk; ++kj)                       // row k eliminates in row i
          {
            int generation = (int) firstmatrixelement(*kj);
            if (generation<n)
            {
              mapiterator ij = rowpattern.find(kj.index());
              if (ij==rowpattern.end())
              {
                //std::cout << "    new entry " << i.index() << "," << kj.index()
                //                                                << " level " << (*ik).second+1 << std::endl;

                rowpattern[kj.index()] = generation+1;
              }
            }
          }
        }
      }

      // create row
      for (mapiterator ik=rowpattern.begin(); ik!=rowpattern.end(); ++ik)
        ci.insert((*ik).first);
      ++ci;           // now row i exist

      // write generation index into entries
      coliterator endILUij = ILU[i.index()].end();;
      for (coliterator ILUij=ILU[i.index()].begin(); ILUij!=endILUij; ++ILUij)
        firstmatrixelement(*ILUij) = (K) rowpattern[ILUij.index()];
    }

    //	printmatrix(std::cout,ILU,"ilu pattern","row",10,2);

    // copy entries of A
    for (crowiterator i=A.begin(); i!=endi; ++i)
    {
      coliterator ILUij;
      coliterator endILUij = ILU[i.index()].end();;
      for (ILUij=ILU[i.index()].begin(); ILUij!=endILUij; ++ILUij)
        (*ILUij) = 0;           // clear row
      ccoliterator Aij = (*i).begin();
      ccoliterator endAij = (*i).end();
      ILUij = ILU[i.index()].begin();
      while (Aij!=endAij && ILUij!=endILUij)
      {
        if (Aij.index()==ILUij.index())
        {
          *ILUij = *Aij;
          ++Aij; ++ILUij;
        }
        else
        {
          if (Aij.index()<ILUij.index())
            ++Aij;
          else
            ++ILUij;
        }
      }
    }

    // call decomposition on pattern
    bilu0_decomposition(ILU);
  }


  /** @} end documentation */

} // end namespace

#endif
