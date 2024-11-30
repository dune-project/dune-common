// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_FVECTOR_HH
#define DUNE_COMMON_FVECTOR_HH

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>
#include <initializer_list>

#include <dune/common/boundschecking.hh>
#include <dune/common/densevector.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/math.hh>
#include <dune/common/promotiontraits.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>

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

    //! Default constructor, making value-initialized vector with all components set to zero
    constexpr FieldVector () noexcept
      : _data{}
    {}

    //! Constructor making vector with identical coordinates
    explicit constexpr FieldVector (const K& k)
        noexcept(std::is_nothrow_copy_assignable_v<K>)
    {
      std::fill(_data.begin(), _data.end(), k);
    }

    //! Construct from a std::initializer_list
    constexpr FieldVector (const std::initializer_list<K>& l)
      : _data{}
    {
      assert(l.size() == dimension);
      for (int i = 0; i < dimension; ++i)
        _data[i] = std::data(l)[i];
    }

    //! Constructor from another dense vector if the elements are assignable to K
    template<class T,
      std::enable_if_t<IsFieldVectorSizeCorrect<T,dimension>::value, int> = 0,
      decltype(std::declval<K&>() = std::declval<const T&>()[0], bool{}) = true>
    FieldVector (const DenseVector<T>& x)
    {
      assert(x.size() == dimension);
      for (int i = 0; i < dimension; ++i)
        _data[i] = x[i];
    }

    //! Converting constructor from FieldVector with different element type
    template<class T,
      std::enable_if_t<std::is_assignable_v<K&, const T&>, int> = 0>
    explicit constexpr FieldVector (const FieldVector<T, SIZE>& x)
        noexcept(std::is_nothrow_assignable_v<K&, const T&>)
    {
      for (int i = 0; i < dimension; ++i)
        _data[i] = x[i];
    }

    //! Converting constructor with FieldVector of different size (deleted)
    template<class K1, int SIZE1,
      std::enable_if_t<(SIZE1 != SIZE), int> = 0>
    explicit FieldVector (const FieldVector<K1, SIZE1>&) = delete;

    //! Copy constructor with default behavior
    FieldVector (const FieldVector&) = default;


    //! Assignment from another dense vector
    template<class T,
      std::enable_if_t<IsFieldVectorSizeCorrect<T,dimension>::value, int> = 0,
      decltype(std::declval<K&>() = std::declval<const T&>()[0], bool{}) = true>
    FieldVector& operator= (const DenseVector<T>& x)
    {
      assert(x.size() == dimension);
      for (int i = 0; i < dimension; ++i)
        _data[i] = x[i];
      return *this;
    }

    //! Converting assignment operator from FieldVector with different element type
    template<class T,
      std::enable_if_t<std::is_assignable_v<K&, const T&>, int> = 0>
    FieldVector& operator= (const FieldVector<T, SIZE>& x)
        noexcept(std::is_nothrow_assignable_v<K&, const T&>)
    {
      for (int i = 0; i < dimension; ++i)
        _data[i] = x[i];
      return *this;
    }

    //! Converting assignment operator with FieldVector of different size (deleted)
    template<class K1, int SIZE1,
      std::enable_if_t<(SIZE1 != SIZE), int> = 0>
    FieldVector& operator= (const FieldVector<K1, SIZE1>&) = delete;

    //! Copy assignment operator with default behavior
    constexpr FieldVector& operator= (const FieldVector&) = default;

    using Base::operator=;

    //! Obtain the number of elements stored in the vector
    static constexpr size_type size () noexcept { return dimension; }

    //! Return a reference to the `i`th element
    reference operator[] (size_type i)
    {
      DUNE_ASSERT_BOUNDS(i < dimension);
      return _data[i];
    }

    //! Return a (const) reference to the `i`th element
    const_reference operator[] (size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i < dimension);
      return _data[i];
    }

    //! Return pointer to underlying array
    constexpr K* data () noexcept
    {
      return _data.data();
    }

    //! Return pointer to underlying array
    constexpr const K* data () const noexcept
    {
      return _data.data();
    }

    //! Vector space multiplication with scalar
    template<class Scalar,
      std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend constexpr auto operator* (const FieldVector& vector, Scalar scalar)
    {
      using T = typename PromotionTraits<value_type,Scalar>::PromotedType;
      FieldVector<T,SIZE> result;

      for (size_type i = 0; i < vector.size(); ++i)
        result[i] = vector[i] * scalar;
      return result;
    }

    //! Vector space multiplication with scalar
    template<class Scalar,
      std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend constexpr auto operator* (Scalar scalar, const FieldVector& vector)
    {
      using T = typename PromotionTraits<value_type,Scalar>::PromotedType;
      FieldVector<T,SIZE> result;

      for (size_type i = 0; i < vector.size(); ++i)
        result[i] = scalar * vector[i];
      return result;
    }

    //! Vector space division by scalar
    template<class Scalar,
      std::enable_if_t<IsNumber<Scalar>::value, int> = 0>
    friend constexpr auto operator/ (const FieldVector& vector, Scalar scalar)
    {
      using T = typename PromotionTraits<value_type,Scalar>::PromotedType;
      FieldVector<T,SIZE> result;

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
  std::istream &operator>> (std::istream& in, FieldVector<K, SIZE>& v)
  {
    FieldVector<K, SIZE> w;
    for (int i = 0; i < SIZE; ++i)
      in >> w[i];
    if (in)
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
    constexpr FieldVector () noexcept
      : _data()
    {}

    /** \brief Constructor with a given scalar */
    template<class T,
      std::enable_if_t<std::is_constructible_v<K,T>, int> = 0>
    constexpr FieldVector (const T& k) noexcept
      : _data(k)
    {}

    /** \brief Construct from a std::initializer_list */
    constexpr FieldVector (const std::initializer_list<K>& l)
    {
      assert(l.size() == 1);
      _data = *l.begin();
    }

    //! Constructor from static vector of different type
    template<class T,
      std::enable_if_t<std::is_constructible_v<K,T>, int> = 0>
    constexpr FieldVector (const FieldVector<T,1>& x) noexcept
      : _data(x[0])
    {}

    //! Constructor from other dense vector
    template<class T,
      std::enable_if_t<IsFieldVectorSizeCorrect<T,1>::value, int> = 0,
      decltype(std::declval<K&>() = std::declval<const T&>()[0], bool{}) = true>
    FieldVector (const DenseVector<T>& x)
    {
      assert(x.size() == 1);
      _data = x[0];
    }

    //! copy constructor
    constexpr FieldVector (const FieldVector&) = default;

    //! copy assignment operator
    constexpr FieldVector& operator= (const FieldVector&) = default;

    //! assignment from static vector of different type
    template<class T,
      decltype(std::declval<K&>() = std::declval<const T&>(), bool{}) = true>
    constexpr FieldVector& operator= (const FieldVector<T,1>& other) noexcept
    {
      _data = other[0];
      return *this;
    }

    //! assignment from other dense vector
    template<class T,
      std::enable_if_t<IsFieldVectorSizeCorrect<T,1>::value, int> = 0,
      decltype(std::declval<K&>() = std::declval<const T&>()[0], bool{}) = true>
    FieldVector& operator= (const DenseVector<T>& other)
    {
      assert(other.size() == 1);
      _data = other[0];
      return *this;
    }

    //! Assignment operator for scalar
    template<class T,
      decltype(std::declval<K&>() = std::declval<const T&>(), bool{}) = true>
    constexpr FieldVector& operator= (const T& k) noexcept
    {
      _data = k;
      return *this;
    }

    //===== forward methods to container
    static constexpr size_type size () noexcept { return 1; }

    reference operator[] ([[maybe_unused]] size_type i)
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return _data;
    }
    const_reference operator[] ([[maybe_unused]] size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i == 0);
      return _data;
    }

    //! return pointer to underlying array
    constexpr K* data () noexcept
    {
      return &_data;
    }

    //! return pointer to underlying array
    constexpr const K* data () const noexcept
    {
      return &_data;
    }

    //===== conversion operator

    /** \brief Conversion operator */
    constexpr operator reference () noexcept { return _data; }

    /** \brief Const conversion operator */
    constexpr operator const_reference () const noexcept { return _data; }
  };

  /* ----- FV / FV ----- */
  /* mostly not necessary as these operations are already covered via the cast operator */

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator> (const FieldVector<K,1>& a, const FieldVector<K,1>& b) noexcept
  {
    return a[0]>b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator>= (const FieldVector<K,1>& a, const FieldVector<K,1>& b) noexcept
  {
    return a[0]>=b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator< (const FieldVector<K,1>& a, const FieldVector<K,1>& b) noexcept
  {
    return a[0]<b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator<= (const FieldVector<K,1>& a, const FieldVector<K,1>& b) noexcept
  {
    return a[0]<=b[0];
  }

  /* ----- FV / scalar ----- */

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator+ (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]+b;
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator- (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]-b;
  }

  //! Binary multiplication, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator* (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]*b;
  }

  //! Binary division, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator/ (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]/b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator> (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]>b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator>= (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]>=b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator< (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]<b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator<= (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]<=b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator== (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]==b;
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator!= (const FieldVector<K,1>& a, const K b) noexcept
  {
    return a[0]!=b;
  }

  /* ----- scalar / FV ------ */

  //! Binary addition, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator+ (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a+b[0];
  }

  //! Binary subtraction, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator- (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a-b[0];
  }

  //! Binary multiplication, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator* (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a*b[0];
  }

  //! Binary division, when using FieldVector<K,1> like K
  template<class K>
  constexpr FieldVector<K,1> operator/ (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a/b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator> (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a>b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator>= (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a>=b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator< (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a<b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator<= (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a<=b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator== (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a==b[0];
  }

  //! Binary compare, when using FieldVector<K,1> like K
  template<class K>
  constexpr bool operator!= (const K a, const FieldVector<K,1>& b) noexcept
  {
    return a!=b[0];
  }
#endif

  /* Overloads for common classification functions */
  namespace MathOverloads {

    //! Returns whether all entries are finite
    template<class K, int SIZE>
    auto isFinite (const FieldVector<K,SIZE>& b, PriorityTag<2>, ADLTag)
    {
      bool out = true;
      for (int i = 0; i < SIZE; ++i) {
        out &= Dune::isFinite(b[i]);
      }
      return out;
    }

    //! Returns whether any entry is infinite
    template<class K, int SIZE>
    bool isInf (const FieldVector<K,SIZE>& b, PriorityTag<2>, ADLTag)
    {
      bool out = false;
      for (int i = 0; i < SIZE; ++i) {
        out |= Dune::isInf(b[i]);
      }
      return out;
    }

    //! Returns whether any entry is NaN
    template<class K, int SIZE,
      std::enable_if_t<HasNaN<K>::value, int> = 0>
    bool isNaN (const FieldVector<K,SIZE>& b, PriorityTag<2>, ADLTag)
    {
      bool out = false;
      for (int i = 0; i < SIZE; ++i) {
        out |= Dune::isNaN(b[i]);
      }
      return out;
    }

    //! Returns true if either b or c is NaN
    template<class K,
      std::enable_if_t<HasNaN<K>::value, int> = 0>
    bool isUnordered (const FieldVector<K,1>& b, const FieldVector<K,1>& c,
                      PriorityTag<2>, ADLTag)
    {
      return Dune::isUnordered(b[0],c[0]);
    }

  } // end namespace MathOverloads

  /** @} end documentation */

} // end namespace Dune

#endif // DUNE_COMMON_FVECTOR_HH
