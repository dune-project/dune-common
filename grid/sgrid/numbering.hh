// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __NUMBERING_HH__
#define __NUMBERING_HH__

namespace Dune {

  //! class holding d-dimensional array of type T
  template<class T, int d>
  class Tupel {
  public:
    //! make empty tupel
    Tupel() {}

    //! read/write components
    int& operator[] (int i) {return x[i];}

  private:
    int x[d];
  };

  //! generate lexicographic ordering in a cube of dimension dim with arbitry size per direction
  template<int dim>
  class LexOrder {
  public:
    //! preprocess ordering
    void init (Tupel<int,dim>& _N);

    //! get total number of tupels
    int tupels ();

    //! compute number from a given tupel
    int n (Tupel<int,dim>& z);

    //! compute tupel from number 0 <= n < tupels()
    Tupel<int,dim> z (int n);

  private:
    Tupel<int,dim> N;     // number of elements per direction
    int P[dim+1];         // P[i] = Prod_{i=0}^{i} N[i];
  };

  //! generate consecutive numbering of dim sets of size N_i
  template<int dim>
  class JoinOrder {
  public:
    //! preprocess ordering
    void init (Tupel<int,dim>& _N);

    //! get total number of elements in all sets
    int size ();

    //! compute number from subset and index
    int n (int subset, int index);

    //! compute subset from number
    int subset (int n);

    //! compute index in subset from number
    int index (int n);

  private:
    Tupel<int,dim> N;       // number of elements per direction
    int offset[dim+1];      // P[i] = Sum_{i=0}^{i} N[i];
  };

  //! associate a unique consecutive index to all entities of a given codimension in a d-dimension cube
  template<int dim>
  class CubeMapper {
  public:
    //! construct with number of elements (of codim 0) in each direction
    CubeMapper (Tupel<int,dim>& _N);

    //! get number of elements in each codimension
    int elements (int codim);

    //! compute codim from coordinate
    int codim (Tupel<int,dim>& z);

    /*! compute number from coordinate 0 <= n < elements(codim(z))
         general implementation is O(2^dim)
     */
    int n (Tupel<int,dim>& z);

    //! compute coordinates from number and codimension
    Tupel<int,dim> z (int i, int codim);

  private:
    Tupel<int,dim> N;     // number of elements per direction
    int ne[dim+1];        // number of elements per codimension
    int nb[1<<dim];       // number of elements per binary partition
    int cb[1<<dim];       // codimension of binary partition
    LexOrder<dim> lex[1<<dim];         // lex ordering within binary partition
    JoinOrder<1<<dim> join[dim+1];     // join subsets of codimension

    int power2 (int i) {return 1<<i;}
    int ones (int b);     // count number of bits set in binary rep of b
    int partition (Tupel<int,dim>& z);     // get binary representation of partition
    Tupel<int,dim> reduce (Tupel<int,dim>& z);
    Tupel<int,dim> expand (Tupel<int,dim>& r, int b);
  };

} // end namespace

#include "numbering.cc"

#endif
