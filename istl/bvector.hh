// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BVECTOR_HH__
#define __DUNE_BVECTOR_HH__

#include <math.h>
#include <complex>

#include "../common/exceptions.hh"

#include "spmv.hh"

/*! \file __FILE__

   This file implements a vector space as a tensor product of
   a given vector space. The number of components can be given at
   run-time.
 */

namespace Dune {

  /** @defgroup SPMV Dune Sparse Matrix Vector Templates
              @addtogroup SPMV
              @{
   */

  /**! Construct a vector space out of a tensor product of other
           vector spaces. The number of components is given at run-time

           Vector has a window mode where it is assumed that the
       object provides a window into a larger vector. Dynamic
           memory management is disabled in this case. The window
           is manipulated with the setwindow function.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=SPMV_Allocator>
  class BlockVector
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


    //===== constructors and such

    //! makes empty vector in non-window mode
    BlockVector ()
    {
      n = 0;
      p = 0;
      wmode = false;
    }

    //! makes empty vector and selects mode
    BlockVector (bool _wmode)     // this is the only way to make a vector in window mode
    {
      n = 0;
      p = 0;
      wmode = _wmode;
    }

    //! make vector with _n components
    BlockVector (int _n)
    {
      wmode = false;     // this is non-window mode
      n = _n;
      if (n>0)
        p = A::template malloc<B>(n);
      else
      {
        n = 0;
        p = 0;
      }
    }

    //! copy constructor
    BlockVector (const BlockVector& a)
    {
      wmode = false;     // this is non-window mode

      // allocate memory with same size as a
      n = a.n;
      if (n>0)
        p = A::template malloc<B>(n);
      else
      {
        n = 0;
        p = 0;
      }

      // und kopiere Elemente
      for (int i=0; i<n; i++) p[i]=a.p[i];
    }

    //! free dynamic memory
    ~BlockVector ()
    {
      if (n>0 && !wmode) A::template free<B>(p);
    }

    //! reallocate vector to given size, any data is lost
    void resize (int _n)
    {
      if (n==_n) return;

#ifdef DUNE_ISTL_WITH_CHECKING
      if (wmode) DUNE_THROW(ISTLError,"no resize in window mode possible");
#endif

      if (n>0) A::template free<B>(p);
      n = _n;
      if (n>0)
        p = A::template malloc<B>(n);
      else
      {
        n = 0;
        p = 0;
      }
    }

    //! assignment
    BlockVector& operator= (const BlockVector& a)
    {
      if (&a!=this)     // check if this and a are different objects
      {
        // adjust size of array
        if (n!=a.n)           // check if size is different
        {

#ifdef DUNE_ISTL_WITH_CHECKING
          if (wmode) DUNE_THROW(ISTLError,"no resizing in window mode possible");
#endif

          if (n>0) A::template free<B>(p);                 // delete old memory
          n = a.n;
          if (n>0)
            p = A::template malloc<B>(n);
          else
          {
            n = 0;
            p = 0;
          }
        }
        // copy data
        for (int i=0; i<n; i++) p[i]=a.p[i];
      }
      return *this;
    }

    //===== assignment from scalar
    BlockVector& operator= (const field_type& k)
    {
      for (int i=0; i<n; i++)
        p[i] = k;
      return *this;
    }

    //===== access to components

    //! random access to blocks
    B& operator[] (int i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const B& operator[] (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! random access via round brackets supplied to be consistent with matrix
    B& operator() (int i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const B& operator() (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! Iterator class for sequential access
    class Iterator
    {
    public:
      //! constructor
      Iterator (B* _p)
      {
        p = _p;
      }

      //! prefix increment
      Iterator& operator++()
      {
        ++p;
        return *this;
      }

      //! equality
      bool operator== (const Iterator& i) const
      {
        return p==i.p;
      }

      //! inequality
      bool operator!= (const Iterator& i) const
      {
        return p!=i.p;
      }

      //! dereferencing
      B& operator* ()
      {
        return *p;
      }

      //! arrow
      B* operator-> ()
      {
        return p;
      }

    private:
      B* _p;
    };

    //! begin iterator
    Iterator begin () const
    {
      return Iterator(p);
    }

    //! end iterator
    Iterator end () const
    {
      return Iterator(p+n);
    }


    //===== vector space arithmetic

    //! vector space addition
    BlockVector& operator+= (const BlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<n; ++i) p[i] += y.p[i];
      return *this;
    }

    //! vector space subtraction
    BlockVector& operator-= (const BlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<n; ++i) p[i] -= y.p[i];
      return *this;
    }

    //! vector space multiplication with scalar
    BlockVector& operator*= (const field_type& k)
    {
      for (int i=0; i<n; ++i) p[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    BlockVector& operator/= (const field_type& k)
    {
      for (int i=0; i<n; ++i) p[i] /= k;
      return *this;
    }

    //! vector space axpy operation
    BlockVector& axpy (const field_type& a, const BlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<n; ++i) p[i].axpy(a,y.p[i]);
      return *this;
    }


    //===== Euclidean scalar product

    //! scalar product
    field_type operator* (const BlockVector& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      field_type sum=0;
      for (int i=0; i<n; ++i) sum += p[i]*y.p[i];
      return sum;
    }


    //===== norms

    //! one norm (sum over absolute values of entries)
    double one_norm () const
    {
      double sum=0;
      for (int i=0; i<n; ++i) sum += p[i].one_norm();
      return sum;
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    double one_norm_real () const
    {
      double sum=0;
      for (int i=0; i<n; ++i) sum += p[i].one_norm_real();
      return sum;
    }

    //! two norm sqrt(sum over squared values of entries)
    double two_norm () const
    {
      double sum=0;
      for (int i=0; i<n; ++i) sum += p[i].two_norm2();
      return sqrt(sum);
    }

    //! sqare of two norm (sum over squared values of entries), need for block recursion
    double two_norm2 () const
    {
      double sum=0;
      for (int i=0; i<n; ++i) sum += p[i].two_norm2();
      return sum;
    }

    //! infinity norm (maximum of absolute values of entries)
    double infinity_norm () const
    {
      double max=0;
      for (int i=0; i<n; ++i) max = std::max(max,p[i].infinity_norm());
      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      double max=0;
      for (int i=0; i<n; ++i) max = std::max(max,p[i].infinity_norm_real());
      return max;
    }


    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    int N () const
    {
      return n;
    }

    //! dimension of the vector space
    int dim () const
    {
      int d=0;
      for (int i=0; i<n; i++)
        d += p[i].dim();
      return d;
    }


    //===== in window mode provide a function to set the window

    //! set the window in window mode -- you must know what you do!
    void set (const int& _n, B* _p)
    {
      n = _n;
      p = _p;
    }

  private:
    bool wmode;     // true if object provides window in larger vector
    int n;          // number of elements in array
    B *p;           // pointer to dynamically allocated built-in array
  };



  /** @} end documentation */

} // end namespace

#endif
