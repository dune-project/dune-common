// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BASEARRAY_HH__
#define __DUNE_BASEARRAY_HH__

#include <math.h>
#include <complex>

#include "istlexception.hh"
#include "allocator.hh"

/*! \file

   This file implements several basic array containers.
 */

namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */

  /**  A simple array container for objects of type B implementing

       -  iterator access
       -  const_iterator access
       -  random access

           This container has *NO* memory management at all,
           copy constuctor, assignment and destructor are all default.

           The constructor is made protected to emphasize that objects
       are only usably in derived classes.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class base_array_unmanaged
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;


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
      iterator ()
      {
        p = 0;
        i = 0;
      }

      iterator (B* _p, int _i) : p(_p), i(_i)
      {       }

      //! prefix increment
      iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      B& operator* ()
      {
        return p[i];
      }

      //! arrow
      B* operator-> ()
      {
        return p+i;
      }

      // return index corresponding to pointer
      int index () const
      {
        return i;
      }

      friend class const_iterator;

    private:
      B* p;
      int i;
    };

    //! begin iterator
    iterator begin ()
    {
      return iterator(p,0);
    }

    //! end iterator
    iterator end ()
    {
      return iterator(p,n);
    }

    //! random access returning iterator (end if not contained)
    iterator find (int i)
    {
      if (i>=0 && i<n)
        return iterator(p,i);
      else
        return iterator(p,n);
    }

    //! const_iterator class for sequential access
    class const_iterator
    {
    public:
      //! constructor
      const_iterator ()
      {
        p = 0;
        i = 0;
      }

      const_iterator (const B* _p, int _i) : p(_p), i(_i)
      {       }

      const_iterator (const iterator& it) : p(it.p), i(it.i)
      {       }

      //! prefix increment
      const_iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      const B& operator* () const
      {
        return p[i];
      }

      //! arrow
      const B* operator-> () const
      {
        return p+i;
      }

      // return index corresponding to pointer
      int index () const
      {
        return i;
      }

      friend class iterator;

    private:
      const B* p;
      int i;
    };

    //! begin const_iterator
    const_iterator begin () const
    {
      return const_iterator(p,0);
    }

    //! end const_iterator
    const_iterator end () const
    {
      return const_iterator(p,n);
    }


    //===== sizes

    //! number of blocks in the array (are of size 1 here)
    int size () const
    {
      return n;
    }

  protected:
    //! makes empty array
    base_array_unmanaged ()
    {
      n = 0;
      p = 0;
    }

    int n;     // number of elements in array
    B *p;      // pointer to dynamically allocated built-in array
  };



  /**  Extend base_array_unmanaged by functions to manipulate

           This container has *NO* memory management at all,
           copy constuctor, assignment and destructor are all default.
           A container can be constructed as empty or from a given pointer
       and size. This class is used to implement a view into a larger
       array.

           You can copy such an object to a base_array to make a real copy.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class base_array_window : public base_array_unmanaged<B,A>
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;

    //! make iterators available as types
    typedef typename base_array_unmanaged<B,A>::iterator iterator;

    //! make iterators available as types
    typedef typename base_array_unmanaged<B,A>::const_iterator const_iterator;


    //===== constructors and such

    //! makes empty array
    base_array_window ()
    {
      this->n = 0;
      this->p = 0;
    }

    //! make array from given pointer and size
    base_array_window (B* _p, int _n)
    {
      this->n = _n;
      this->p = _p;
    }

    //===== window manipulation methods

    //! set pointer and length
    void set (int _n, B* _p)
    {
      this->n = _n;
      this->p = _p;
    }

    //! advance pointer by newsize elements and then set size to new size
    void advance (int newsize)
    {
      this->p += this->n;
      this->n = newsize;
    }

    //! increment pointer by offset and set size
    void move (int offset, int newsize)
    {
      this->p += offset;
      this->n = newsize;
    }

    //! increment pointer by offset, leave size
    void move (int offset)
    {
      this->p += offset;
    }

    //! return the pointer
    B* getptr ()
    {
      return this->p;
    }
  };



  /**   This container extends base_array_unmanaged by memory management
        with the usual copy semantics providing the full range of
        copy constructor, destructor and assignement operators.

            You can make

        - empty array
        - array with n components dynamically allocated
        - resize an array with complete loss of data
        - assign/construct from a base_array_window to make a copy of the data

            Error checking: no error checking is provided normally.
            Setting the compile time switch DUNE_ISTL_WITH_CHECKING
            enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class base_array : public base_array_unmanaged<B,A>
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;

    //! make iterators available as types
    typedef typename base_array_unmanaged<B,A>::iterator iterator;

    //! make iterators available as types
    typedef typename base_array_unmanaged<B,A>::const_iterator const_iterator;

    //===== constructors and such

    //! makes empty array
    base_array ()
    {
      this->n = 0;
      this->p = 0;
    }

    //! make array with _n components
    base_array (int _n)
    {
      this->n = _n;
      if (this->n>0)
        this->p = A::template malloc<B>(this->n);
      else
      {
        this->n = 0;
        this->p = 0;
      }
    }

    //! copy constructor
    base_array (const base_array& a)
    {
      // allocate memory with same size as a
      this->n = a.n;
      if (this->n>0)
        this->p = A::template malloc<B>(this->n);
      else
      {
        this->n = 0;
        this->p = 0;
      }

      // and copy elements
      for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
    }

    //! construct from base class object
    base_array (const base_array_unmanaged<B,A>& _a)
    {
      const base_array& a = static_cast<const base_array&>(_a);

      // allocate memory with same size as a
      this->n = a.n;
      if (this->n>0)
        this->p = A::template malloc<B>(this->n);
      else
      {
        this->n = 0;
        this->p = 0;
      }

      // and copy elements
      for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
    }


    //! free dynamic memory
    ~base_array ()
    {
      if (this->n>0) A::template free<B>(this->p);
    }

    //! reallocate array to given size, any data is lost
    void resize (int _n)
    {
      if (this->n==_n) return;

      if (this->n>0) A::template free<B>(this->p);
      this->n = _n;
      if (this->n>0)
        this->p = A::template malloc<B>(this->n);
      else
      {
        this->n = 0;
        this->p = 0;
      }
    }

    //! assignment
    base_array& operator= (const base_array& a)
    {
      if (&a!=this)     // check if this and a are different objects
      {
        // adjust size of array
        if (this->n!=a.n)           // check if size is different
        {
          if (this->n>0) A::template free<B>(this->p);                 // delete old memory
          this->n = a.n;
          if (this->n>0)
            this->p = A::template malloc<B>(this->n);
          else
          {
            this->n = 0;
            this->p = 0;
          }
        }
        // copy data
        for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
      }
      return *this;
    }

    //! assign from base class object
    base_array& operator= (const base_array_unmanaged<B,A>& a)
    {
      return this->operator=(static_cast<const base_array&>(a));
    }

  };




  /**  A simple array container with non-consecutive index set.
       Elements in the array are of type B. This class provides

       -  iterator access
       -  const_iterator access
       -  random access in log(n) steps using binary search
           -  find returning iterator

           This container has *NO* memory management at all,
           copy constuctor, assignment and destructor are all default.

           The constructor is made protected to emphasize that objects
       are only usably in derived classes.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class compressed_base_array_unmanaged
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;


    //===== access to components

    //! random access to blocks, assumes ascending ordering
    B& operator[] (int i)
    {
      int l=0, r=n-1;
      while (l<r)
      {
        int q = (l+r)/2;
        if (i <= j[q]) r=q;
        else l = q+1;
      }
#ifdef DUNE_ISTL_WITH_CHECKING
      if (j[l]!=i) DUNE_THROW(ISTLError,"index not in compressed array");
#endif
      return p[l];
    }

    //! same for read only access, assumes ascending ordering
    const B& operator[] (int i) const
    {
      int l=0, r=n-1;
      while (l<r)
      {
        int q = (l+r)/2;
        if (i <= j[q]) r=q;
        else l = q+1;
      }
#ifdef DUNE_ISTL_WITH_CHECKING
      if (j[l]!=i) DUNE_THROW(ISTLError,"index not in compressed array");
#endif
      return p[l];
    }

    // forward declaration
    class const_iterator;

    //! iterator class for sequential access
    class iterator
    {
    public:
      //! constructor
      iterator ()
      {
        p = 0;
        j = 0;
        i = 0;
      }

      iterator (B* _p, int* _j, int _i) : p(_p), j(_j), i(_i)
      {       }

      //! prefix increment
      iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        //        return (p+i)==(it.p+it.i);
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        //        return (p+i)!=(it.p+it.i);
        return (i)!=(it.i);
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        //		return (p+i)==(it.p+it.i);
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        //        return (p+i)!=(it.p+it.i);
        return (i)!=(it.i);
      }

      //! dereferencing
      B& operator* ()
      {
        return p[i];
      }

      //! arrow
      B* operator-> ()
      {
        return p+i;
      }

      // return index corresponding to pointer
      int index () const
      {
        return j[i];
      }

      // return index corresponding to pointer
      void setindex (int k)
      {
        return j[i] = k;
      }

      friend class const_iterator;

    private:
      B* p;
      int* j;
      int i;
    };

    //! begin iterator
    iterator begin ()
    {
      return iterator(p,j,0);
    }

    //! end iterator
    iterator end ()
    {
      return iterator(p,j,n);
    }

    //! const_iterator class for sequential access
    class const_iterator
    {
    public:
      //! constructor
      const_iterator ()
      {
        p = 0;
        j = 0;
        i = 0;
      }

      const_iterator (const B* _p, const int* _j, int _i) : p(_p), j(_j), i(_i)
      {       }

      const_iterator (const iterator& it) : p(it.p), j(it.j), i(it.i)
      {       }

      //! prefix increment
      const_iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        //        return (p+i)==(it.p+it.i);
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        //        return (p+i)!=(it.p+it.i);
        return (i)!=(it.i);
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        //        return (p+i)==(it.p+it.i);
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        //		return (p+i)!=(it.p+it.i);
        return (i)!=(it.i);
      }

      //! dereferencing
      const B& operator* () const
      {
        return p[i];
      }

      //! arrow
      const B* operator-> () const
      {
        return p+i;
      }

      // return index corresponding to pointer
      int index () const
      {
        return j[i];
      }

      friend class iterator;

    private:
      const B* p;
      const int* j;
      int i;
    };

    //! begin const_iterator
    const_iterator begin () const
    {
      return const_iterator(p,j,0);
    }

    //! end const_iterator
    const_iterator end () const
    {
      return const_iterator(p,j,n);
    }

    //! random access returning iterator (end if not contained)
    iterator find (int i)
    {
      int l=0, r=n-1;
      while (l<r)
      {
        int q = (l+r)/2;
        if (i <= j[q]) r=q;
        else l = q+1;
      }

      if (i==j[l])
        return iterator(p,j,l);
      else
        return iterator(p,j,n);
    }

    //===== sizes

    //! number of blocks in the array (are of size 1 here)
    int size () const
    {
      return n;
    }

  protected:
    //! makes empty array
    compressed_base_array_unmanaged ()
    {
      n = 0;
      p = 0;
      j = 0;
    }

    int n;      // number of elements in array
    B *p;       // pointer to dynamically allocated built-in array
    int* j;     // the index set
  };



  /** @} end documentation */

} // end namespace

#endif
