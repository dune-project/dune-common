// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __NUMBERING_CC__
#define __NUMBERING_CC__

namespace Dune {

  template<int dim>
  inline void LexOrder<dim>::init (Tupel<int,dim>& _N)
  {
    // store argument
    N=_N;

    // build P array
    P[0] = 1;
    for (int i=1; i<=dim; i++) P[i] = P[i-1]*N[i-1];
  }

  template<int dim>
  inline int LexOrder<dim>::tupels ()
  {
    return P[dim];
  }

  template<int dim>
  inline int LexOrder<dim>::n (Tupel<int,dim>& z)
  {
    int r=0;
    for (int i=0; i<dim; i++) r += z[i]*P[i];
    return r;
  }

  template<int dim>
  inline Tupel<int,dim> LexOrder<dim>::z (int n)
  {
    Tupel<int,dim> z;
    for (int i=0; i<dim; i++)
    {
      z[i] = n%N[i];
      n = n/N[i];
    }
    return z;
  }

  //************************************************************************

  template<int dim>
  inline void JoinOrder<dim>::init (Tupel<int,dim>& _N)
  {
    // store argument
    N=_N;

    // build P array
    offset[0] = 0;
    for (int i=1; i<=dim; i++) offset[i] = offset[i-1]+N[i-1];
  }

  template<int dim>
  inline int JoinOrder<dim>::size ()
  {
    return offset[dim];
  }

  template<int dim>
  inline int JoinOrder<dim>::n (int subset, int index)
  {
    return index+offset[subset];
  }

  template<int dim>
  inline int JoinOrder<dim>::index (int n)
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
  inline int JoinOrder<dim>::subset (int n)
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
  CubeMapper<dim>::CubeMapper (Tupel<int,dim> _N)
  {
    make(_N);
  }

  template<int dim>
  CubeMapper<dim>::CubeMapper ()
  {
    Tupel<int,dim> M;

    // make mesh of single cube
    for (int i=0; i<dim; i++) M[i]=1;

    make(M);
  }

  template<int dim>
  void CubeMapper<dim>::make (Tupel<int,dim>& _N)
  {
    // store argument
    N=_N;

    // preprocess binary partitions
    for (int i=0; i<=dim; i++) ne[i] = 0;
    for (int b=0; b<power2(dim); b++)     // loop over all binary partitions
    {
      int mask=1;
      Tupel<int,dim> t;
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
      Tupel<int,1<<dim> t;
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
  inline void CubeMapper<dim>::print (std::ostream& ss, int indent)
  {
    for (int k=0; k<indent; k++) ss << " ";
    ss << "CubeMapper [" ;
    for (int i=0; i<dim; i++)
      cout << N[i] << " ";
    ss << "]" << endl;
    for (int i=0; i<=dim; i++)
    {
      for (int k=0; k<indent; k++) ss << " ";
      ss << "  " << ne[i] << " elements of codim " << i
         << " in dimension " << dim << endl;
    }
  }

  template<int dim>
  inline int CubeMapper<dim>::elements (int codim)
  {
    return ne[codim];
  }

  template<int dim>
  inline int CubeMapper<dim>::codim (Tupel<int,dim>& z)
  {
    int r=0;
    for (int i=0; i<dim; i++)
      if (z[i]%2==0) r++;           // count even components
    return r;
  }

  template<int dim>
  inline int CubeMapper<dim>::n (Tupel<int,dim>& z)
  {
    int p = partition(z);            // get partition
    Tupel<int,dim> r=compress(z);     // get compressd coordinate

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
  inline Tupel<int,dim> CubeMapper<dim>::z (int i, int codim)
  {
    Tupel<int,dim> r;

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
  inline int CubeMapper<dim>::ones (int b)
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
  inline int CubeMapper<dim>::partition (Tupel<int,dim>& z)
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
  inline Tupel<int,dim> CubeMapper<dim>::compress (Tupel<int,dim>& z)
  {
    Tupel<int,dim> r;
    for (int i=0; i<dim; i++)
      if (z[i]%2==0)
        r[i] = z[i]/2;                     // even component
      else
        r[i] = (z[i]-1)/2;                 // odd component
    return r;
  }

  template<int dim>
  inline Tupel<int,dim> CubeMapper<dim>::expand (Tupel<int,dim>& r, int b)
  {
    Tupel<int,dim> z;
    for (int i=0; i<dim; i++)
      if (b&(1<<i))
        z[i] = r[i]*2;                     // even component
      else
        z[i] = 2*r[i]+1;                   // odd component
    return z;
  }

}

#endif
