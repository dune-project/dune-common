// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BASEARRAY_HH__
#define __DUNE_BASEARRAY_HH__

#include <math.h>
#include <complex>

#include "../common/exceptions.hh"

#include "istl.hh"

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

  /**! A simple array container for objects of type B with
       dynamic allocation, copy semantics, iterator and random access.

           The so-called vector mode disables dynamic memory management
       and assumes that memory for the array is managed outside this class.
       This is used to provide access to a part of a larger array.

           You can make a copy of such a window which is then an object on
       its own.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class base_array
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;


    //===== constructors and such

    //! makes empty vector in non-window mode
    base_array ()
    {
      n = 0;
      p = 0;
      wmode = false;
    }

    //! makes empty vector where you can switch on window mode
    base_array (bool _wmode)
    {
      n = 0;
      p = 0;
      wmode = _wmode;
    }

    //! make vector from array allocated outside ("window mode")
    base_array (B* _p, int _n)
    {
      n = _n;
      p = _p;
      wmode = true;
    }

    //! make vector with _n components
    base_array (int _n)
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
    base_array (const base_array& a)
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

      // and copy elements
      for (int i=0; i<n; i++) p[i]=a.p[i];
    }

    //! free dynamic memory
    ~base_array ()
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
    base_array& operator= (const base_array& a)
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

    // forward declaration
    class const_iterator;

    //! iterator class for sequential access
    class iterator
    {
    public:
      //! constructor
      iterator (B* _p) : p(_p)
      {       }

      //! prefix increment
      iterator& operator++()
      {
        ++p;
        return *this;
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        return p==it.p;
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        return p!=it.p;
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        return p==it.p;
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        return p!=it.p;
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

      friend class const_iterator;

    private:
      B* p;
      int i;
    };

    //! begin iterator
    iterator begin ()
    {
      return iterator(p);
    }

    //! end iterator
    iterator end ()
    {
      return iterator(p+n);
    }

    //! const_iterator class for sequential access
    class const_iterator
    {
    public:
      //! constructor
      const_iterator (B* _p) : p(_p)
      {       }

      const_iterator (const iterator& it) : p(it.p)
      {       }

      //! prefix increment
      const_iterator& operator++()
      {
        ++p;
        return *this;
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        return p==it.p;
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        return p!=it.p;
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        return p==it.p;
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        return p!=it.p;
      }

      //! dereferencing
      const B& operator* ()
      {
        return *p;
      }

      //! arrow
      const B* operator-> ()
      {
        return p;
      }

      friend class iterator;

    private:
      B* p;
    };

    //! begin const_iterator
    const_iterator begin () const
    {
      return const_iterator(p);
    }

    //! end const_iterator
    const_iterator end () const
    {
      return const_iterator(p+n);
    }


    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    int size () const
    {
      return n;
    }

    //===== in window mode provide a function to set the window
    // with these functions you should know what you do ...

    void set (int _n, B* _p)
    {
      n = _n;
      p = _p;
    }

    void advance (int newsize)
    {
      p += n;
      n = newsize;
    }

    void move (int offset, int newsize)
    {
      p += offset;
      n = newsize;
    }

    void move (int offset)
    {
      p += offset;
    }

    B* getptr ()
    {
      return p;
    }

  private:
    bool wmode;     // true if object provides window in larger vector
    int n;          // number of elements in array
    mutable B *p;     // pointer to dynamically allocated built-in array
  };


  /** @} end documentation */

} // end namespace

#endif
