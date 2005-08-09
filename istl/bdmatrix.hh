// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BLOCK_DIAGONAL_MATRIX_HH
#define DUNE_BLOCK_DIAGONAL_MATRIX_HH

#include <dune/istl/bvector.hh>

/** \file
    \brief Implementation of the BDMatrix class
 */

namespace Dune {

  /** \brief A block-diagonal matrix */
  template <class B, class A=ISTLAllocator>
  class BDMatrix
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
    //typedef CompressedBlockVectorWindow<B,A> row_type;

    //! increment block level counter
    enum {blocklevel = B::blocklevel+1};


    /** \brief Change the size of the matrix */
    void resize(int n) {data_.resize(n);}

    /** \brief Number of rows of the matrix */
    int N() const {return data_.size();}

    /** \brief Number of columns of the matrix */
    int M() const {return data_.size();}

    /** \brief Get the i-th diagonal element */
    B& operator[](int i) {return data_[i];}

    /** \brief y += Ax */
    template <class X, class Y>
    void umv(const X& x, Y& y) const {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      for (int i=0; i<data_.size(); i++)
        data_[i].umv(x[i], y[i]);
    }

    /** \brief y -= Ax */
    template <class X, class Y>
    void mmv(const X& x, Y& y) const {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      for (int i=0; i<data_.size(); i++)
        data_[i].mmv(x[i], y[i]);
    }

    /** \brief Inverts the matrix */
    void invert() {
      for (int i=0; i<data_.size(); i++)
        data_[i].invert();
    }

  private:

    BlockVector<B> data_;
  };


}  // end namespace Dune

#endif
