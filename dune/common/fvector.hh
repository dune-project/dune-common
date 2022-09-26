// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_FVECTOR_HH
#define DUNE_FVECTOR_HH

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <complex>
#include <cstring>
#include <utility>
#include <initializer_list>
#include <algorithm>

#include "typetraits.hh"
#include "exceptions.hh"

#include "ftraits.hh"
#include "densevector.hh"
#include "boundschecking.hh"

#include <dune/common/math.hh>
#include <dune/common/promotiontraits.hh>

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
    typedef std::array<K,SIZE> container_type;
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
    /**
     * \brief True if C is not of type FieldVector or its dimension
     * is not equal SIZE.
     */
    constexpr static bool value = true;
  };

  template<typename T, int SIZE>
  struct IsFieldVectorSizeCorrect<FieldVector<T,SIZE>,SIZE>
  {
    constexpr static bool value = true;
  };

  template<typename T, int SIZE, int SIZE1>
  struct IsFieldVectorSizeCorrect<FieldVector<T,SIZE1>,SIZE>
  {
    constexpr static bool value = false;
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
    std::array<K,SIZE> _data;
    typedef DenseVector< FieldVector<K,SIZE> > Base;
  public:
    //! The size of this vector.
    constexpr static int dimension = SIZE;

    typedef typename Base::size_type size_type;
    typedef typename Base::value_type value_type;

    /** \brief The type used for references to the vector entry */
    typedef value_type& reference;

    /** \brief The type used for const references to the vector entry */
    typedef const value_type& const_reference;

    //! Constructor making default-initialized vector
    constexpr FieldVector()
      : _data{{}}
    {}

    //! Constructor making vector with identical coordinates
    explicit FieldVector (const K& t)
    {
      std::fill(_data.begin(),_data.end(),t);
    }

#if __GNUC__ == 5 && !defined(__clang__)
    // `... = default;` causes an internal compiler error on GCC 5.4 (Ubuntu 16.04)
    //! copy constructor
    FieldVector(const FieldVector& x) : _data(x._data) {}
#else
    //! Copy constructor
    FieldVector (const FieldVector&) = default;
#endif

    /** \brief Construct from a std::initializer_list */
    FieldVector (std::initializer_list<K> const &l)
    {
      assert(l.size() == dimension);// Actually, this is not needed any more!
      std::copy_n(l.begin(), std::min(static_cast<std::size_t>(dimension),
                                      l.size()),
                 _data.begin());
    }

    //! copy assignment operator
    FieldVector& operator= (const FieldVector&) = default;

    template <typename T>
    FieldVector& operator= (const FieldVector<T, SIZE>& x)
    {
      std::copy_n(x.begin(), SIZE, _data.begin());
      return *this;
    }

    template<typename T, int N>
    FieldVector& operator=(const FieldVector<T, N>&) = delete;

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
    FieldVector (const DenseVector<C> & x,
                 [[maybe_unused]] typename std::enable_if<IsFieldVectorSizeCorrect<C,SIZE>::value>::type* dummy=0)
    {
      // do a run-time size check, for the case that x is not a FieldVector
      assert(x.size() == SIZE); // Actually this is not needed any more!
      std::copy_n(x.begin(), std::min(static_cast<std::size_t>(SIZE),x.size()), _data.begin());
    }

    //! Constructor making vector with identical coordinates
    template<class K1>
    explicit FieldVector (const FieldVector<K1,SIZE> & x)
    {
      std::copy_n(x.begin(), SIZE, _data.begin());
    }

    template<typename T, int N>
    explicit FieldVector(const FieldVector<T, N>&) = delete;

    using Base::operator=;

    // make this thing a vector
    static constexpr size_type size () { return SIZE; }

    K & operator[](size_type i) {
      DUNE_ASSERT_BOUNDS(i < SIZE);
      return _data[i];
    }
    const K & operator[](size_type i) const {
      DUNE_ASSERT_BOUNDS(i < SIZE);
      return _data[i];
    }

    //! return pointer to underlying array
    K* data() noexcept
    {
      return _data.data();
    }

    //! return pointer to underlying array
    const K* data() const noexcept
    {
      return _data.data();
    }

    //! vector space multiplication with scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator* ( const FieldVector& vector, Scalar scalar)
    {
      FieldVector<typename PromotionTraits<value_type,Scalar>::PromotedType,SIZE> result;

      for (size_type i = 0; i < vector.size(); ++i)
        result[i] = vector[i] * scalar;

      return result;
    }

    //! vector space multiplication with scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator* ( Scalar scalar, const FieldVector& vector)
    {
      FieldVector<typename PromotionTraits<value_type,Scalar>::PromotedType,SIZE> result;

      for (size_type i = 0; i < vector.size(); ++i)
        result[i] = scalar * vector[i];

      return result;
    }

    //! vector space division by scalar
    template <class Scalar,
              std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend auto operator/ ( const FieldVector& vector, Scalar scalar)
    {
      FieldVector<typename PromotionTraits<value_type,Scalar>::PromotedType,SIZE> result;

      for (size_type i = 0; i < vector.size(); ++i)
        result[i] = vector[i] / scalar;

      return result;
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
    //! The size of this vector.
    constexpr static int dimension = 1;

    typedef typename Base::size_type size_type;

    /** \brief The type used for references to the vector entry */
    typedef K& reference;

    /** \brief The type used for const references to the vector entry */
    typedef const K& const_reference;

    //===== construction

    /** \brief Default constructor */
    constexpr FieldVector ()
      : _data()
    {}

    /** \brief Constructor with a given scalar */
    template<typename T,
             typename EnableIf = typename std::enable_if<
               std::is_convertible<T, K>::value &&
               ! std::is_base_of<DenseVector<typename FieldTraits<T>::field_type>, K
                                 >::value
               >::type
             >
    FieldVector (const T& k) : _data(k) {}

    //! Constructor from static vector of different type
    template<class C,
             std::enable_if_t<
               std::is_assignable<K&, typename DenseVector<C>::value_type>::value, int> = 0>
    FieldVector (const DenseVector<C> & x)
    {
      static_assert(((bool)IsFieldVectorSizeCorrect<C,1>::value), "FieldVectors do not match in dimension!");
      assert(x.size() == 1);
      _data = x[0];
    }

    //! copy constructor
    FieldVector(const FieldVector&) = default;

    //! copy assignment operator
    FieldVector& operator=(const FieldVector&) = default;

    template <typename T>
    FieldVector& operator= (const FieldVector<T, 1>& other)
    {
      _data = other[0];
      return *this;
    }

    template<typename T, int N>
    FieldVector& operator=(const FieldVector<T, N>&) = delete;

    /** \brief Construct from a std::initializer_list */
    FieldVector (std::initializer_list<K> const &l)
    {
      assert(l.size() == 1);
      _data = *l.begin();
    }

    //! Assignment operator for scalar
    template<typename T,
             typename EnableIf = typename std::enable_if<
               std::is_assignable<K&, T>::value &&
               ! std::is_base_of<DenseVector<typename FieldTraits<T>::field_type>, K
                                 >::value
               >::type
             >
    inline FieldVector& operator= (const T& k)
    {
      _data = k;
      return *this;
    }

    //===== forward methods to container
    static constexpr size_type size () { return 1; }

    K & operator[]([[maybe_unused]] size_type i)
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return _data;
    }
    const K & operator[]([[maybe_unused]] size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return _data;
    }

    //! return pointer to underlying array
    K* data() noexcept
    {
      return &_data;
    }

    //! return pointer to underlying array
    const K* data() const noexcept
    {
      return &_data;
    }

    //===== conversion operator

    /** \brief Conversion operator */
    operator K& () { return _data; }

    /** \brief Const conversion operator */
    operator const K& () const { return _data; }
  };

  /* ----- FV / FV ----- */
  /* mostly not necessary as these operations are already covered via the cast operator */

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

  /* Overloads for common classification functions */
  namespace MathOverloads {

    // ! Returns whether all entries are finite
    template<class K, int SIZE>
    auto isFinite(const FieldVector<K,SIZE> &b, PriorityTag<2>, ADLTag) {
      bool out = true;
      for(int i=0; i<SIZE; i++) {
        out &= Dune::isFinite(b[i]);
      }
      return out;
    }

    // ! Returns whether any entry is infinite
    template<class K, int SIZE>
    bool isInf(const FieldVector<K,SIZE> &b, PriorityTag<2>, ADLTag) {
      bool out = false;
      for(int i=0; i<SIZE; i++) {
        out |= Dune::isInf(b[i]);
      }
      return out;
    }

    // ! Returns whether any entry is NaN
    template<class K, int SIZE, typename = std::enable_if_t<HasNaN<K>::value>>
    bool isNaN(const FieldVector<K,SIZE> &b, PriorityTag<2>, ADLTag) {
      bool out = false;
      for(int i=0; i<SIZE; i++) {
        out |= Dune::isNaN(b[i]);
      }
      return out;
    }

    // ! Returns true if either b or c is NaN
    template<class K, typename = std::enable_if_t<HasNaN<K>::value>>
    bool isUnordered(const FieldVector<K,1> &b, const FieldVector<K,1> &c,
                     PriorityTag<2>, ADLTag) {
      return Dune::isUnordered(b[0],c[0]);
    }
  } //MathOverloads

  /** @} end documentation */

} // end namespace

#endif
