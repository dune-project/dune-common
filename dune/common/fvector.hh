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

  /** @addtogroup DenseMatVec
      @{
   */

  /*! \file
   * \brief This file implements a vector constructed from a given type
     representing a field and a compile-time given size.
   */

  template< class K, int SIZE > class FieldVector;
  template< class K, int SIZE >
  struct DenseMatVecTraits< FieldVector<K,SIZE> >
  {
    typedef FieldVector<K,SIZE> derived_type;
    typedef Dune::array<K,SIZE> container_type;
    typedef K value_type;
    typedef typename container_type::size_type size_type;
  };

  template< class K, int SIZE >
  struct FieldTraits< FieldVector<K,SIZE> >
  {
    typedef typename FieldTraits<K>::field_type field_type;
    typedef typename FieldTraits<K>::real_type real_type;
  };

  /** \brief vector space out of a tensor product of fields.
   *
   * \tparam K    the field type (use float, double, complex, etc)
   * \tparam SIZE number of components.
   */
  template< class K, int SIZE >
  class FieldVector :
    public DenseVector< FieldVector<K,SIZE> >
  {
    Dune::array<K,SIZE> _data;
    typedef DenseVector< FieldVector<K,SIZE> > Base;
  public:
    //! export size
    enum {
      //! The size of this vector.
      dimension = SIZE,
      //! The size of this vector.
      size = SIZE
    };

    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;

    //! Constructor making uninitialized vector
    FieldVector() {}

    //! Constructor making vector with identical coordinates
    explicit FieldVector (const K& t)
    {
      fill(t);
    }

    //! Constructor making vector with identical coordinates
    FieldVector (const FieldVector & x) : _data(x._data)
    {}

    using Base::operator=;

    // make this thing a vector
    size_type vec_size() const { return _data.size(); }
    K & vec_access(size_type i) { return _data[i]; }
    const K & vec_access(size_type i) const { return _data[i]; }
  private:
    void fill(const K& t)
    {
      for (int i=0; i<SIZE; i++) _data[i]=t;
    }
  };

  /** \brief Read a FieldVector from an input stream
   *  \relates FieldVector
   *
   *  \note This operator is STL compliant, i.e., the content of v is only
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

#ifndef DOXYGEN
  template< class K >
  struct DenseMatVecTraits< FieldVector<K,1> >
  {
    typedef FieldVector<K,1> derived_type;
    typedef K container_type;
    typedef K value_type;
    typedef size_t size_type;
  };

  /** \brief Vectors containing only one component
   */
  template<class K>
  class FieldVector<K, 1> :
    public DenseVector< FieldVector<K,1> >
  {
    K _data;
    typedef DenseVector< FieldVector<K,1> > Base;
  public:
    //! export size
    enum {
      //! The size of this vector.
      dimension = 1,
      //! The size of this vector.
      size = 1
    };

    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;

    //===== construction

    /** \brief Default constructor */
    FieldVector () {}

    /** \brief Constructor with a given scalar */
    FieldVector (const K& k) { (*this)[0] = k; }

    using Base::operator=;

    //===== forward methods to container
    size_type vec_size() const { return 1; }
    K & vec_access(size_type i)
    {
      assert(i == 0);
      return _data;
    }
    const K & vec_access(size_type i) const
    {
      assert(i == 0);
      return _data;
    }

    //===== conversion operator

    /** \brief Conversion operator */
    operator K () { return (*this)[0]; }

    /** \brief Const conversion operator */
    operator K () const { return (*this)[0]; }
  };

  //! Binary vector addition
  template<class K>
  inline FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    return a[0]+b[0];
  }

  //! Binary vector subtraction
  template<class K>
  inline FieldVector<K,1> operator- (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    return a[0]-b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator> (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    return a[0]>b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator>= (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    return a[0]>=b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator< (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    return a[0]<b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator<= (const FieldVector<K,1>& a, const FieldVector<K,1>& b)
  {
    return a[0]<=b[0];
  }

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const K b)
  {
    return a[0]+b;
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator- (const FieldVector<K,1>& a, const K b)
  {
    return a[0]-b;
  }


  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator> (const FieldVector<K,1>& a, const K b)
  {
    return a[0]>b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator>= (const FieldVector<K,1>& a, const K b)
  {
    return a[0]>=b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator< (const FieldVector<K,1>& a, const K b)
  {
    return a[0]<b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator<= (const FieldVector<K,1>& a, const K b)
  {
    return a[0]<=b;
  }

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator+ (const K a, const FieldVector<K,1>& b)
  {
    return a+b[0];
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator- (const K a, const FieldVector<K,1>& b)
  {
    return a-b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator> (const K a, const FieldVector<K,1>& b)
  {
    return a>b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator>= (const K a, const FieldVector<K,1>& b)
  {
    return a>=b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator< (const K a, const FieldVector<K,1>& b)
  {
    return a<b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator<= (const K a, const FieldVector<K,1>& b)
  {
    return a<=b[0];
  }
#endif

  /** @} end documentation */

} // end namespace

#endif
