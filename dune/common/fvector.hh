// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_FVECTOR_HH
#define DUNE_FVECTOR_HH

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <complex>
#include <cstring>

#include "exceptions.hh"
#include "array.hh"
#include "densevector.hh"

namespace Dune {

  // forward declaration of template
  template<class K, int SIZE> class FieldVector;

  template<class K, int SIZE>
  struct FieldTraits< FieldVector<K,SIZE> >
  {
    typedef const typename FieldTraits<K>::field_type field_type;
    typedef const typename FieldTraits<K>::real_type real_type;
  };

  /** @defgroup DenseMatVec Dense Matrix and Vector Template Library
      @ingroup Common
      @{
   */

  /*! \file
   * \brief This file implements a vector constructed from a given type
     representing a field and a compile-time given size.
   */

  /** \brief Construct a vector space out of a tensor product of fields.
   *
   *  K is the field type (use float, double, complex, etc) and SIZE
   *  is the number of components.
   *
   *  It is generally assumed that K is a numerical type compatible with double
   *  (E.g. norms are always computed in double precision).
   */
  template< class K, int SIZE >
  class FieldVector : public DenseVector< Dune::array<K,SIZE> >
  {
  public:
    //! export size
    enum {
      //! The size of this vector.
      dimension = SIZE,
      //! The size of this vector.
      size = SIZE
    };

    typedef typename DenseVector< Dune::array<K,SIZE> >::size_type size_type;

    //! Constructor making uninitialized vector
    FieldVector() {}

    //! Constructor making vector with identical coordinates
    explicit FieldVector (const K& t)
    {
      for (size_type i=0; i<SIZE; i++) (*this)[i] = t;
    }

    //! Constructor making vector with identical coordinates
    FieldVector (const DenseVector< Dune::array<K,SIZE> > & x) :
      DenseVector< Dune::array<K,SIZE> > (x)
    {}

    using DenseVector< Dune::array<K,SIZE> >::operator=;
  };

  /** \brief Read a FieldVector from an input stream
   *  \relates FieldVector
   *
   *  \note This operator is STL compilant, i.e., the content of v is only
   *        changed if the read operation is successful.
   *
   *  \param[in]  in  std :: istream to read from
   *  \param[out] v   FieldVector to be read
   *
   *  \returns the input stream (in)
   */
  template<class K, int SIZE>
  inline std::istream &operator>> ( std::istream &in,
                                    FieldVector<K, SIZE> &v )
  {
    FieldVector<K, SIZE> w;
    for( typename FieldVector<K, SIZE>::size_type i = 0; i < SIZE; ++i )
      in >> w[ i ];
    if(in)
      v = w;
    return in;
  }

  // forward declarations
  template<class K, int n, int m> class FieldMatrix;

#ifndef DOXYGEN
  /** \brief Vectors containing only one component
   */
  template< class K >
  class FieldVector< K, 1 >
  {
    enum { n = 1 };
  public:
    friend class FieldMatrix<K,1,1>;

    //===== type definitions and constants

    //! export the type representing the field
    typedef K field_type;

    //! export the type representing the components
    typedef K block_type;

    //! The type for the index access and size operations.
    typedef std::size_t size_type;

    //! We are at the leaf of the block recursion
    enum {blocklevel = 1};

    //! export size
    enum {size = 1};

    //! export size
    enum {dimension = 1};

    //===== construction

    /** \brief Default constructor */
    FieldVector ()
    {       }

    /** \brief Constructor with a given scalar */
    FieldVector (const K& k)
    {
      p = k;
    }

    /** \brief Assignment from the base type */
    FieldVector& operator= (const K& k)
    {
      p = k;
      return *this;
    }

    //===== access to components

    //! random access
    K& operator[] (size_type i)
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i != 0) DUNE_THROW(MathError,"index out of range");
#endif
      return p;
    }

    //! same for read only access
    const K& operator[] (size_type i) const
    {
#ifdef DUNE_ISTL_WITH_CHECKING
      if (i != 0) DUNE_THROW(MathError,"index out of range");
#endif
      return p;
    }

    //! Iterator class for sequential access
    typedef DenseIterator<FieldVector<K,1>,K> Iterator;
    //! typedef for stl compliant access
    typedef Iterator iterator;

    //! begin iterator
    Iterator begin ()
    {
      return Iterator(*this,0);
    }

    //! end iterator
    Iterator end ()
    {
      return Iterator(*this,n);
    }

    //! begin iterator
    Iterator rbegin ()
    {
      return Iterator(*this,n-1);
    }

    //! end iterator
    Iterator rend ()
    {
      return Iterator(*this,-1);
    }

    //! return iterator to given element or end()
    Iterator find (size_type i)
    {
      if (i<n)
        return Iterator(*this,i);
      else
        return Iterator(*this,n);
    }

    //! ConstIterator class for sequential access
    typedef DenseIterator<const FieldVector<K,1>,const K> ConstIterator;
    //! typedef for stl compliant access
    typedef ConstIterator const_iterator;

    //! begin ConstIterator
    ConstIterator begin () const
    {
      return ConstIterator(*this,0);
    }

    //! end ConstIterator
    ConstIterator end () const
    {
      return ConstIterator(*this,n);
    }

    //! begin ConstIterator
    ConstIterator rbegin () const
    {
      return ConstIterator(*this,n-1);
    }

    //! end ConstIterator
    ConstIterator rend () const
    {
      return ConstIterator(*this,-1);
    }

    //! return iterator to given element or end()
    ConstIterator find (size_type i) const
    {
      if (i<n)
        return ConstIterator(*this,i);
      else
        return ConstIterator(*this,n);
    }
    //===== vector space arithmetic

    //! vector space add scalar to each comp
    FieldVector& operator+= (const K& k)
    {
      p += k;
      return *this;
    }

    //! vector space subtract scalar from each comp
    FieldVector& operator-= (const K& k)
    {
      p -= k;
      return *this;
    }

    //! vector space multiplication with scalar
    FieldVector& operator*= (const K& k)
    {
      p *= k;
      return *this;
    }

    //! vector space division by scalar
    FieldVector& operator/= (const K& k)
    {
      p /= k;
      return *this;
    }

    //! vector space axpy operation ( *this += a y )
    FieldVector& axpy (const K& a, const FieldVector& y)
    {
      p += a*y.p;
      return *this;
    }

    //! scalar product (x^T y)
    inline K operator* ( const K & k ) const
    {
      return p * k;
    }

    //===== norms

    //! one norm (sum over absolute values of entries)
    typename FieldTraits<K>::real_type one_norm () const
    {
      return std::abs(p);
    }

    //! simplified one norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type one_norm_real () const
    {
      return fvmeta::absreal(p);
    }

    //! two norm sqrt(sum over squared values of entries)
    typename FieldTraits<K>::real_type two_norm () const
    {
      return fvmeta::sqrt(fvmeta::abs2(p));
    }

    //! square of two norm (sum over squared values of entries), need for block recursion
    typename FieldTraits<K>::real_type two_norm2 () const
    {
      return fvmeta::abs2(p);
    }

    //! infinity norm (maximum of absolute values of entries)
    typename FieldTraits<K>::field_type infinity_norm () const
    {
      return std::abs(p);
    }

    //! simplified infinity norm (uses Manhattan norm for complex values)
    typename FieldTraits<K>::real_type infinity_norm_real () const
    {
      return fvmeta::absreal(p);
    }

    //===== sizes

    //! number of blocks in the vector (are of size 1 here)
    size_type N () const
    {
      return 1;
    }

    //! dimension of the vector space (==1)
    size_type dim () const
    {
      return 1;
    }

    //===== conversion operator

    /** \brief Conversion operator */
    operator K () {return p;}

    /** \brief Const conversion operator */
    operator K () const {return p;}

  private:
    // the data
    K p;
  };

  //! Binary vector addition
  template<class K>
  inline FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z+=b);
  }

  //! Binary vector subtraction
  template<class K>
  inline FieldVector<K,1> operator- (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z-=b);
  }

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const K b)
  {
    FieldVector<K,1> z = a;
    return (z[0]+=b);
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator- (const FieldVector<K,1>& a, const K b)
  {
    FieldVector<K,1> z = a;
    return (z[0]-=b);
  }

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator+ (const K a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z[0]+=b);
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator- (const K a, const FieldVector<K,1>& b)
  {
    FieldVector<K,1> z = a;
    return (z[0]-=b);
  }
#endif

  /** @} end documentation */

} // end namespace

#endif
