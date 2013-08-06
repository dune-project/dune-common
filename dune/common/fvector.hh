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
#include <utility>

#include "typetraits.hh"
#include "exceptions.hh"
#include "array.hh"
#include "densevector.hh"
#include "static_assert.hh"

namespace Dune {

  /** @addtogroup DenseMatVec
      @{
   */

  /*! \file
   * \brief Implements a vector constructed from a given type
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

  /**
   * @brief TMP to check the size of a DenseVectors statically, if possible.
   *
   * If the implementation type of C is  a FieldVector, we statically check
   * whether its dimension is SIZE.
   * @tparam C The implementation of the other DenseVector
   * @tparam SIZE The size we need assume.
   */
  template<typename C, int SIZE>
  struct IsFieldVectorSizeCorrect
  {
    enum {
      /**
         *@param True if C is not of type FieldVector or its dimension
       * is not equal SIZE.
       */
      value = true
    };
  };

  template<typename T, int SIZE>
  struct IsFieldVectorSizeCorrect<FieldVector<T,SIZE>,SIZE>
  {
    enum {value = true};
  };

  template<typename T, int SIZE, int SIZE1>
  struct IsFieldVectorSizeCorrect<FieldVector<T,SIZE1>,SIZE>
  {
    enum {value = false};
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
      dimension = SIZE
    };

    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;

    //! Constructor making default-initialized vector
    FieldVector()
    // Use C++11 unified initialization if available - tends to generate
    // fastest code
#if HAVE_INITIALIZER_LIST
      : _data{}
    {}
#else
    {
      // fall back to library approach - this gives faster code than array placement
      // new. Apart from that, placement new may create problems if K is a complex
      // type. In that case, the default constructor of the _data elements has already
      // been called and may have allocated memory.
      std::fill(_data.begin(),_data.end(),K());
    }
#endif

    //! Constructor making vector with identical coordinates
    explicit FieldVector (const K& t)
    {
      fill(t);
    }

    //! Constructor making vector with identical coordinates
    FieldVector (const FieldVector & x) : _data(x._data)
    {}

    /**
     * \brief Copy constructor from a second vector of possibly different type
     *
     * If the DenseVector type of the this constructor's argument
     * is implemented by a FieldVector, it is statically checked
     * if it has the correct size. If this is not the case
     * the constructor is removed from the overload set using SFINAE.
     *
     * \param[in]  x  A DenseVector with correct size.
     * \param[in]  dummy  A void* dummy argument needed by SFINAE.
     */
    template<class C>
    FieldVector (const DenseVector<C> & x, typename Dune::enable_if<IsFieldVectorSizeCorrect<C,SIZE>::value>::type* dummy=0 )
    {
      // do a run-time size check, for the case that x is not a FieldVector
      assert(x.size() == SIZE);
      for (size_type i = 0; i<SIZE; i++)
        _data[i] = x[i];
    }

    //! Constructor making vector with identical coordinates
    template<class K1, int SIZE1>
    explicit FieldVector (const FieldVector<K1,SIZE1> & x)
    {
      dune_static_assert(SIZE1 == SIZE, "FieldVector in constructor has wrong size");
      for (size_type i = 0; i<SIZE; i++)
        _data[i] = x[i];
    }
    using Base::operator=;

    // make this thing a vector
    size_type vec_size() const { return SIZE; }
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
      dimension = 1
    };

    typedef typename Base::size_type size_type;

    //===== construction

    /** \brief Default constructor */
    FieldVector ()
      : _data()
    {}

    /** \brief Constructor with a given scalar */
    FieldVector (const K& k) : _data(k) {}

    //! Constructor making vector with identical coordinates
    template<class C>
    FieldVector (const DenseVector<C> & x)
    {
      dune_static_assert(((bool)IsFieldVectorSizeCorrect<C,1>::value), "FieldVectors do not match in dimension!");
      assert(x.size() == 1);
      _data = x[0];
    }

    //! Assignment operator for scalar
    inline FieldVector& operator= (const K& k)
    {
      _data = k;
      return *this;
    }

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
    operator K () { return _data; }

    /** \brief Const conversion operator */
    operator K () const { return _data; }
  };

  /* ----- FV / FV ----- */
  /* not necessary as these operations are already covered via the cast operator */

  /* ----- FV / scalar ----- */

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

  //! Binary multiplication, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator* (const FieldVector<K,1>& a, const K b)
  {
    return a[0]*b;
  }

  //! Binary division, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator/ (const FieldVector<K,1>& a, const K b)
  {
    return a[0]/b;
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

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator== (const FieldVector<K,1>& a, const K b)
  {
    return a[0]==b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator!= (const FieldVector<K,1>& a, const K b)
  {
    return a[0]!=b;
  }

  /* ----- scalar / FV ------ */

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

  //! Binary multiplication, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator* (const K a, const FieldVector<K,1>& b)
  {
    return a*b[0];
  }

  //! Binary division, when using FieldVector<K,1> like K
  template<class K>
  inline FieldVector<K,1> operator/ (const K a, const FieldVector<K,1>& b)
  {
    return a/b[0];
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

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator== (const K a, const FieldVector<K,1>& b)
  {
    return a==b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  inline bool operator!= (const K a, const FieldVector<K,1>& b)
  {
    return a!=b[0];
  }
#endif

  /** @} end documentation */

} // end namespace

#endif
