// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BVECTOR_HH__
#define __DUNE_BVECTOR_HH__

#include <math.h>
#include <complex>

#include "../common/exceptions.hh"

#include "istl.hh"
#include "basearray.hh"

/*! \file __FILE__

   This file implements a vector space as a tensor product of
   a given vector space. The number of components can be given at
   run-time.
 */

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  /**! Construct a vector space out of a tensor product of other
           vector spaces. The number of components is given at run-time

           Vector is derived from the base_array container.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class BlockVector : public base_array<B,A>
  {
  public:

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename B::field_type field_type;

    //! export the type representing the components
    typedef B block_type;

    //! export the allocator type
    typedef A allocator_type;

    //! increment block level counter
    enum {blocklevel = B::blocklevel+1};

    //! make iterators available as types
    typedef typename base_array<B,A>::iterator Iterator;

    //! make iterators available as types
    typedef typename base_array<B,A>::const_iterator ConstIterator;


    //===== constructors and such

    //! makes empty vector in non-window mode
    BlockVector () : base_array<B,A>()
    {       }

    //! makes empty vector in non-window mode
    BlockVector (bool _wmode) : base_array<B,A>(_wmode)
    {       }

    //! make vector from array allocated outside ("window mode")
    BlockVector (B* _p, int _n) : base_array<B,A>(_p,_n)
    {       }

    //! make vector with _n components
    BlockVector (int _n) : base_array<B,A>(_n)
    {       }


    //===== assignment from scalar

    BlockVector& operator= (const field_type& k)
    {
      for (int i=0; i<base_array<B,A>::size(); i++)
        (*this)[i] = k;
      return *this;
    }


    //===== vector space arithmetic

    //! vector space addition
    BlockVector& operator+= (const BlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (base_array<B,A>::size()!=y.size()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<base_array<B,A>::size(); ++i) (*this)[i] += y[i];
      return *this;
    }

    //! vector space subtraction
    BlockVector& operator-= (const BlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (base_array<B,A>::size()!=y.size()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<base_array<B,A>::size(); ++i) (*this)[i] -= y[i];
      return *this;
    }

    //! vector space multiplication with scalar
    BlockVector& operator*= (const field_type& k)
    {
      for (int i=0; i<base_array<B,A>::size(); ++i) (*this)[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    BlockVector& operator/= (const field_type& k)
    {
      for (int i=0; i<base_array<B,A>::size(); ++i) (*this)[i] /= k;
      return *this;
    }

    //! vector space axpy operation
    BlockVector& axpy (const field_type& a, const BlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (base_array<B,A>::size()!=y.size()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<base_array<B,A>::size(); ++i) (*this)[i].axpy(a,y[i]);
      return *this;
    }


    //===== Euclidean scalar product

    //! scalar product
    field_type operator* (const BlockVector& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (base_array<B,A>::size()!=y.size()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      field_type sum=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) sum += (*this)[i]*y[i];
      return sum;
    }


    //===== norms

    //! one norm (sum over absolute values of entries)
    double one_norm () const
    {
      double sum=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) sum += (*this)[i].one_norm();
      return sum;
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    double one_norm_real () const
    {
      double sum=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) sum += (*this)[i].one_norm_real();
      return sum;
    }

    //! two norm sqrt(sum over squared values of entries)
    double two_norm () const
    {
      double sum=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) sum += (*this)[i].two_norm2();
      return sqrt(sum);
    }

    //! sqare of two norm (sum over squared values of entries), need for block recursion
    double two_norm2 () const
    {
      double sum=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) sum += (*this)[i].two_norm2();
      return sum;
    }

    //! infinity norm (maximum of absolute values of entries)
    double infinity_norm () const
    {
      double max=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) max = std::max(max,(*this)[i].infinity_norm());
      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      double max=0;
      for (int i=0; i<base_array<B,A>::size(); ++i) max = std::max(max,(*this)[i].infinity_norm_real());
      return max;
    }


    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    int N () const
    {
      return base_array<B,A>::size();
    }

    //! dimension of the vector space
    int dim () const
    {
      int d=0;
      for (int i=0; i<base_array<B,A>::size(); i++)
        d += (*this)[i].dim();
      return d;
    }

  };



  /** @} end documentation */

} // end namespace

#endif
