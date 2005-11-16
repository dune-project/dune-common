// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BLOCK_DIAGONAL_MATRIX_HH
#define DUNE_BLOCK_DIAGONAL_MATRIX_HH

#include <dune/istl/bcrsmatrix.hh>

/** \file
    \author Oliver Sander
    \brief Implementation of the BDMatrix class
 */

namespace Dune {

  /** \brief A block-diagonal matrix

     \todo It would be safer and more efficient to have a real implementation of
     a block-diagonal matrix and not just subclassing from BCRSMatrix.  But that's
     quite a lot of work for that little advantage.*/
  template <class B, class A=ISTLAllocator>
  class BDMatrix : public BCRSMatrix<B,A>
  {
  public:

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename B::field_type field_type;

    //! export the type representing the components
    typedef B block_type;

    //! export the allocator type
    typedef A allocator_type;

    //! implement row_type with compressed vector
    //typedef BCRSMatrix<B,A>::row_type row_type;

    //! The type for the index access and the size
    typedef typename A::size_type size_type;

    //! increment block level counter
    enum {blocklevel = B::blocklevel+1};

    /** \brief Default constructor */
    BDMatrix() : BCRSMatrix<B,A>() {}

    explicit BDMatrix(int size)
      : BCRSMatrix<B,A>(size, size, BCRSMatrix<B,A>::random) {

      for (int i=0; i<size; i++)
        this->BCRSMatrix<B,A>::setrowsize(i, 1);

      this->BCRSMatrix<B,A>::endrowsizes();

      for (int i=0; i<size; i++)
        this->BCRSMatrix<B,A>::addindex(i, i);

      this->BCRSMatrix<B,A>::endindices();

    }

    //! assignment
    BDMatrix& operator= (const BDMatrix& other) {
      this->BCRSMatrix<B,A>::operator=(other);
      return *this;
    }

    //! assignment from scalar
    BDMatrix& operator= (const field_type& k) {
      this->BCRSMatrix<B,A>::operator=(k);
      return *this;
    }

    /** \brief Inverts the matrix */
    void invert() {
      for (int i=0; i<this->N(); i++)
        (*this)[i][i].invert();
    }

  private:

    // ////////////////////////////////////////////////////////////////////////////
    //   The following methods from the base class should now actually be called
    // ////////////////////////////////////////////////////////////////////////////

    // createbegin and createend should be in there, too, but I can't get it to compile
    //     BCRSMatrix<B,A>::CreateIterator createbegin () {}
    //     BCRSMatrix<B,A>::CreateIterator createend () {}
    void setrowsize (size_type i, size_type s) {}
    void incrementrowsize (size_type i) {}
    void endrowsizes () {}
    void addindex (size_type row, size_type col) {}
    void endindices () {}
  };


}  // end namespace Dune

#endif
