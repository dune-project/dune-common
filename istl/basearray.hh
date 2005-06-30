// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BASEARRAY_HH
#define DUNE_BASEARRAY_HH

#include <cmath>
#include <complex>
#include <cstddef>

#include "istlexception.hh"
#include "allocator.hh"

/** \file
   \brief Implements several basic array containers.
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
  template<class B, class A=ISTLAllocator>
  class base_array_unmanaged
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;

    //! the type for the index access
    typedef typename A::size_type size_type;


    //===== access to components

    //! random access to blocks
    B& operator[] (size_type i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return p[i];
    }

    //! same for read only access
    const B& operator[] (size_type i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i>=n) DUNE_THROW(ISTLError,"index out of range");
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

      iterator (const B* _p, B* _i) : p(_p), i(_i)
      {       }

      //! prefix increment
      iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      iterator& operator--()
      {
        --i;
        return *this;
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        return (i)!=(it.i);
      }

      //! equality with a const iterator
      bool operator== (const const_iterator& it) const
      {
        return (i)==(it.i);
      }

      //! inequality with a const iterator
      bool operator!= (const const_iterator& it) const
      {
        return (i)!=(it.i);
      }

      //! dereferencing
      B& operator* () const
      {
        return *i;
      }

      //! arrow
      B* operator-> () const
      {
        return i;
      }

      //! return index corresponding to pointer
      size_type index () const
      {
        return i-p;
      }

      friend class const_iterator;

    private:
      const B* p;
      B* i;

    };

    //! begin iterator
    iterator begin ()
    {
      return iterator(p,p);
    }

    //! end iterator
    iterator end ()
    {
      return iterator(p,p+n);
    }

    //! begin reverse iterator
    iterator rbegin ()
    {
      return iterator(p,p+n-1);
    }

    //! end reverse iterator
    iterator rend ()
    {
      return iterator(p,p-1);
    }

    //! random access returning iterator (end if not contained)
    iterator find (size_type i)
    {
      if (i<n)
        return iterator(p,p+i);
      else
        return iterator(p,p+n);
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

      const_iterator (const B* _p, const B* _i) : p(_p), i(_i)
      {       }

      const_iterator (const iterator& it) : p(it.p), i(it.i)
      {       }

      //! prefix increment
      const_iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      const_iterator& operator--()
      {
        --i;
        return *this;
      }

      //! equality
      bool operator== (const const_iterator& it) const
      {
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const const_iterator& it) const
      {
        return (i)!=(it.i);
      }

      //! equality
      bool operator== (const iterator& it) const
      {
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const iterator& it) const
      {
        return (i)!=(it.i);
      }

      //! dereferencing
      const B& operator* () const
      {
        return *i;
      }

      //! arrow
      const B* operator-> () const
      {
        return i;
      }

      //! return index corresponding to pointer
      size_type index () const
      {
        return i-p;
      }

      friend class iterator;

    private:
      const B* p;
      const B* i;
    };

    //! begin const_iterator
    const_iterator begin () const
    {
      return const_iterator(p,p+0);
    }

    //! end const_iterator
    const_iterator end () const
    {
      return const_iterator(p,p+n);
    }

    //! begin reverse const_iterator
    const_iterator rbegin () const
    {
      return const_iterator(p,p+n-1);
    }

    //! end reverse const_iterator
    const_iterator rend () const
    {
      return const_iterator(p,p-1);
    }

    //! random access returning iterator (end if not contained)
    const_iterator find (size_type i) const
    {
      if (i<n)
        return const_iterator(p,p+i);
      else
        return const_iterator(p,p+n);
    }


    //===== sizes

    //! number of blocks in the array (are of size 1 here)
    size_type size () const
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

    size_type n;     // number of elements in array
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
  template<class B, class A=ISTLAllocator>
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

    //! The type used for the index access
    typedef typename base_array_unmanaged<B,A>::size_type size_type;

    //! The type used for the difference between two iterator positions
    typedef typename A::difference_type difference_type;

    //===== constructors and such

    //! makes empty array
    base_array_window ()
    {
      this->n = 0;
      this->p = 0;
    }

    //! make array from given pointer and size
    base_array_window (B* _p, size_type _n)
    {
      this->n = _n;
      this->p = _p;
    }

    //===== window manipulation methods

    //! set pointer and length
    void set (size_type _n, B* _p)
    {
      this->n = _n;
      this->p = _p;
    }

    //! advance pointer by newsize elements and then set size to new size
    void advance (difference_type newsize)
    {
      this->p += this->n;
      this->n = newsize;
    }

    //! increment pointer by offset and set size
    void move (difference_type offset, size_type newsize)
    {
      this->p += offset;
      this->n = newsize;
    }

    //! increment pointer by offset, leave size
    void move (difference_type offset)
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
  template<class B, class A=ISTLAllocator>
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

    //! The type used for the index access
    typedef typename base_array_unmanaged<B,A>::size_type size_type;

    //! The type used for the difference between two iterator positions
    typedef typename A::difference_type difference_type;

    //===== constructors and such

    //! makes empty array
    base_array ()
    {
      this->n = 0;
      this->p = 0;
    }

    //! make array with _n components
    base_array (size_type _n)
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
      for (size_type i=0; i<this->n; i++) this->p[i]=a.p[i];
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
      for (size_type i=0; i<this->n; i++) this->p[i]=a.p[i];
    }


    //! free dynamic memory
    ~base_array ()
    {
      if (this->n>0) A::template free<B>(this->p);
    }

    //! reallocate array to given size, any data is lost
    void resize (size_type _n)
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
        for (size_type i=0; i<this->n; i++) this->p[i]=a.p[i];
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
  template<class B, class A=ISTLAllocator>
  class compressed_base_array_unmanaged
  {
  public:

    //===== type definitions and constants

    //! export the type representing the components
    typedef B member_type;

    //! export the allocator type
    typedef A allocator_type;

    //! The type used for the index access
    typedef typename A::size_type size_type;

    //===== access to components

    //! random access to blocks, assumes ascending ordering
    B& operator[] (size_type i)
    {
      size_type l=0, r=n-1;
      while (l<r)
      {
        size_type q = (l+r)/2;
        if (i <= j[q]) r=q;
        else l = q+1;
      }
#ifdef DUNE_ISTL_WITH_CHECKING
      if (j[l]!=i) DUNE_THROW(ISTLError,"index not in compressed array");
#endif
      return p[l];
    }

    //! same for read only access, assumes ascending ordering
    const B& operator[] (size_type i) const
    {
      size_type l=0, r=n-1;
      while (l<r)
      {
        size_type q = (l+r)/2;
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

      iterator (B* _p, size_type* _j, size_type _i) : p(_p), j(_j), i(_i)
      {       }

      //! prefix increment
      iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      iterator& operator--()
      {
        --i;
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
      B& operator* () const
      {
        return p[i];
      }

      //! arrow
      B* operator-> () const
      {
        return p+i;
      }

      //! return index corresponding to pointer
      size_type index () const
      {
        return j[i];
      }

      //! Set index corresponding to pointer
      void setindex (size_type k)
      {
        return j[i] = k;
      }

      /**
       * @brief offset from the fist entry.
       *
       * An iterator positioned at the beginning
       * has to be increment this amount of times to
       * the same position.
       */
      size_type offset () const
      {
        return i;
      }

      friend class const_iterator;

    private:
      B* p;
      size_type* j;
      size_type i;
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

    //! begin reverse iterator
    iterator rbegin ()
    {
      return iterator(p,j,n-1);
    }

    //! end reverse iterator
    iterator rend ()
    {
      return iterator(p,j,-1);
    }

    //! random access returning iterator (end if not contained)
    iterator find (size_type i)
    {
      size_type l=0, r=n-1;
      while (l<r)
      {
        size_type q = (l+r)/2;
        if (i <= j[q]) r=q;
        else l = q+1;
      }

      if (i==j[l])
        return iterator(p,j,l);
      else
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

      //! \todo please doc me!
      const_iterator (const B* _p, const size_type* _j, size_type _i) : p(_p), j(_j), i(_i)
      {       }

      //! Copy constructor from a non-const iterator
      const_iterator (const iterator& it) : p(it.p), j(it.j), i(it.i)
      {       }

      //! prefix increment
      const_iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      const_iterator& operator--()
      {
        --i;
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

      //! return index corresponding to pointer
      size_type index () const
      {
        return j[i];
      }

      /**
       * @brief offset from the fist entry.
       *
       * An iterator positioned at the beginning
       * has to be increment this amount of times to
       * the same position.
       */
      size_type offset () const
      {
        return i;
      }

      friend class iterator;

    private:
      const B* p;
      const size_type* j;
      size_type i;
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

    //! begin reverse const_iterator
    const_iterator rbegin () const
    {
      return const_iterator(p,j,n-1);
    }

    //! end reverse const_iterator
    const_iterator rend () const
    {
      return const_iterator(p,j,-1);
    }

    //! random access returning iterator (end if not contained)
    const_iterator find (size_type i) const
    {
      size_type l=0, r=n-1;
      while (l<r)
      {
        size_type q = (l+r)/2;
        if (i <= j[q]) r=q;
        else l = q+1;
      }

      if (i==j[l])
        return const_iterator(p,j,l);
      else
        return const_iterator(p,j,n);
    }

    //===== sizes

    //! number of blocks in the array (are of size 1 here)
    size_type size () const
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

    size_type n;      // number of elements in array
    B *p;       // pointer to dynamically allocated built-in array
    size_type* j;     // the index set
  };



  /** @} end documentation */

} // end namespace

#endif
