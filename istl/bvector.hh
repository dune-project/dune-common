// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BVECTOR_HH
#define DUNE_BVECTOR_HH

#include <math.h>
#include <complex>

#include "istlexception.hh"
#include "allocator.hh"
#include "basearray.hh"

/*! \file

   \brief  This file implements a vector space as a tensor product of
   a given vector space. The number of components can be given at
   run-time.
 */

namespace Dune {

  /**
              @addtogroup ISTL
              @{
   */


  /** block_vector_unmanaged extends the base_array_unmanaged by
      vector operations such as addition and scalar multiplication.
          No memory management is added.

          Error checking: no error checking is provided normally.
          Setting the compile time switch DUNE_ISTL_WITH_CHECKING
          enables error checking.
   */
  template<class B, class A=ISTLAllocator>
  class block_vector_unmanaged : public base_array_unmanaged<B,A>
  {
  public:

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename B::field_type field_type;

    //! export the type representing the components
    typedef B block_type;

    //! export the allocator type
    typedef A allocator_type;

    //! make iterators available as types
    typedef typename base_array_unmanaged<B,A>::iterator Iterator;

    //! make iterators available as types
    typedef typename base_array_unmanaged<B,A>::const_iterator ConstIterator;


    //===== assignment from scalar

    //! Assignment from a scalar
    block_vector_unmanaged& operator= (const field_type& k)
    {
      for (int i=0; i<this->n; i++)
        (*this)[i] = k;
      return *this;
    }


    //===== vector space arithmetic

    //! vector space addition
    block_vector_unmanaged& operator+= (const block_vector_unmanaged& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (this->n!=y.N()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<this->n; ++i) (*this)[i] += y[i];
      return *this;
    }

    //! vector space subtraction
    block_vector_unmanaged& operator-= (const block_vector_unmanaged& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (this->n!=y.N()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<this->n; ++i) (*this)[i] -= y[i];
      return *this;
    }

    //! vector space multiplication with scalar
    block_vector_unmanaged& operator*= (const field_type& k)
    {
      for (int i=0; i<this->n; ++i) (*this)[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    block_vector_unmanaged& operator/= (const field_type& k)
    {
      for (int i=0; i<this->n; ++i) (*this)[i] /= k;
      return *this;
    }

    //! vector space axpy operation
    block_vector_unmanaged& axpy (const field_type& a, const block_vector_unmanaged& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (this->n!=y.N()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      for (int i=0; i<this->n; ++i) (*this)[i].axpy(a,y[i]);
      return *this;
    }


    //===== Euclidean scalar product

    //! scalar product
    field_type operator* (const block_vector_unmanaged& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (this->n!=y.N()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif
      field_type sum=0;
      for (int i=0; i<this->n; ++i) sum += (*this)[i]*y[i];
      return sum;
    }


    //===== norms

    //! one norm (sum over absolute values of entries)
    double one_norm () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (*this)[i].one_norm();
      return sum;
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    double one_norm_real () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (*this)[i].one_norm_real();
      return sum;
    }

    //! two norm sqrt(sum over squared values of entries)
    double two_norm () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (*this)[i].two_norm2();
      return sqrt(sum);
    }

    //! sqare of two norm (sum over squared values of entries), need for block recursion
    double two_norm2 () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (*this)[i].two_norm2();
      return sum;
    }

    //! infinity norm (maximum of absolute values of entries)
    double infinity_norm () const
    {
      double max=0;
      for (int i=0; i<this->n; ++i) max = std::max(max,(*this)[i].infinity_norm());
      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      double max=0;
      for (int i=0; i<this->n; ++i) max = std::max(max,(*this)[i].infinity_norm_real());
      return max;
    }


    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    int N () const
    {
      return this->n;
    }

    //! dimension of the vector space
    int dim () const
    {
      int d=0;
      for (int i=0; i<this->n; i++)
        d += (*this)[i].dim();
      return d;
    }

  protected:
    //! make constructor protected, so only derived classes can be instantiated
    block_vector_unmanaged () : base_array_unmanaged<B,A>()
    {       }
  };


  /** BlockVector adds memory management with ordinary
      copy semantics to the block_vector_unmanaged template.

          Error checking: no error checking is provided normally.
          Setting the compile time switch DUNE_ISTL_WITH_CHECKING
          enables error checking.
   */
  template<class B, class A=ISTLAllocator>
  class BlockVector : public block_vector_unmanaged<B,A>
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
    typedef typename block_vector_unmanaged<B,A>::Iterator Iterator;

    //! make iterators available as types
    typedef typename block_vector_unmanaged<B,A>::ConstIterator ConstIterator;

    //===== constructors and such

    //! makes empty vector
    BlockVector () : block_vector_unmanaged<B,A>()
    {       }

    //! make vector with _n components
    BlockVector (int _n)
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
    BlockVector (const BlockVector& a)
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
    BlockVector (const block_vector_unmanaged<B,A>& _a)
    {
      // upcast, because protected data inaccessible
      const BlockVector& a = static_cast<const BlockVector&>(_a);

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
    ~BlockVector ()
    {
      if (this->n>0) A::template free<B>(this->p);
    }

    //! reallocate vector to given size, any data is lost
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
    BlockVector& operator= (const BlockVector& a)
    {
      if (&a!=this)     // check if this and a are different objects
      {
        // adjust size of vector
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
    BlockVector& operator= (const block_vector_unmanaged<B,A>& a)
    {
      // forward to regular assignement operator
      return this->operator=(static_cast<const BlockVector&>(a));
    }

    //! assign from scalar
    BlockVector& operator= (const field_type& k)
    {
      // forward to operator= in base class
      (static_cast<block_vector_unmanaged<B,A>&>(*this)) = k;
      return *this;
    }
  };

  //! Send BlockVector to an output stream
  template<class K, class A>
  std::ostream& operator<< (std::ostream& s, const BlockVector<K, A>& v)
  {
    for (int i=0; i<v.size(); i++)
      s << v[i] << std::endl;

    return s;
  }

  /** BlockVectorWindow adds window manipulation functions
          to the block_vector_unmanaged template.

          This class has no memory management. It assumes that the storage
          for the entries of the vector is maintained outside of this class.

          But you can copy objects of this class and of the base class
      with reference semantics.

          Assignment copies the data, if the format is incopmpatible with
      the argument an exception is thrown in debug mode.

          Error checking: no error checking is provided normally.
          Setting the compile time switch DUNE_ISTL_WITH_CHECKING
          enables error checking.
   */
  template<class B, class A=ISTLAllocator>
  class BlockVectorWindow : public block_vector_unmanaged<B,A>
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
    typedef typename block_vector_unmanaged<B,A>::Iterator Iterator;

    //! make iterators available as types
    typedef typename block_vector_unmanaged<B,A>::ConstIterator ConstIterator;


    //===== constructors and such
    //! makes empty array
    BlockVectorWindow () : block_vector_unmanaged<B,A>()
    {       }

    //! make array from given pointer and size
    BlockVectorWindow (B* _p, int _n)
    {
      this->n = _n;
      this->p = _p;
    }

    //! copy constructor, this has reference semantics!
    BlockVectorWindow (const BlockVectorWindow& a)
    {
      this->n = a.n;
      this->p = a.p;
    }

    //! construct from base class object with reference semantics!
    BlockVectorWindow (const block_vector_unmanaged<B,A>& _a)
    {
      // cast needed to access protected data
      const BlockVectorWindow& a = static_cast<const BlockVectorWindow&>(_a);

      // make me point to the other's data
      this->n = a.n;
      this->p = a.p;
    }


    //! assignment
    BlockVectorWindow& operator= (const BlockVectorWindow& a)
    {
      // check correct size
#ifdef DUNE_ISTL_WITH_CHECKING
      if (this->n!=a.N()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif

      if (&a!=this)     // check if this and a are different objects
      {
        // copy data
        for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
      }
      return *this;
    }

    //! assign from base class object
    BlockVectorWindow& operator= (const block_vector_unmanaged<B,A>& a)
    {
      // forward to regular assignment operator
      return this->operator=(static_cast<const BlockVectorWindow&>(a));
    }

    //! assign from scalar
    BlockVectorWindow& operator= (const field_type& k)
    {
      (static_cast<block_vector_unmanaged<B,A>&>(*this)) = k;
      return *this;
    }


    //===== window manipulation methods

    //! set size and pointer
    void set (int _n, B* _p)
    {
      this->n = _n;
      this->p = _p;
    }

    //! set size only
    void setsize (int _n)
    {
      this->n = _n;
    }

    //! set pointer only
    void setptr (B* _p)
    {
      this->p = _p;
    }

    //! get pointer
    B* getptr ()
    {
      return this->p;
    }

    //! get size
    int getsize ()
    {
      return this->n;
    }
  };



  /** compressed_block_vector_unmanaged extends the compressed base_array_unmanaged by
      vector operations such as addition and scalar multiplication.
          No memory management is added.

          Error checking: no error checking is provided normally.
          Setting the compile time switch DUNE_ISTL_WITH_CHECKING
          enables error checking.
   */
  template<class B, class A=ISTLAllocator>
  class compressed_block_vector_unmanaged : public compressed_base_array_unmanaged<B,A>
  {
  public:

    //===== type definitions and constants

    //! export the type representing the field
    typedef typename B::field_type field_type;

    //! export the type representing the components
    typedef B block_type;

    //! export the allocator type
    typedef A allocator_type;

    //! make iterators available as types
    typedef typename compressed_base_array_unmanaged<B,A>::iterator Iterator;

    //! make iterators available as types
    typedef typename compressed_base_array_unmanaged<B,A>::const_iterator ConstIterator;


    //===== assignment from scalar

    compressed_block_vector_unmanaged& operator= (const field_type& k)
    {
      for (int i=0; i<this->n; i++)
        (this->p)[i] = k;
      return *this;
    }


    //===== vector space arithmetic

    //! vector space addition
    template<class V>
    compressed_block_vector_unmanaged& operator+= (const V& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!includesindexset(y)) DUNE_THROW(ISTLError,"index set mismatch");
#endif
      for (int i=0; i<this->n; ++i) (this->p)[i] += y[(this->j)[i]];
      return *this;
    }

    //! vector space subtraction
    template<class V>
    compressed_block_vector_unmanaged& operator-= (const V& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!includesindexset(y)) DUNE_THROW(ISTLError,"index set mismatch");
#endif
      for (int i=0; i<this->n; ++i) (this->p)[i] -= y[(this->j)[i]];
      return *this;
    }

    //! vector space axpy operation
    template<class V>
    compressed_block_vector_unmanaged& axpy (const field_type& a, const V& y)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!includesindexset(y)) DUNE_THROW(ISTLError,"index set mismatch");
#endif
      for (int i=0; i<this->n; ++i) (this->p)[i].axpy(a,y[(this->j)[i]]);
      return *this;
    }

    //! vector space multiplication with scalar
    compressed_block_vector_unmanaged& operator*= (const field_type& k)
    {
      for (int i=0; i<this->n; ++i) (this->p)[i] *= k;
      return *this;
    }

    //! vector space division by scalar
    compressed_block_vector_unmanaged& operator/= (const field_type& k)
    {
      for (int i=0; i<this->n; ++i) (this->p)[i] /= k;
      return *this;
    }


    //===== Euclidean scalar product

    //! scalar product
    field_type operator* (const compressed_block_vector_unmanaged& y) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (!includesindexset(y)) DUNE_THROW(ISTLError,"index set mismatch");
#endif
      field_type sum=0;
      for (int i=0; i<this->n; ++i)
        sum += (this->p)[i] * y[(this->j)[i]];
      return sum;
    }


    //===== norms

    //! one norm (sum over absolute values of entries)
    double one_norm () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (this->p)[i].one_norm();
      return sum;
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    double one_norm_real () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (this->p)[i].one_norm_real();
      return sum;
    }

    //! two norm sqrt(sum over squared values of entries)
    double two_norm () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (this->p)[i].two_norm2();
      return sqrt(sum);
    }

    //! sqare of two norm (sum over squared values of entries), need for block recursion
    double two_norm2 () const
    {
      double sum=0;
      for (int i=0; i<this->n; ++i) sum += (this->p)[i].two_norm2();
      return sum;
    }

    //! infinity norm (maximum of absolute values of entries)
    double infinity_norm () const
    {
      double max=0;
      for (int i=0; i<this->n; ++i) max = std::max(max,(this->p)[i].infinity_norm());
      return max;
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    double infinity_norm_real () const
    {
      double max=0;
      for (int i=0; i<this->n; ++i) max = std::max(max,(this->p)[i].infinity_norm_real());
      return max;
    }


    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    int N () const
    {
      return this->n;
    }

    //! dimension of the vector space
    int dim () const
    {
      int d=0;
      for (int i=0; i<this->n; i++)
        d += (this->p)[i].dim();
      return d;
    }

  protected:
    //! make constructor protected, so only derived classes can be instantiated
    compressed_block_vector_unmanaged () : compressed_base_array_unmanaged<B,A>()
    {       }

    //! return true if index sets coincide
    template<class V>
    bool includesindexset (const V& y)
    {
      typename V::Iterator e=y.end();
      for (int i=0; i<this->n; i++)
        if (y.find((this->j)[i])==e)
          return false;
      return true;
    }
  };


  /** CompressedBlockVectorWindow adds window manipulation functions
          to the compressed_block_vector_unmanaged template.

          This class has no memory management. It assumes that the storage
          for the entries of the vector and its index set is maintained outside of this class.

          But you can copy objects of this class and of the base class
      with reference semantics.

          Assignment copies the data, if the format is incopmpatible with
      the argument an exception is thrown in debug mode.

          Error checking: no error checking is provided normally.
          Setting the compile time switch DUNE_ISTL_WITH_CHECKING
          enables error checking.
   */
  template<class B, class A=ISTLAllocator>
  class CompressedBlockVectorWindow : public compressed_block_vector_unmanaged<B,A>
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
    typedef typename compressed_block_vector_unmanaged<B,A>::Iterator Iterator;

    //! make iterators available as types
    typedef typename compressed_block_vector_unmanaged<B,A>::ConstIterator ConstIterator;


    //===== constructors and such
    //! makes empty array
    CompressedBlockVectorWindow () : compressed_block_vector_unmanaged<B,A>()
    {       }

    //! make array from given pointers and size
    CompressedBlockVectorWindow (B* _p, int* _j, int _n)
    {
      this->n = _n;
      this->p = _p;
      this->j = _j;
    }

    //! copy constructor, this has reference semantics!
    CompressedBlockVectorWindow (const CompressedBlockVectorWindow& a)
    {
      this->n = a.n;
      this->p = a.p;
      this->j = a.j;
    }

    //! construct from base class object with reference semantics!
    CompressedBlockVectorWindow (const compressed_block_vector_unmanaged<B,A>& _a)
    {
      // cast needed to access protected data (upcast)
      const CompressedBlockVectorWindow& a = static_cast<const CompressedBlockVectorWindow&>(_a);

      // make me point to the other's data
      this->n = a.n;
      this->p = a.p;
      this->j = a.j;
    }


    //! assignment
    CompressedBlockVectorWindow& operator= (const CompressedBlockVectorWindow& a)
    {
      // check correct size
#ifdef DUNE_ISTL_WITH_CHECKING
      if (this->n!=a.N()) DUNE_THROW(ISTLError,"vector size mismatch");
#endif

      if (&a!=this)     // check if this and a are different objects
      {
        // copy data
        for (int i=0; i<this->n; i++) this->p[i]=a.p[i];
        for (int i=0; i<this->n; i++) this->j[i]=a.j[i];
      }
      return *this;
    }

    //! assign from base class object
    CompressedBlockVectorWindow& operator= (const compressed_block_vector_unmanaged<B,A>& a)
    {
      // forward to regular assignment operator
      return this->operator=(static_cast<const CompressedBlockVectorWindow&>(a));
    }

    //! assign from scalar
    CompressedBlockVectorWindow& operator= (const field_type& k)
    {
      (static_cast<compressed_block_vector_unmanaged<B,A>&>(*this)) = k;
      return *this;
    }


    //===== window manipulation methods

    //! set size and pointer
    void set (int _n, B* _p, int* _j)
    {
      this->n = _n;
      this->p = _p;
      this->j = _j;
    }

    //! set size only
    void setsize (int _n)
    {
      this->n = _n;
    }

    //! set pointer only
    void setptr (B* _p)
    {
      this->p = _p;
    }

    //! set pointer only
    void setindexptr (int* _j)
    {
      this->j = _j;
    }

    //! get pointer
    B* getptr ()
    {
      return this->p;
    }

    //! get pointer
    int* getindexptr ()
    {
      return this->j;
    }

    //! get size
    int getsize ()
    {
      return this->n;
    }
  };



  /** @} end documentation */

} // end namespace

#endif
