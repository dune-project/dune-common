// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BCRSMATRIX_HH
#define DUNE_BCRSMATRIX_HH

#include <math.h>
#include <complex>
#include <set>
#include <iostream>

#include "istlexception.hh"
#include "allocator.hh"
#include "bvector.hh"

/*! \file
 * \brief Implementation of the BCRSMatrix class
 */

namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */


  /** \brief A sparse block matrix with compressed row storage

     Implements a block compressed row storage scheme. The block
       type B can be any type implementing the matrix interface.

           Different ways to build up a compressed row
           storage matrix are supported:

           1. Row-wise scheme

           Rows are built up in sequential order. Size of the row and
       the column indices are defined. A row can be used as soon as it
       is initialized. With respect to memory there are two variants of
       this scheme: (a) number of non-zeroes known in advance (application
       finite difference schemes), (b) number of non-zeroes not known
       in advance (application: Sparse LU, ILU(n)).

           2. Random scheme

           For general finite element implementations the number of rows n
       is known, the number of non-zeroes might also be known (e.g.
     #edges + #nodes for P1) but the size of a row and the indices of a row
       can not be defined in sequential order.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
#ifdef DUNE_EXPRESSIONTEMPLATES
  template<class B, class A>
  class BCRSMatrix : public ExprTmpl::Matrix< BCRSMatrix<B,A> >
#else
  template<class B, class A=ISTLAllocator>
  class BCRSMatrix
#endif
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
    typedef CompressedBlockVectorWindow<B,A> row_type;

    //! The type for the index access and the size
    typedef typename A::size_type size_type;

    //! increment block level counter
    enum {
      //! The number of blocklevels the matrix contains.
      blocklevel = B::blocklevel+1
    };

    //! we support two modes
    enum BuildMode {
      /**
       * @brief Build in a row-wise manner.
       *
       * Rows are built up in sequential order. Size of the row and
       * the column indices are defined. A row can be used as soon as it
       * is initialized. With respect to memory there are two variants of
       * this scheme: (a) number of non-zeroes known in advance (application
       * finite difference schemes), (b) number of non-zeroes not known
       * in advance (application: Sparse LU, ILU(n)).
       */
      row_wise,
      /**
       * @brief Build entries randomly.
       *
       * For general finite element implementations the number of rows n
       * is known, the number of non-zeroes might also be known (e.g.
       * #edges + #nodes for P1) but the size of a row and the indices of a row
       * can not be defined in sequential order.
       */
      random,
      /**
       * @brief Build mode not set!
       */
      unknown
    };


    //===== random access interface to rows of the matrix

    //! random access to the rows
    row_type& operator[] (size_type i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (r==0) DUNE_THROW(ISTLError,"row not initialized yet");
      if (i>=n) DUNE_THROW(ISTLError,"index out of range");
      if (r[i].getptr()==0) DUNE_THROW(ISTLError,"row not initialized yet");
#endif
      return r[i];
    }

    //! same for read only access
    const row_type& operator[] (size_type i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!ready) DUNE_THROW(ISTLError,"row not initialized yet");
      if (i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return r[i];
    }


    //===== iterator interface to rows of the matrix

    // forward declaration
    class ConstIterator;

    //! Iterator access to matrix rows
    class Iterator
    {
    public:
      //! constructor
      Iterator (row_type* _p, size_type _i)
      {
        p = _p;
        i = _i;
      }

      //! empty constructor, use with care!
      Iterator ()
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
        //		return (p+i)==(it.p+it.i);
        return (i)==(it.i);
      }

      //! inequality
      bool operator!= (const Iterator& it) const
      {
        //		return (p+i)!=(it.p+it.i);
        return (i)!=(it.i);
      }

      //! less than
      bool operator< (const Iterator& it) const
      {
        return (i)<(it.i);
      }

      //! dereferencing
      row_type& operator* () const
      {
        return p[i];
      }

      //! arrow
      row_type* operator-> () const
      {
        return p+i;
      }

      //! return index
      size_type index ()
      {
        return i;
      }

      friend class ConstIterator;

    private:
      row_type* p;
      size_type i;
    };

    //! Get iterator to first row
    Iterator begin ()
    {
      return Iterator(r,0);
    }

    //! Get iterator to one beyond last row
    Iterator end ()
    {
      return Iterator(r,n);
    }

    //! Get Iterator to last row
    Iterator rbegin ()
    {
      return Iterator(r,n-1);
    }

    //! Get Iterator to one before first row
    Iterator rend ()
    {
      return Iterator(r,-1);
    }

    //! rename the iterators for easier access
    typedef Iterator RowIterator;

    /** \brief Iterator for the entries of each row */
    typedef typename row_type::Iterator ColIterator;


    //! Const iterator access to rows
    class ConstIterator
    {
    public:
      //! constructor
      ConstIterator (const row_type* _p, size_type _i) : p(_p), i(_i)
      {       }

      //! empty constructor, use with care!
      ConstIterator ()
      {
        p = 0;
        i = 0;
      }

      //! Copy constructor from mutable iterator.
      ConstIterator(const Iterator& other)
        : p(other.p), i(other.i)
      {}

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
      const row_type& operator* () const
      {
        return p[i];
      }

      //! arrow
      const row_type* operator-> () const
      {
        return p+i;
      }

      //! return index
      size_type index () const
      {
        return i;
      }

      friend class Iterator;

    private:
      const row_type* p;
      size_type i;
    };

    //! Get const iterator to first row
    ConstIterator begin () const
    {
      return ConstIterator(r,0);
    }

    //! Get const iterator to one beyond last row
    ConstIterator end () const
    {
      return ConstIterator(r,n);
    }

    //! Get const iterator to last row
    ConstIterator rbegin () const
    {
      return ConstIterator(r,n-1);
    }

    //! Get const iterator to one before first row
    ConstIterator rend () const
    {
      return ConstIterator(r,-1);
    }

    //! rename the const row iterator for easier access
    typedef ConstIterator ConstRowIterator;

    //! Const iterator to the entries of a row
    typedef typename row_type::ConstIterator ConstColIterator;

    //===== constructors & resizers

    //! an empty matrix
    BCRSMatrix ()
    {
      // the state
      build_mode = unknown;
      ready = false;

      // store parameters
      n = 0;
      m = 0;
      nnz = 0;

      // allocate rows
      r = 0;

      // allocate a and j array
      a = 0;
      j = 0;
    }

    //! matrix with known number of nonzeroes
    BCRSMatrix (size_type _n, size_type _m, size_type _nnz, BuildMode bm)
    {
      // the state
      build_mode = bm;
      ready = false;

      // store parameters
      n = _n;
      m = _m;
      nnz = _nnz;

      // allocate rows
      if (n>0)
      {
        r = A::template malloc<row_type>(n);
      }
      else
      {
        r = 0;
      }

      // allocate a and j array
      if (nnz>0)
      {
        a = A::template malloc<B>(nnz);
        j = A::template malloc<size_type>(nnz);
      }
      else
      {
        a = 0;
        j = 0;
      }
    }

    //! matrix with unknown number of nonzeroes
    BCRSMatrix (size_type _n, size_type _m, BuildMode bm)
    {
      // the state
      build_mode = bm;
      ready = false;

      // store parameters
      n = _n;
      m = _m;
      nnz = 0;

      // allocate rows
      if (n>0)
      {
        r = A::template malloc<row_type>(n);
      }
      else
      {
        r = 0;
      }

      // allocate a and j array
      a = 0;
      j = 0;
    }

    //! copy constructor
    BCRSMatrix (const BCRSMatrix& Mat)
    {
      // deep copy in global array

      // copy sizes
      n = Mat.n;
      m = Mat.m;
      nnz = Mat.nnz;

      // in case of row-wise allocation
      if (nnz<=0)
      {
        nnz = 0;
        for (size_type i=0; i<n; i++)
          nnz += Mat.r[i].getsize();
      }

      // allocate rows
      if (n>0)
      {
        r = A::template malloc<row_type>(n);
      }
      else
      {
        r = 0;
      }

      // allocate a and j array
      if (nnz>0)
      {
        a = A::template malloc<B>(nnz);
        j = A::template malloc<size_type>(nnz);
      }
      else
      {
        a = 0;
        j = 0;
      }

      // build window structure
      for (size_type i=0; i<n; i++)
      {
        // set row i
        r[i].setsize(Mat.r[i].getsize());
        if (i==0)
        {
          r[i].setptr(a);
          r[i].setindexptr(j);
        }
        else
        {
          r[i].setptr( r[i-1].getptr()+r[i-1].getsize() );
          r[i].setindexptr( r[i-1].getindexptr()+r[i-1].getsize() );
        }
      }

      // copy data
      for (size_type i=0; i<n; i++) r[i] = Mat.r[i];

      // finish off
      build_mode = row_wise;     // dummy
      ready = true;
    }

    //! destructor
    ~BCRSMatrix ()
    {
      if (nnz>0)
      {
        // a,j have been allocated as one long vector
        A::template free<size_type>(j);
        A::template free<B>(a);
      }
      else
      {
        // check if memory for rows have been allocated individually
        for (size_type i=0; i<n; i++)
          if (r[i].getsize()>0)
          {
            A::template free<size_type>(r[i].getindexptr());
            A::template free<B>(r[i].getptr());
          }
      }

      // deallocate the rows
      if (n>0) A::template free<row_type>(r);
    }

    //! assignment
    BCRSMatrix& operator= (const BCRSMatrix& Mat)
    {
      // return immediately when self-assignment
      if (&Mat==this) return *this;

      // make it simple: ALWAYS throw away memory for a and j
      if (nnz>0)
      {
        // a,j have been allocated as one long vector
        A::template free<size_type>(j);
        A::template free<B>(a);
      }
      else
      {
        // check if memory for rows have been allocated individually
        for (size_type i=0; i<n; i++)
          if (r[i].getsize()>0)
          {
            A::template free<size_type>(r[i].getindexptr());
            A::template free<B>(r[i].getptr());
          }
      }

      // reallocate the rows if required
      if (n!=Mat.n)
      {
        // free rows
        A::template free<row_type>(r);

        // allocate rows
        n = Mat.n;
        if (n>0)
        {
          r = A::template malloc<row_type>(n);
        }
        else
        {
          r = 0;
        }
      }

      // allocate a,j
      m = Mat.m;
      nnz = Mat.nnz;

      // in case of row-wise allocation
      if (nnz<=0)
      {
        nnz = 0;
        for (size_type i=0; i<n; i++)
          nnz += Mat.r[i].getsize();
      }

      // allocate a and j array
      if (nnz>0)
      {
        a = A::template malloc<B>(nnz);
        j = A::template malloc<size_type>(nnz);
      }
      else
      {
        a = 0;
        j = 0;
      }

      // build window structure
      for (size_type i=0; i<n; i++)
      {
        // set row i
        r[i].setsize(Mat.r[i].getsize());
        if (i==0)
        {
          r[i].setptr(a);
          r[i].setindexptr(j);
        }
        else
        {
          r[i].setptr( r[i-1].getptr()+r[i-1].getsize() );
          r[i].setindexptr( r[i-1].getindexptr()+r[i-1].getsize() );
        }
      }

      // copy data
      for (size_type i=0; i<n; i++) r[i] = Mat.r[i];

      // finish off
      build_mode = row_wise;     // dummy
      ready = true;
      return *this;
    }

    //! Assignment from a scalar
    BCRSMatrix& operator= (const field_type& k)
    {
      for (size_type i=0; i<n; i++) r[i] = k;
      return *this;
    }

    //===== row-wise creation interface

    //! Iterator class for sequential creation of blocks
    class CreateIterator
    {
    public:
      //! constructor
      CreateIterator (BCRSMatrix& _Mat, size_type _i) : Mat(_Mat)
      {
        if (Mat.build_mode!=row_wise)
          DUNE_THROW(ISTLError,"creation only allowed for uninitialized matrix");

        i = _i;
        nnz = 0;
      }

      //! prefix increment
      CreateIterator& operator++()
      {
        // this should only be called if matrix is in creation
        if (Mat.ready)
          DUNE_THROW(ISTLError,"matrix already built up");

        // row i is defined through the pattern
        // get memory for the row and initialize the j array
        // this depends on the allocation mode

        // compute size of the row
        size_type s = pattern.size();

        // update number of nonzeroes including this row
        nnz += s;

        // alloc memory / set window
        if (Mat.nnz>0)
        {
          // memory is allocated in one long array

          // check if that memory is sufficient
          if (nnz>Mat.nnz)
            DUNE_THROW(ISTLError,"allocated nnz too small");

          // set row i
          if (i==0)
            Mat.r[i].set(s,Mat.a,Mat.j);
          else
            Mat.r[i].set(s,Mat.r[i-1].getptr()+Mat.r[i-1].getsize(),
                         Mat.r[i-1].getindexptr()+Mat.r[i-1].getsize());
        }
        else
        {
          // memory is allocated individually per row
          // allocate and set row i
          if (s>0)
          {
            B*   a = A::template malloc<B>(s);
            size_type* j = A::template malloc<size_type>(s);
            Mat.r[i].set(s,a,j);
          }
          else
            Mat.r[i].set(0,0,0);
        }

        // initialize the j array for row i from pattern
        size_type k=0;
        size_type *j =  Mat.r[i].getindexptr();
        for (typename std::set<size_type>::const_iterator it=pattern.begin(); it!=pattern.end(); ++it)
          j[k++] = *it;

        // now go to next row
        i++;
        pattern.clear();

        // check if this was last row
        if (i==Mat.n)
        {
          Mat.ready = true;
        }
        // done
        return *this;
      }

      //! inequality
      bool operator!= (const CreateIterator& it) const
      {
        return (i!=it.i) || (&Mat!=&it.Mat);
      }

      //! equality
      bool operator== (const CreateIterator& it) const
      {
        return (i==it.i) && (&Mat==&it.Mat);
      }

      //! dereferencing
      size_type index ()
      {
        return i;
      }

      //! put column index in row
      void insert (size_type j)
      {
        pattern.insert(j);
      }

      //! return true if column index is in row
      bool contains (size_type j)
      {
        if (pattern.find(j)!=pattern.end())
          return true;
        else
          return false;
      }

    private:
      BCRSMatrix& Mat;     // the matrix we are defining
      size_type i;               // current row to be defined
      size_type nnz;             // count total number of nonzeros
      std::set<size_type> pattern;     // used to compile entries in a row
    };


    //! allow CreateIterator to access internal data
    friend class CreateIterator;

    //! get initial create iterator
    CreateIterator createbegin ()
    {
      return CreateIterator(*this,0);
    }

    //! get create iterator pointing to one after the last block
    CreateIterator createend ()
    {
      return CreateIterator(*this,n);
    }


    //===== random creation interface

    //! set number of indices in row i to s
    void setrowsize (size_type i, size_type s)
    {
      if (build_mode!=random)
        DUNE_THROW(ISTLError,"requires random build mode");
      if (ready)
        DUNE_THROW(ISTLError,"matrix already built up");

      r[i].setsize(s);
    }

    //! increment size of row i by 1
    void incrementrowsize (size_type i)
    {
      if (build_mode!=random)
        DUNE_THROW(ISTLError,"requires random build mode");
      if (ready)
        DUNE_THROW(ISTLError,"matrix already built up");

      r[i].setsize(r[i].getsize()+1);
    }

    //! indicate that size of all rows is defined
    void endrowsizes ()
    {
      if (build_mode!=random)
        DUNE_THROW(ISTLError,"requires random build mode");
      if (ready)
        DUNE_THROW(ISTLError,"matrix already built up");

      // compute total size, check positivity
      size_type total=0;
      for (size_type i=0; i<n; i++)
      {
        if (r[i].getsize()<=0)
          DUNE_THROW(ISTLError,"rowsize must be positive");
        total += r[i].getsize();
      }

      // allocate/check memory
      if (nnz>0)
      {
        // there is already memory
        if (total>nnz)
          DUNE_THROW(ISTLError,"nnz too small");
      }
      else
      {
        // we allocate the memory here
        nnz = total;
        if (nnz>0)
        {
          a = A::template malloc<B>(nnz);
          j = A::template malloc<size_type>(nnz);
        }
        else
        {
          a = 0;
          j = 0;
        }
      }

      // set the window pointers correctly
      for (size_type i=0; i<n; i++)
      {
        // set row i
        if (i==0)
        {
          r[i].setptr(a);
          r[i].setindexptr(j);
        }
        else
        {
          r[i].setptr( r[i-1].getptr()+r[i-1].getsize() );
          r[i].setindexptr( r[i-1].getindexptr()+r[i-1].getsize() );
        }
      }

      // initialize j array with m (an invalid column index)
      // this indicates an unused entry
      for (size_type k=0; k<nnz; k++)
        j[k] = m;
    }

    //! add index (row,col) to the matrix
    void addindex (size_type row, size_type col)
    {
      if (build_mode!=random)
        DUNE_THROW(ISTLError,"requires random build mode");
      if (ready)
        DUNE_THROW(ISTLError,"matrix already built up");

      // get row
      size_type* p = r[row].getindexptr();
      size_type s = r[row].getsize();

      // binary search for col
      size_type l=0, r=s-1;
      while (l<r)
      {
        size_type q = (l+r)/2;
        if (col <= p[q]) r=q;
        else l = q+1;
      }

      // check if index is already in row
      if (p[l]==col) return;

      // no, find first free entry by binary search
      l=0; r=s-1;
      while (l<r)
      {
        size_type q = (l+r)/2;
        if (m <= p[q]) r=q;
        else l = q+1;
      }
      if (p[l]!=m)
        DUNE_THROW(ISTLError,"row is too small");

      // now p[l] is the leftmost free entry
      p[l] = col;

      // use insertion sort to move index to correct position
      for (size_type i=l-1; i>=0; i--)
        if (p[i]>p[i+1])
          std::swap(p[i],p[i+1]);
        else
          break;
    }

    //! indicate that all indices are defined, check consistency
    void endindices ()
    {
      if (build_mode!=random)
        DUNE_THROW(ISTLError,"requires random build mode");
      if (ready)
        DUNE_THROW(ISTLError,"matrix already built up");

      // check if there are undefined indices
      for (size_type k=0; k<nnz; k++)
        if (j[k]<0 || j[k]>=m)
        {
          std::cout << "j[" << k << "]=" << j[k] << std::endl;
          DUNE_THROW(ISTLError,"undefined index detected");
        }
      // if not, set matrix to ready
      ready = true;
    }

    //===== vector space arithmetic

    //! vector space multiplication with scalar
    BCRSMatrix& operator*= (const field_type& k)
    {
      if (nnz>0)
      {
        // process 1D array
        for (size_type i=0; i<nnz; i++)
          a[i] *= k;
      }
      else
      {
        RowIterator endi=end();
        for (RowIterator i=begin(); i!=endi; ++i)
        {
          ColIterator endj = (*i).end();
          for (ColIterator j=(*i).begin(); j!=endj; ++j)
            (*j) *= k;
        }
      }

      return *this;
    }

    //! vector space division by scalar
    BCRSMatrix& operator/= (const field_type& k)
    {
      if (nnz>0)
      {
        // process 1D array
        for (size_type i=0; i<nnz; i++)
          a[i] /= k;
      }
      else
      {
        RowIterator endi=end();
        for (RowIterator i=begin(); i!=endi; ++i)
        {
          ColIterator endj = (*i).end();
          for (ColIterator j=(*i).begin(); j!=endj; ++j)
            (*j) /= k;
        }
      }

      return *this;
    }


    //===== linear maps

    //! y += A x
    template<class X, class Y>
    void umv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).umv(x[j.index()],y[i.index()]);
      }
    }

    //! y -= A x
    template<class X, class Y>
    void mmv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).mmv(x[j.index()],y[i.index()]);
      }
    }

    //! y += alpha A x
    template<class X, class Y>
    void usmv (const field_type& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).usmv(alpha,x[j.index()],y[i.index()]);
      }
    }

    //! y += A^T x
    template<class X, class Y>
    void umtv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).umtv(x[i.index()],y[j.index()]);
      }
    }

    //! y -= A^T x
    template<class X, class Y>
    void mmtv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).mmtv(x[i.index()],y[j.index()]);
      }
    }

    //! y += alpha A^T x
    template<class X, class Y>
    void usmtv (const field_type& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).usmtv(alpha,x[i.index()],y[j.index()]);
      }
    }

    //! y += A^H x
    template<class X, class Y>
    void umhv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).umhv(x[i.index()],y[j.index()]);
      }
    }

    //! y -= A^H x
    template<class X, class Y>
    void mmhv (const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).mmhv(x[i.index()],y[j.index()]);
      }
    }

    //! y += alpha A^H x
    template<class X, class Y>
    void usmhv (const field_type& alpha, const X& x, Y& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (x.N()!=N()) DUNE_THROW(ISTLError,"index out of range");
      if (y.N()!=M()) DUNE_THROW(ISTLError,"index out of range");
#endif
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          (*j).usmhv(alpha,x[i.index()],y[j.index()]);
      }
    }


    //===== norms

    //! square of frobenius norm, need for block recursion
    double frobenius_norm2 () const
    {
      double sum=0;

      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          sum += (*j).frobenius_norm2();
      }

      return sum;
    }

    //! frobenius norm: sqrt(sum over squared values of entries)
    double frobenius_norm () const
    {
      return sqrt(frobenius_norm2());
    }

    //! infinity norm (row sum norm, how to generalize for blocks?)
    double infinity_norm () const
    {
      double max=0;
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        double sum=0;
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          sum += (*j).infinity_norm();
        max = std::max(max,sum);
      }
      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      double max=0;
      ConstRowIterator endi=end();
      for (ConstRowIterator i=begin(); i!=endi; ++i)
      {
        double sum=0;
        ConstColIterator endj = (*i).end();
        for (ConstColIterator j=(*i).begin(); j!=endj; ++j)
          sum += (*j).infinity_norm_real();
        max = std::max(max,sum);
      }
      return max;
    }


    //===== sizes

    //! number of blocks in row direction
    size_type N () const
    {
      return n;
    }

    //! number of blocks in column direction
    size_type M () const
    {
      return m;
    }

    //! row dimension of block r
    size_type rowdim (size_type i) const
    {
      return r[i].getptr()->rowdim();
    }

    //! col dimension of block c
    size_type coldim (size_type c) const
    {
      // find an entry in column j
      if (nnz>0)
      {
        for (size_type k=0; k<nnz; k++)
          if (j[k]==c) {
            return a[k].coldim();
          }
      }
      else
      {
        for (size_type i=0; i<n; i++)
        {
          size_type* j = r[i].getindexptr();
          B*   a = r[i].getptr();
          for (size_type k=0; k<r[i].getsize(); k++)
            if (j[k]==c) {
              return a[k].coldim();
            }
        }
      }

      // not found
      return 0;
    }

    //! dimension of the destination vector space
    size_type rowdim () const
    {
      size_type nn=0;
      for (size_type i=0; i<n; i++)
        nn += rowdim(i);
      return nn;
    }

    //! dimension of the source vector space
    size_type coldim () const
    {
      size_type mm=0;
      for (size_type i=0; i<m; i++)
        mm += coldim(i);
      return mm;
    }

    //===== query

    //! return true if (i,j) is in pattern
    bool exists (size_type i, size_type j) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
      if (j<0 || i>=m) DUNE_THROW(ISTLError,"index out of range");
#endif
      if (r[i].find(j)!=r[i].end())
        return true;
      else
        return false;
    }


  private:
    // state information
    BuildMode build_mode;     // row wise or whole matrix
    bool ready;               // true if matrix is ready to use

    // size of the matrix
    size_type n;       // number of rows
    size_type m;       // number of columns
    size_type nnz;     // number of nonzeros allocated in the a and j array below
    // zero means that memory is allocated seperately for each row.

    // the rows are dynamically allocated
    row_type* r;     // [n] the individual rows having pointers into a,j arrays

    // dynamically allocated memory
    B*   a;      // [nnz] non-zero entries of the matrix in row-wise ordering
    size_type* j;      // [nnz] column indices of entries
  };


#ifdef DUNE_EXPRESSIONTEMPLATES
  template <class B, class A>
  struct FieldType< BCRSMatrix<B,A> >
  {
    typedef typename FieldType<B>::type type;
  };

  template <class B, class A>
  struct BlockType< BCRSMatrix<B,A> >
  {
    typedef B type;
  };
  template <class B, class A>
  struct RowType< BCRSMatrix<B,A> >
  {
    typedef CompressedBlockVectorWindow<B,A> type;
  };
#endif

  /** @} end documentation */

} // end namespace

#endif
