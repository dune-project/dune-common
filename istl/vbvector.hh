// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_VBVECTOR_HH__
#define __DUNE_VBVECTOR_HH__

#include <math.h>
#include <complex>
#include <iostream>

#include "../common/exceptions.hh"

#include "istl.hh"
#include "bvector.hh"

/*! \file __FILE__

   This file implements a vector space as a tensor product of
   a given vector space. The number of components can be given at
   run-time as well as the size of the blocks.
 */

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */

  /**! Construct a vector space out of a tensor product of other
           vector spaces. The number of components is given at run-time.

   */
  template<class B, class A=ISTL_Allocator>
  class VariableBlockVector
  {
  public:

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename B::field_type field_type;

    //! export the allocator type
    typedef A allocator_type;

    //! export the type representing the components
    typedef BlockVector<B> block_type;

    //! increment block level counter, yes, it is two levels !
    enum {blocklevel = B::blocklevel+2};


    //===== constructors and such

    //! constructor without arguments makes empty vector, put subobject in window mode
    VariableBlockVector () : window(true)
    {
      // nothing is known ...
      nblocks = 0;
      s = 0;
      e = 0;
      n = 0;
      p = 0;
      initialized = false;
    }

    //! make vector with _n components, put subobject in window mode
    VariableBlockVector (int _nblocks) : window(true)
    {
      // now the s and e array can be allocated
      nblocks = _nblocks;
      if (nblocks>0)
      {
        s = A::template malloc<int>(nblocks);
        e = A::template malloc<int>(nblocks);
      }
      else
      {
        nblocks = 0;
        s = 0;
        e = 0;
      }

      // the p array is still not allocated
      n = 0;
      p = 0;

      // and the vector not fully usable
      initialized = false;
    }

    //! copy constructor, put subobject in window mode
    VariableBlockVector (const VariableBlockVector& a) : window(true)
    {
      // allocate s and e arrays
      nblocks = a.nblocks;
      if (nblocks>0)
      {
        s = A::template malloc<int>(nblocks);
        e = A::template malloc<int>(nblocks);
      }
      else
      {
        nblocks = 0;
        s = 0;
        e = 0;
      }

      // allocate p array
      n = a.n;
      if (n>0)
        p = A::template malloc<B>(n);
      else
      {
        n = 0;
        p = 0;
      }

      // and copy the data
      for (int i=0; i<nblocks; i++)
      {
        s[i]=a.s[i];
        e[i]=a.e[i];
      }
      for (int i=0; i<n; i++) p[i]=a.p[i];

      // and we have a usable vector
      initialized = true;
    }

    //! free dynamic memory in reverse order
    ~VariableBlockVector ()
    {
      if (n>0) A::template free<B>(p);
      if (nblocks>0)
      {
        A::template free<int>(e);
        A::template free<int>(s);
      }
    }

    //! same effect as constructor with same argument
    void resize (int _nblocks)
    {
      // deallocate in reverse order
      if (n>0) A::template free<B>(p);
      if (nblocks>0)
      {
        A::template free<int>(e);
        A::template free<int>(s);
      }

      // now the s and e array can be allocated
      nblocks = _nblocks;
      if (nblocks>0)
      {
        s = A::template malloc<int>(nblocks);
        e = A::template malloc<int>(nblocks);
      }
      else
      {
        nblocks = 0;
        s = 0;
        e = 0;
      }

      // the p array is still not allocated
      n = 0;
      p = 0;

      // and the vector not fully usable
      initialized = false;
    }

    //! assignment
    VariableBlockVector& operator= (const VariableBlockVector& a)
    {
      if (&a!=this)     // check if this and a are different objects
      {
        // reallocate arrays if necessary
        if (n!=a.n || nblocks!=a.nblocks)
        {
          if (n>0) A::template free<B>(p);
          if (nblocks>0)
          {
            A::template free<int>(e);
            A::template free<int>(s);
          }
          nblocks = a.nblocks;
          if (nblocks>0)
          {
            s = A::template malloc<int>(nblocks);
            e = A::template malloc<int>(nblocks);
          }
          else
          {
            nblocks = 0;
            s = 0;
            e = 0;
          }
          n = a.n;
          if (n>0)
            p = A::template malloc<B>(n);
          else
          {
            n = 0;
            p = 0;
          }
        }

        // and copy the data
        for (int i=0; i<nblocks; i++)
        {
          s[i]=a.s[i];
          e[i]=a.e[i];
        }
        for (int i=0; i<n; i++) p[i]=a.p[i];
      }

      // and we have a usable vector
      initialized = true;

      return *this;     // Gebe Referenz zurueck damit a=b=c; klappt
    }


    //===== the creation interface

    //! Iterator class for sequential creation of blocks
    class CreateIterator
    {
    public:
      //! constructor
      CreateIterator (VariableBlockVector& _v, int _i) : v(_v)
      {
        i = _i;
        k = 0;
      }

      //! prefix increment
      CreateIterator& operator++()
      {
        // set the blocks size
        v.s[i] = k;

        // accumulate total size
        v.n += k;

        // compute entry into big array
        if (i==0)
          v.e[i] = 0;
        else
          v.e[i] = v.e[i-1]+v.s[i-1];

        //              std::cout << "block " << i
        //                                << " size " << v.s[i]
        //                                << " total " << v.n << std::endl;

        // go to next block
        ++i;

        // if we are past the last block, finish off
        if (i==v.nblocks)
        {
          // allocate p array
          if (v.n>0)
            v.p = A::template malloc<B>(v.n);
          else
          {
            v.n = 0;
            v.p = 0;
          }

          // and the vector is ready
          v.initialized = true;

          //			std::cout << "made vbvector with " << v.n << " components" << std::endl;
        }
        return *this;
      }

      //! inequality
      bool operator!= (const CreateIterator& it) const
      {
        return (i!=it.i) || (&v!=&it.v);
      }

      //! equality
      bool operator== (const CreateIterator& it) const
      {
        return (i==it.i) && (&v==&it.v);
      }

      //! dereferencing
      int index ()
      {
        return i;
      }

      //! set size of current block
      void setblocksize (int _k)
      {
        k = _k;
      }

    private:
      VariableBlockVector& v;     // my vector
      int i;                      // current block to be defined
      int k;     // block size
    };

    // CreateIterator wants to set all the arrays ...
    friend class CreateIterator;

    //! get initial create iterator
    CreateIterator createbegin ()
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (initialized) DUNE_THROW(ISTLError,"no CreateIterator in initialized state");
#endif
      n = 0;     // clear total counter
      return CreateIterator(*this,0);
    }

    //! get create iterator pointing to one after the last block
    CreateIterator createend ()
    {
      return CreateIterator(*this,nblocks);
    }

    //===== assignment from scalar
    VariableBlockVector& operator= (const field_type& k)
    {
      for (int i=0; i<n; i++)
        p[i] = k;
      return *this;
    }

    //===== access to components

    //! random access to blocks
    block_type& operator[] (int i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=nblocks) DUNE_THROW(ISTLError,"index out of range");
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
#endif
      window.set(s[i],p+e[i]);
      return window;
    }

    //! same for read only access
    const block_type& operator[] (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=nblocks) DUNE_THROW(ISTLError,"index out of range");
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
#endif
      window.set(s[i],p+e[i]);
      return window;
    }

    //! Iterator class for sequential access
    class Iterator
    {
    public:
      //! constructor
      Iterator (const VariableBlockVector& _v, int _i, B* _p) : v(_v),window(_p,_v.s[0])
      {
        i = _i;
      }

      //! prefix increment
      Iterator& operator++()
      {
        ++i;
        window.advance(v.s[i]);
        return *this;
      }

      //! equality
      bool operator== (const Iterator& it) const
      {
        return window.getptr()==it.window.getptr();
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        return (window.getptr())!=(it.window.getptr());
      }

      //! dereferencing
      block_type& operator* ()
      {
        return window;
      }

      //! arrow
      block_type* operator-> ()
      {
        return &window;
      }

      //! return block index
      int index ()
      {
        return i;
      }

    private:
      mutable block_type window;     // provides a window into the vector
      int i;
      const VariableBlockVector& v;
    };

    // Iterator wants to see all the arrays ...
    friend class Iterator;

    //! begin iterator
    Iterator begin () const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
#endif
      return Iterator(*this,0,p);
    }

    //! end iterator
    Iterator end () const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
#endif
      return Iterator(*this,nblocks,p+n);
    }


    //===== vector space arithmetic

    //! vector space addition
    VariableBlockVector& operator+= (const VariableBlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<n; ++i) p[i] += y.p[i];
      return *this;
    }

    //! vector space subtraction
    VariableBlockVector& operator-= (const VariableBlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<n; ++i) p[i] -= y.p[i];
      return *this;
    }

    //! vector space multiplication with scalar
    VariableBlockVector& operator*= (const field_type& k)
    {
      for (int i=0; i<n; ++i) p[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    VariableBlockVector& operator/= (const field_type& k)
    {
      for (int i=0; i<n; ++i) p[i] /= k;
      return *this;
    }

    //! vector space axpy operation
    VariableBlockVector& axpy (const field_type& a, const VariableBlockVector& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
      if (n!=y.n) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<n; ++i) p[i].axpy(a,y.p[i]);
      return *this;
    }


    //===== Euclidean scalar product

    //! scalar product
    field_type operator* (const VariableBlockVector& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!initialized) DUNE_THROW(ISTLError,"tried to access uninitialized vector");
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

    //! number of blocks in the vector (are of variable size here)
    int N () const
    {
      return nblocks;
    }

    //! dimension of the vector space
    int dim () const
    {
      int d=0;
      for (int i=0; i<n; i++)
        d += p[i].dim();
      return d;
    }


  private:
    int nblocks;    // number of blocks in vector
    int* s;         // s[nblocks] gives number of elements in each block
    int* e;         // e[nblocks] says that p[e[i]] is the first element of block i
    int n;          // number of elements in p array
    B *p;           // pointer to dynamically allocated built-in array
    mutable block_type window;     // provides a window into the vector
    bool initialized;              // true if vector has been initialized
  };



  /** @} end documentation */

} // end namespace

#endif
