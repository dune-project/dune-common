// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_VBVECTOR_HH
#define DUNE_VBVECTOR_HH

#include <math.h>
#include <complex>
#include <iostream>

#include "istlexception.hh"
#include "allocator.hh"
#include "bvector.hh"

/** \file
 * \brief ???
 */

namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */

  /** implements a vector consisting of a number of blocks (to
          be given at run-time) which themselves consist of a number
          of blocks (also given at run-time) of the given type B.

          VariableBlockVector is a container of containers!

   */
  template<class B, class A=ISTLAllocator>
  class VariableBlockVector : public block_vector_unmanaged<B,A>
                              // this derivation gives us all the blas level 1 and norms
                              // on the large array. However, access operators have to be
                              // overwritten.
  {
  public:

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename B::field_type field_type;

    //! export the allocator type
    typedef A allocator_type;

    /** export the type representing the components, note that this
            is *not* the type refered to by the iterators and random access.
            However, it can be used to copy blocks (which is its only purpose).
     */
    typedef BlockVector<B,A> block_type;

    /** increment block level counter, yes, it is two levels because
            VariableBlockVector is a container of containers
     */
    enum {blocklevel = B::blocklevel+2};

    // just a shorthand
    typedef BlockVectorWindow<B,A> window_type;


    //===== constructors and such

    /** constructor without arguments makes empty vector,
            object cannot be used yet
     */
    VariableBlockVector () : block_vector_unmanaged<B,A>()
    {
      // nothing is known ...
      nblocks = 0;
      block = 0;
      initialized = false;
    }

    /** make vector with given number of blocks, but size of each block is not yet known,
            object cannot be used yet
     */
    VariableBlockVector (int _nblocks) : block_vector_unmanaged<B,A>()
    {
      // we can allocate the windows now
      nblocks = _nblocks;
      if (nblocks>0)
      {
        block = A::template malloc<window_type>(nblocks);
      }
      else
      {
        nblocks = 0;
        block = 0;;
      }

      // Note: memory in base class still not allocated
      // the vector not usable
      initialized = false;
    }

    /** make vector with given number of blocks each having a constant size,
            object is fully usable then
     */
    VariableBlockVector (int _nblocks, int m) : block_vector_unmanaged<B,A>()
    {
      // and we can allocate the big array in the base class
      this->n = _nblocks*m;
      if (this->n>0)
      {
        this->p = A::template malloc<B>(this->n);
      }
      else
      {
        this->n = 0;
        this->p = 0;
      }

      // we can allocate the windows now
      nblocks = _nblocks;
      if (nblocks>0)
      {
        // alloc
        block = A::template malloc<window_type>(nblocks);

        // set the windows into the big array
        for (int i=0; i<nblocks; ++i)
          block[i].set(m,this->p+(i*m));
      }
      else
      {
        nblocks = 0;
        block = 0;;
      }

      // and the vector is usable
      initialized = true;
    }

    //! copy constructor, has copy semantics
    VariableBlockVector (const VariableBlockVector& a)
    {
      // allocate the big array in the base class
      this->n = a.n;
      if (this->n>0)
      {
        // alloc
        this->p = A::template malloc<B>(this->n);

        // copy data
        for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
      }
      else
      {
        this->n = 0;
        this->p = 0;
      }

      // we can allocate the windows now
      nblocks = a.nblocks;
      if (nblocks>0)
      {
        // alloc
        block = A::template malloc<window_type>(nblocks);

        // and we must set the windows
        block[0].set(a.block[0].getsize(),this->p);           // first block
        for (int i=1; i<nblocks; ++i)                         // and the rest
          block[i].set(a.block[i].getsize(),block[i-1].getptr()+block[i-1].getsize());
      }
      else
      {
        nblocks = 0;
        block = 0;;
      }

      // and we have a usable vector
      initialized = true;
    }

    //! free dynamic memory
    ~VariableBlockVector ()
    {
      if (this->n>0) A::template free<B>(this->p);
      if (nblocks>0) A::template free<window_type>(block);
    }


    //! same effect as constructor with same argument
    void resize (int _nblocks)
    {
      // deallocate memory if necessary
      if (this->n>0) A::template free<B>(this->p);
      if (nblocks>0) A::template free<window_type>(block);
      this->n = 0;
      this->p = 0;

      // we can allocate the windows now
      nblocks = _nblocks;
      if (nblocks>0)
      {
        block = A::template malloc<window_type>(nblocks);
      }
      else
      {
        nblocks = 0;
        block = 0;;
      }

      // and the vector not fully usable
      initialized = false;
    }

    //! same effect as constructor with same argument
    void resize (int _nblocks, int m)
    {
      // deallocate memory if necessary
      if (this->n>0) A::template free<B>(this->p);
      if (nblocks>0) A::template free<window_type>(block);

      // and we can allocate the big array in the base class
      this->n = _nblocks*m;
      if (this->n>0)
      {
        this->p = A::template malloc<B>(this->n);
      }
      else
      {
        this->n = 0;
        this->p = 0;
      }

      // we can allocate the windows now
      nblocks = _nblocks;
      if (nblocks>0)
      {
        // alloc
        block = A::template malloc<window_type>(nblocks);

        // set the windows into the big array
        for (int i=0; i<nblocks; ++i)
          block[i].set(m,this->p+(i*m));
      }
      else
      {
        nblocks = 0;
        block = 0;;
      }

      // and the vector is usable
      initialized = true;
    }

    //! assignment
    VariableBlockVector& operator= (const VariableBlockVector& a)
    {
      if (&a!=this)     // check if this and a are different objects
      {
        // reallocate arrays if necessary
        // Note: still the block sizes may vary !
        if (this->n!=a.n || nblocks!=a.nblocks)
        {
          // deallocate memory if necessary
          if (this->n>0) A::template free<B>(this->p);
          if (nblocks>0) A::template free<window_type>(block);

          // allocate the big array in the base class
          this->n = a.n;
          if (this->n>0)
          {
            // alloc
            this->p = A::template malloc<B>(this->n);
          }
          else
          {
            this->n = 0;
            this->p = 0;
          }

          // we can allocate the windows now
          nblocks = a.nblocks;
          if (nblocks>0)
          {
            // alloc
            block = A::template malloc<window_type>(nblocks);
          }
          else
          {
            nblocks = 0;
            block = 0;;
          }
        }

        // copy block structure, might be different although
        // sizes are the same !
        if (nblocks>0)
        {
          block[0].set(a.block[0].getsize(),this->p);                 // first block
          for (int i=1; i<nblocks; ++i)                               // and the rest
            block[i].set(a.block[i].getsize(),block[i-1].getptr()+block[i-1].getsize());
        }

        // and copy the data
        for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
      }

      // and we have a usable vector
      initialized = true;

      return *this;     // Gebe Referenz zurueck damit a=b=c; klappt
    }


    //===== assignment from scalar

    //! assign from scalar
    VariableBlockVector& operator= (const field_type& k)
    {
      (static_cast<block_vector_unmanaged<B,A>&>(*this)) = k;
      return *this;
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
        n = 0;
      }

      //! prefix increment
      CreateIterator& operator++()
      {
        // we are at block i and the blocks size is known

        // set the blocks size to current k
        v.block[i].setsize(k);

        // accumulate total size
        n += k;

        // go to next block
        ++i;

        // reset block size
        k = 0;

        // if we are past the last block, finish off
        if (i==v.nblocks)
        {
          // now we can allocate the big array in the base class of v
          v.n = n;
          if (n>0)
          {
            // alloc
            v.p = A::template malloc<B>(n);
          }
          else
          {
            v.n = 0;
            v.p = 0;
          }

          // and we set the window pointer
          if (v.nblocks>0)
          {
            v.block[0].setptr(v.p);                       // pointer tofirst block
            for (int j=1; j<v.nblocks; ++j)               // and the rest
              v.block[j].setptr(v.block[j-1].getptr()+v.block[j-1].getsize());
          }

          // and the vector is ready
          v.initialized = true;

          //std::cout << "made vbvector with " << v.n << " components" << std::endl;
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
      int k;                      // size of current block to be defined
      int n;                      // total number of elements to be allocated
    };

    // CreateIterator wants to set all the arrays ...
    friend class CreateIterator;

    //! get initial create iterator
    CreateIterator createbegin ()
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (initialized) DUNE_THROW(ISTLError,"no CreateIterator in initialized state");
#endif
      return CreateIterator(*this,0);
    }

    //! get create iterator pointing to one after the last block
    CreateIterator createend ()
    {
      return CreateIterator(*this,nblocks);
    }


    //===== access to components
    // has to be overwritten from base class because it must
    // return access to the windows

    //! random access to blocks
    window_type& operator[] (int i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=nblocks) DUNE_THROW(ISTLError,"index out of range");
#endif
      return block[i];
    }

    //! same for read only access
    const window_type& operator[] (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=nblocks) DUNE_THROW(ISTLError,"index out of range");
#endif
      return block[i];
    }

    // forward declaration
    class ConstIterator;

    //! Iterator class for sequential access
    class Iterator
    {
    public:
      //! constructor, no arguments
      Iterator ()
      {
        p = 0;
        i = 0;
      }

      //! constructor
      Iterator (window_type* _p, int _i) : p(_p), i(_i)
      {       }

      //! prefix increment
      Iterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      Iterator& operator--()
      {
        --i;
        return *this;
      }

      //! equality
      bool operator== (const Iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! equality
      bool operator== (const ConstIterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const ConstIterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      window_type& operator* ()
      {
        return p[i];
      }

      //! arrow
      window_type* operator-> ()
      {
        return p+i;
      }

      // return index corresponding to pointer
      int index () const
      {
        return i;
      }

      friend class ConstIterator;

    private:
      window_type* p;
      int i;
    };

    //! begin Iterator
    Iterator begin ()
    {
      return Iterator(block,0);
    }

    //! end Iterator
    Iterator end ()
    {
      return Iterator(block,nblocks);
    }

    //! begin Iterator
    Iterator rbegin ()
    {
      return Iterator(block,nblocks-1);
    }

    //! end Iterator
    Iterator rend ()
    {
      return Iterator(block,-1);
    }

    //! random access returning iterator (end if not contained)
    Iterator find (int i)
    {
      if (i>=0 && i<nblocks)
        return Iterator(block,i);
      else
        return Iterator(block,nblocks);
    }

    //! ConstIterator class for sequential access
    class ConstIterator
    {
    public:
      //! constructor
      ConstIterator ()
      {
        p = 0;
        i = 0;
      }

      //! constructor from pointer
      ConstIterator (const window_type* _p, int _i) : p(_p), i(_i)
      {       }

      //! constructor from non_const iterator
      ConstIterator (const Iterator& it) : p(it.p), i(it.i)
      {       }

      //! prefix increment
      ConstIterator& operator++()
      {
        ++i;
        return *this;
      }

      //! prefix decrement
      ConstIterator& operator--()
      {
        --i;
        return *this;
      }

      //! equality
      bool operator== (const ConstIterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const ConstIterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! equality
      bool operator== (const Iterator& it) const
      {
        return (p+i)==(it.p+it.i);
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        return (p+i)!=(it.p+it.i);
      }

      //! dereferencing
      const window_type& operator* () const
      {
        return p[i];
      }

      //! arrow
      const window_type* operator-> () const
      {
        return p+i;
      }

      // return index corresponding to pointer
      int index () const
      {
        return i;
      }

      friend class Iterator;

    private:
      const window_type* p;
      int i;
    };

    //! begin ConstIterator
    ConstIterator begin () const
    {
      return ConstIterator(block,0);
    }

    //! end ConstIterator
    ConstIterator end () const
    {
      return ConstIterator(block,nblocks);
    }

    //! begin ConstIterator
    ConstIterator rbegin () const
    {
      return ConstIterator(block,nblocks-1);
    }

    //! end ConstIterator
    ConstIterator rend () const
    {
      return ConstIterator(block,-1);
    }


    //===== sizes

    //! number of blocks in the vector (are of variable size here)
    int N () const
    {
      return nblocks;
    }


  private:
    int nblocks;            // number of blocks in vector
    window_type* block;     // array of blocks pointing to the array in the base class
    bool initialized;       // true if vector has been initialized
  };



  /** @} end documentation */

} // end namespace

#endif
