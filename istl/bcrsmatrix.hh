// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BCRSMATRIX_HH__
#define __DUNE_BCRSMATRIX_HH__

#include <math.h>
#include <complex>
#include <set>

#include "istl.hh"

/*! \file __FILE__

   This file implements a vector constructed from a given type
   representing a field and a compile-time given size.
 */

namespace Dune {

  /** @defgroup ISTL Iterative Solvers Template Library
              @addtogroup ISTL
              @{
   */


  /**! implements a block compressed row storage scheme. The block
       type B can be any type implementing the matrix interface.

           Error checking: no error checking is provided normally.
           Setting the compile time switch DUNE_ISTL_WITH_CHECKING
           enables error checking.
   */
  template<class B, class A=ISTL_Allocator>
  class BCRSMatrix
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

    //! forward declaration
    class CreateIterator;

    //===== the row type

    //! class implementing one row of the matrix
    class row_type
    {
    public:

      //! allow the enclosing class complete access
      friend class CreateIterator;

      //! return number of this row
      int index ()
      {
        return i;
      }

      //! return number of nonzeros in this row
      int size ()
      {
        return s;
      }


      //===== iterator interface to row entries

      //! Iterator class for sequential access
      class Iterator
      {
      public:
        //! constructor
        Iterator (B* _a, int* _j, int _k)
        {
          a = _a;
          j = _j;
          k = _k;
        }

        //! prefix increment
        Iterator& operator++()
        {
          ++k;
          return *this;
        }

        //! equality
        bool operator== (const Iterator& it) const
        {
          return (a+k)==(it.a+it.k);
        }

        //! inequality
        bool operator!= (const Iterator& it) const
        {
          return (a+k)!=(it.a+it.k);
        }

        //! dereferencing
        B& operator* ()
        {
          return a[k];
        }

        //! arrow
        B* operator-> ()
        {
          return a+k;
        }

        //! return index
        int index ()
        {
          return j[k];
        }

      private:
        B*   a;         // matrix entries
        int* j;         // corresponding column indices
        int k;          // counter
      };

      //! begin iterator
      Iterator begin ()
      {
        return Iterator(a,j,0);
      }

      //! end iterator
      Iterator end ()
      {
        return Iterator(a,j,s);
      }


      //===== random access interface to row entries

      //! random access to an entry in the row
      B& operator[] (int i)
      {
#ifdef DUNE_ISTL_WITH_CHECKING
        if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
        // replace this by binary search later ...
        for (int k=0; k<s; k++)
          if (j[k]==i)
            return a[k];

        DUNE_THROW(ISTLError,"index out of range");
      }

      //! random access to an entry in the row (read only)
      const B& operator[] (int i) const
      {
#ifdef DUNE_ISTL_WITH_CHECKING
        if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
        // replace this by binary search later ...
        for (int k=0; k<s; k++)
          if (j[k]==i)
            return a[k];

        DUNE_THROW(ISTLError,"index out of range");
      }


    private:
      //! recommended: use the set method to set internals
      void set (int _i, int _s, B* _a, int* _j)
      {
        i = _i;
        s = _s;
        a = _a;
        j = _j;
      }

      int i;      // this is row i;
      int s;      // having s entries
      B*   a;     // non-zero entries of row i
      int* j;     // the column indices
    };


    //===== random access interface to rows of the matrix

    //! random access to the rows
    row_type& operator[] (int i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return r[i];
    }

    //! same for read only access
    const row_type& operator[] (int i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i<0 || i>=n) DUNE_THROW(ISTLError,"index out of range");
#endif
      return r[i];
    }


    //===== iterator interface to rows of the matrix

    //! Iterator access to rows
    class Iterator
    {
    public:
      //! constructor
      Iterator (row_type* _p, int _i)
      {
        p = _p;
        i = _i;
      }

      //! prefix increment
      Iterator& operator++()
      {
        ++i;
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

      //! dereferencing
      row_type& operator* ()
      {
        return p[i];
      }

      //! arrow
      row_type* operator-> ()
      {
        return p+i;
      }

      //! return index
      int index ()
      {
        return i;
      }

    private:
      row_type* p;
      int i;
    };

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(r,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(r,n);
    }

    //! rename the iterators for easier access
    typedef Iterator RowIterator;
    typedef typename row_type::Iterator ColIterator;



    //===== creation interface

    //! we support two modes
    enum AllocMode {row_wise, whole_matrix, unknown};


    //! Iterator class for sequential creation of blocks
    class CreateIterator
    {
    public:
      //! constructor
      CreateIterator (BCRSMatrix& _Mat, int _i) : Mat(_Mat)
      {
        i = _i;
        nnz = 0;
        Mat.n_ready = 0;
      }

      //! prefix increment
      CreateIterator& operator++()
      {
        // row i is defined through the pattern
        // get memory for the row and initialize the j array
        // this depends on the allocation mode

        // compute size of the row
        int s = pattern.size();
        nnz += s;

        // memory for the row has been allocated already
        if (Mat.alloc_mode==whole_matrix)
        {
          // check if memory is sufficient
          if (nnz>Mat.nnz)
            DUNE_THROW(ISTLError,"allocated nnz too small");

          // set row i
          if (i==0)
            Mat.r[i].set(i,s,Mat.a,Mat.j);
          else
            Mat.r[i].set(i,s,Mat.r[i-1].a+Mat.r[i-1].s,Mat.r[i-1].j+Mat.r[i-1].s);
        }

        // memory for the row has to be allocated here
        if (Mat.alloc_mode==row_wise)
        {
          // allocate and set row i
          if (s>0)
          {
            B*   a = A::template malloc<B>(s);
            int* j = A::template malloc<int>(s);
            Mat.r[i].set(i,s,a,j);
          }
          else
            Mat.r[i].set(i,s,0,0);

          // increment nnz
          Mat.nnz += s;
        }

        // initialize the j array for row i from pattern
        int k=0;
        for (std::set<int>::const_iterator it=pattern.begin(); it!=pattern.end(); ++it)
          Mat.r[i].j[k++] = *it;

        // now go to next row
        i++;
        Mat.n_ready++;
        pattern.clear();

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
      int index ()
      {
        return i;
      }

      //! put column index in row
      void insert (int j)
      {
        pattern.insert(j);
      }

      //! return true if column index is in row
      bool contains (int j)
      {
        if (pattern.find(j)!=pattern.end())
          return true;
        else
          return false;
      }

    private:
      BCRSMatrix& Mat;     // the matrix we are defining
      int i;               // current row to be defined
      std::set<int> pattern;     // used to compile entries in a row
      int nnz;             // count total number of nonzeros
    };


    //! allow CreateIterator to access internal data
    friend class CreateIterator;

    //! get initial create iterator
    CreateIterator createbegin ()
    {
      if (create)
        DUNE_THROW(ISTLError,"creation already started");
      create = true;
      return CreateIterator(*this,0);
    }

    //! get create iterator pointing to one after the last block
    CreateIterator createend ()
    {
      return CreateIterator(*this,n);
    }


    //===== constructors & resizers

    // an empty matrix
    BCRSMatrix ()
    {
      // the state
      alloc_mode = unknown;
      create = false;
      n_ready = 0;

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

    // matrix with known number of nonzeroes
    BCRSMatrix (int _n, int _m, int _nnz)
    {
      // the state
      alloc_mode = whole_matrix;
      create = false;
      n_ready = 0;

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
        j = A::template malloc<int>(nnz);
      }
      else
      {
        a = 0;
        j = 0;
      }
    }

    // matrix with unknown number of nonzeroes
    BCRSMatrix (int _n, int _m)
    {
      // the state
      alloc_mode = row_wise;
      create = false;
      n_ready = 0;

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


  private:
    // state information
    AllocMode alloc_mode;     // row wise or whole matrix
    bool create;              // true if a CreateIterator has been created
    int n_ready;              // number of rows that are ready to use

    // size of the matrix
    int n;       // number of rows
    int m;       // number of columns
    int nnz;     // number of nonzeros in the matrix

    // the rows are dynamically allocated
    row_type* r;     // [n] the individual rows having pointers into a,j arrays

    // dynamically allocated memory
    B*   a;      // [nnz] non-zero entries of the matrix in row-wise ordering
    int* j;      // [nnz] column indices of entries
  };


  /** @} end documentation */

} // end namespace

#endif
