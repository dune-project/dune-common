// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_NUMBERING_HH__
#define __DUNE_NUMBERING_HH__

#include <dune/common/fixedarray.hh>

namespace Dune {

  //! generate lexicographic ordering in a cube of dimension dim with arbitry size per direction
  template<int dim>
  class LexOrder {
  public:
    //! preprocess ordering
    void init (const FixedArray<int, dim>& _NN);

    //! get total number of tupels
    int tupels () const;

    //! compute number from a given tupel
    int n (const FixedArray<int,dim>& z) const;

    //! compute tupel from number 0 <= n < tupels()
    FixedArray<int,dim> z (int n) const;

  private:
    FixedArray<int,dim> N; // number of elements per direction
    int P[dim+1];     // P[i] = Prod_{i=0}^{i} N[i];
  };

  //! generate consecutive numbering of dim sets of size N_i
  template<int dim>
  class JoinOrder {
  public:
    //! preprocess ordering
    void init (const FixedArray<int,dim>& _NN);

    //! get total number of elements in all sets
    int size () const;

    //! compute number from subset and index
    int n (int subset, int index) const;

    //! compute subset from number
    int subset (int n) const;

    //! compute index in subset from number
    int index (int n) const;

  private:
    FixedArray<int,dim> N;       // number of elements per direction
    int offset[dim+1];      // P[i] = Sum_{i=0}^{i} N[i];
  };


  /*! The CubeMapper assigns an id to all entities of all codimensions of a structured mesh
     with arbitrary number of elements (codim 0 entities) in each direction. The ids
     are unique and consecutive within each codimension.

     The idea is as follows: Consider a structured mesh in \f$d\f$ dimensions with \f$N\f$ elements per
     direction. This mesh has \f$N^d\f$ elements in total. Now imagine refined mesh where each element
     is halfened in every coordinate direction. This refined mesh has \f$(2N+1)^d\f$ vertices (entities
     of codimension \f$d\f$). Each vertex of the refined mesh now corresponds to a grid entity of the
     original mesh. Moreover, a vertex in the refined mesh can be identified by integer coordintes \f$z\f$
     where \f$z_i\in\{0,\ldots,2N\}, 0\leq i < d\f$. Let \f$c(z)\f$ be the number of even components in \f$z\f$.
     Then, \f$c(z)\f$ is the codimension of the mesh entity with coordinate \f$z\f$. E.~g.~
     entities of codimension 0 have odd coordinates, all entities of codim \f$d\f$ have \f$d\f$
     even coordinates.

     In order to number all entities of one codimension consecutively we observe that the
     refined mesh can be subdivided into \f$2^d\f$subsets. Subset number \f$b\f$ with
     binary representation \f$(b_{d-1},\ldots,b_0)\f$ corresponds to all \f$z\in [0,2N]^d\f$ where
     \f$z_i\f$ is even if \f$b_i\f$ is 1 and \f$z_i\f$ is odd if \f$b_i\f$ is 0. The entities of codimension
     \f$c\f$ now consist of \f$\left ( \begin{array}{cc}d\\c\end{array} \right)\f$ of those
     subsets. Within the subsets the numbering is lexicographic and then the corrsponding
     subsets are numbered consecutively.
   */
  template<int dim>
  class CubeMapper {
  public:
    //! construct with number of elements (of codim 0) in each direction
    CubeMapper (const FixedArray<int,dim>& _NN);

    //! make cube of single element
    CubeMapper ();

    //! (re)initialize with number of elements (of codim 0) in each direction
    void make (const FixedArray<int,dim>& _NN);

    //! get number of elements in each codimension
    int elements (int codim) const;

    //! compute codim from coordinate
    int codim (const FixedArray<int,dim>& z) const;

    /*! compute number from coordinate 0 <= n < elements(codim(z))
         general implementation is O(2^dim)
     */
    int n (const FixedArray<int,dim>& z) const;

    //! compute coordinates from number and codimension
    FixedArray<int,dim> z (int i, int codim) const;

    //! compress from expanded coordinates to grid for a single partition number
    FixedArray<int,dim> compress (const FixedArray<int,dim>& z) const;

    //! expand with respect to partition number
    FixedArray<int,dim> expand (const FixedArray<int,dim>& r, int b) const;

    //! There are \f$2^d\f$ possibilities of having even/odd coordinates. The binary representation is called partition number
    int partition (const FixedArray<int,dim>& z) const;

    //! print internal data
    void print (std::ostream& ss, int indent) const;

  private:
    FixedArray<int,dim> N;     // number of elements per direction
    int ne[dim+1];        // number of elements per codimension
    int nb[1<<dim];       // number of elements per binary partition
    int cb[1<<dim];       // codimension of binary partition
    LexOrder<dim> lex[1<<dim];         // lex ordering within binary partition
    JoinOrder<1<<dim> join[dim+1];     // join subsets of codimension

    int power2 (int i) const {return 1<<i;}
    int ones (int b) const;     // count number of bits set in binary rep of b
  };

} // end namespace

#include "numbering.cc"

#endif
