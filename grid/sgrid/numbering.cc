// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __NUMBERING_CC__
#define __NUMBERING_CC__

#include <iostream>

namespace Dune {

  template<int dim>
  inline void LexOrder<dim>::init (const FixedArray<int,dim>& _NN)
  {
    // store argument
    N=_NN;

    // build P array
    P[0] = 1;
    for (int i=1; i<=dim; i++) P[i] = P[i-1]*N[i-1];
  }

  template<int dim>
  inline int LexOrder<dim>::tupels () const
  {
    return P[dim];
  }

  template<int dim>
  inline int LexOrder<dim>::n (const FixedArray<int,dim>& z) const
  {
    int r=0;
    for (int i=0; i<dim; i++) r += z[i]*P[i];
    return r;
  }

  template<int dim>
  inline FixedArray<int,dim> LexOrder<dim>::z (int n) const
  {
    FixedArray<int,dim> z;
    for (int i=0; i<dim; i++)
    {
      z[i] = n%N[i];
      n = n/N[i];
    }
    return z;
  }

  //************************************************************************

  template<int dim>
  inline void JoinOrder<dim>::init (const FixedArray<int,dim>& _NN)
  {
    // store argument
    N=_NN;

    // build P array
    offset[0] = 0;
    for (int i=1; i<=dim; i++) offset[i] = offset[i-1]+N[i-1];
  }

  template<int dim>
  inline int JoinOrder<dim>::size () const
  {
    return offset[dim];
  }

  template<int dim>
  inline int JoinOrder<dim>::n (int subset, int index) const
  {
    return index+offset[subset];
  }

  template<int dim>
  inline int JoinOrder<dim>::index (int n) const
  {
    for (int i=0; i<dim; i++)
      if (N[i]!=0)
      {
        if (n<N[i]) return n;
        n -= N[i];
      }
    return n;     // oops, error !
  }

  template<int dim>
  inline int JoinOrder<dim>::subset (int n) const
  {
    for (int i=0; i<dim; i++)
      if (N[i]!=0)
      {
        if (n<N[i]) return i;
        n -= N[i];
      }
    return 0;     // oops, error !
  }

  //************************************************************************

  template<int dim>
  CubeMapper<dim>::CubeMapper (const FixedArray<int,dim>& _NN)
  {
    make(_NN);
  }

  template<int dim>
  CubeMapper<dim>::CubeMapper ()
  {
    FixedArray<int,dim> M;

    // make mesh of single cube
    for (int i=0; i<dim; i++) M[i]=1;

    make(M);
  }

  template<int dim>
  void CubeMapper<dim>::make (const FixedArray<int,dim>& _NN)
  {
    // store argument
    N=_NN;

    // preprocess binary partitions
    for (int i=0; i<=dim; i++) ne[i] = 0;
    for (int b=0; b<power2(dim); b++)     // loop over all binary partitions
    {
      int mask=1;
      FixedArray<int,dim> t;
      for (int i=0; i<dim; i++)
      {
        if (b&mask)
        {
          // bit i is even
          t[i] = N[i]+1;
        }
        else
        {
          // bit i is odd
          t[i] = N[i];
        }
        mask *= 2;
      }
      lex[b].init(t);           // set up lex ordering of tupels
      nb[b] = lex[b].tupels();
      cb[b] = ones(b);
      ne[cb[b]] += nb[b];
    }

    // preprocess lex ordering for each codimension
    for (int c=0; c<=dim; c++)
    {
      FixedArray<int,1<<dim> t;
      for (int b=0; b<power2(dim); b++)           // loop over all binary partitions
        if (ones(b)==c)
        {
          // partition b is part of codim c
          t[b] = nb[b];
        }
        else
        {
          // partition does not contribute to codim c
          t[b] = 0;
        }
      join[c].init(t);           // set join mapper
    }
  }

  template<int dim>
  inline void CubeMapper<dim>::print (std::ostream& ss, int indent) const
  {
    for (int k=0; k<indent; k++) ss << " ";
    ss << "CubeMapper [" ;
    for (int i=0; i<dim; i++)
      std::cout << N[i] << " ";
    ss << "]" << std::endl;
    for (int i=0; i<=dim; i++)
    {
      for (int k=0; k<indent; k++) ss << " ";
      ss << "  " << ne[i] << " elements of codim " << i
         << " in dimension " << dim << std::endl;
    }
  }

  template<int dim>
  inline int CubeMapper<dim>::elements (int codim) const
  {
    return ne[codim];
  }

  template<int dim>
  inline int CubeMapper<dim>::codim (const FixedArray<int,dim>& z) const
  {
    int r=0;
    for (int i=0; i<dim; i++)
      if (z[i]%2==0) r++;           // count even components
    return r;
  }

  template<int dim>
  inline int CubeMapper<dim>::n (const FixedArray<int,dim>& z) const
  {
    int p = partition(z);            // get partition
    FixedArray<int,dim> r=compress(z);     // get compressd coordinate

    // treat easy cases first
    if (p==0 || p==power2(dim)-1)
    {
      // all components are either odd or even
      return lex[p].n(r);
    }

    // general case
    return join[ones(p)].n(p,lex[p].n(r));
  }

  template<int dim>
  inline FixedArray<int,dim> CubeMapper<dim>::z (int i, int codim) const
  {
    FixedArray<int,dim> r;

    // easy cases first
    if (codim==0)
    {
      r = lex[0].z(i);
      return expand(r,0);
    }
    if (codim==dim)
    {
      r = lex[power2(dim)-1].z(i);
      return expand(r,power2(dim)-1);
    }

    // general case
    int p = join[codim].subset(i);
    int n = join[codim].index(i);
    r = lex[p].z(n);
    return expand(r,p);
  }

  template<int dim>
  inline int CubeMapper<dim>::ones (int b) const
  {
    int r = 0;
    int mask = 1;
    for (int i=0; i<dim; i++)
    {
      if (b&mask) r++;
      mask *=2 ;
    }
    return r;
  }

  template<int dim>
  inline int CubeMapper<dim>::partition (const FixedArray<int,dim>& z) const
  {
    int r = 0;
    int mask = 1;
    for (int i=0; i<dim; i++)
    {
      if (z[i]%2==0) r += mask;
      mask *=2 ;
    }
    return r;
  }

  template<int dim>
  inline FixedArray<int,dim> CubeMapper<dim>::compress (const FixedArray<int,dim>& z) const
  {
    FixedArray<int,dim> r;
    for (int i=0; i<dim; i++)
      if (z[i]%2==0)
        r[i] = z[i]/2;                     // even component
      else
        r[i] = (z[i]-1)/2;                 // odd component
    return r;
  }

  template<int dim>
  inline FixedArray<int,dim> CubeMapper<dim>::expand (const FixedArray<int,dim>& r, int b) const
  {
    FixedArray<int,dim> z;
    for (int i=0; i<dim; i++)
      if (b&(1<<i))
        z[i] = r[i]*2;                     // even component
      else
        z[i] = 2*r[i]+1;                   // odd component
    return z;
  }

}

#endif
