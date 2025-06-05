// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_FVECTOR_HH
#define DUNE_COMMON_FVECTOR_HH

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>
#include <initializer_list>

#include <dune/common/boundschecking.hh>
#include <dune/common/densevector.hh>
#include <dune/common/filledarray.hh>
#include <dune/common/ftraits.hh>
#include <dune/common/math.hh>
#include <dune/common/promotiontraits.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/typeutilities.hh>
#include <dune/common/concepts/number.hh>
#include <dune/common/std/algorithm.hh>
#include <dune/common/std/compare.hh>

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
    using Base = DenseVector< FieldVector<K,SIZE> >;

    //! The container storage
    std::array<K,SIZE> _data;

  public:

    //! The size of this vector.
    static constexpr int dimension = SIZE;

    //! The type used for the index access and size operation
    using size_type = typename Base::size_type;

    //! The type of the elements stored in the vector
    using value_type = typename Base::value_type;

    //! The type used for references to the vector entries
    using reference = value_type&;

    //! The type used for const references to the vector entries
    using const_reference = const value_type&;

  public:

    //! Default constructor, making value-initialized vector with all components set to zero
    constexpr FieldVector ()
        noexcept(std::is_nothrow_default_constructible_v<K>)
      : _data{}
    {}

    //! Constructor with a given value initializing all entries to this value
    explicit(SIZE != 1)
    constexpr FieldVector (const value_type& value) noexcept
      : _data{filledArray<SIZE>(value)}
    {}

    //! Constructor with a given scalar initializing all entries to this value
    template<Concept::Number S>
      requires (std::constructible_from<K,S>)
    explicit(SIZE != 1)
    constexpr FieldVector (const S& scalar)
        noexcept(std::is_nothrow_constructible_v<K,S>)
      : _data{filledArray<SIZE,K>(K(scalar))}
    {}

    //! Construct from a std::initializer_list of values
    constexpr FieldVector (const std::initializer_list<K>& l)
      : _data{}
    {
      assert(l.size() == size());
      for (size_type i = 0; i < size(); ++i)
        _data[i] = std::data(l)[i];
    }

    //! Constructor from another dense vector if the elements are assignable to K
    template<class V>
      requires (IsFieldVectorSizeCorrect<V,SIZE>::value &&
        std::is_assignable_v<K&, decltype(std::declval<const V&>()[0])>)
    constexpr FieldVector (const DenseVector<V>& x)
    {
      assert(x.size() == size());
      for (size_type i = 0; i < size(); ++i)
        _data[i] = x[i];
    }

    //! Converting constructor from FieldVector with different element type
    template<class OtherK>
      requires (std::is_assignable_v<K&, const OtherK&>)
    explicit constexpr FieldVector (const FieldVector<OtherK, SIZE>& x)
        noexcept(std::is_nothrow_assignable_v<K&, const OtherK&>)
    {
      for (size_type i = 0; i < size(); ++i)
        _data[i] = x[i];
    }

    //! Copy constructor with default behavior
    constexpr FieldVector (const FieldVector&) = default;


    //! Assignment from another dense vector
    template<class V>
      requires (IsFieldVectorSizeCorrect<V,SIZE>::value &&
        std::is_assignable_v<K&, decltype(std::declval<const V&>()[0])>)
    constexpr FieldVector& operator= (const DenseVector<V>& x)
    {
      assert(x.size() == size());
      for (size_type i = 0; i < size(); ++i)
        _data[i] = x[i];
      return *this;
    }

    //! Assignment operator from scalar
    template<Concept::Number S>
      requires std::constructible_from<K,S>
    constexpr FieldVector& operator= (const S& scalar)
        noexcept(std::is_nothrow_constructible_v<K,S>)
    {
      _data.fill(K(scalar));
      return *this;
    }

    //! Converting assignment operator from FieldVector with different element type
    template<class OtherK>
      requires (std::is_assignable_v<K&, const OtherK&>)
    constexpr FieldVector& operator= (const FieldVector<OtherK, SIZE>& x)
        noexcept(std::is_nothrow_assignable_v<K&, const OtherK&>)
    {
      for (size_type i = 0; i < size(); ++i)
        _data[i] = x[i];
      return *this;
    }

    //! Copy assignment operator with default behavior
    constexpr FieldVector& operator= (const FieldVector&) = default;


    /// \name Capacity
    /// @{

    //! Obtain the number of elements stored in the vector
    static constexpr size_type size () noexcept { return SIZE; }

    /// @}


    /// \name Element access
    /// @{

    /**
     * \brief Return a reference to the `i`th element.
     * \throw RangeError if index `i` is out of range `[0,SIZE)` (only checked if DUNE_CHECK_BOUNDS is defined).
     */
    constexpr reference operator[] (size_type i)
    {
      DUNE_ASSERT_BOUNDS(i < size());
      return _data[i];
    }

    /**
     * \brief Return a (const) reference to the `i`th element.
     * \throw RangeError if index `i` is out of range `[0,SIZE)` (only checked if DUNE_CHECK_BOUNDS is defined).
     */
    constexpr const_reference operator[] (size_type i) const
    {
      DUNE_ASSERT_BOUNDS(i < size());
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

    //! Conversion operator
    constexpr operator const_reference () const noexcept
        requires(SIZE == 1)
    {
      return _data[0];
    }

    //! Conversion operator
    constexpr operator reference () noexcept
        requires(SIZE == 1)
    {
      return _data[0];
    }

    /// @}


    /// \name Comparison operators
    /// @{

    //! comparing FieldVectors<1> with scalar for equality
    template<Concept::Number S>
    friend constexpr bool operator== (const FieldVector& a, const S& b) noexcept
        requires(SIZE == 1)
    {
      return a._data[0] == b;
    }

    //! comparing FieldVectors<1> with scalar for equality
    template<Concept::Number S>
    friend constexpr bool operator== (const S& a, const FieldVector& b) noexcept
        requires(SIZE == 1)
    {
      return a == b._data[0];
    }

    //! three-way comparison of FieldVectors
    template<class T>
      requires (Std::three_way_comparable_with<K,T>)
    friend constexpr auto operator<=> (const FieldVector& a, const FieldVector<T,SIZE>& b) noexcept
    {
#if __cpp_lib_three_way_comparison
      return a._data <=> b._data;
#else
      return Std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end());
#endif
    }

    //! three-way comparison of FieldVectors<1> with scalar
    template<Concept::Number S>
    friend constexpr auto operator<=> (const FieldVector& a, const S& b) noexcept
        requires(SIZE == 1)
    {
      return a._data[0] <=> b;
    }

    //! three-way comparison of FieldVectors<1> with scalar
    template<Concept::Number S>
    friend constexpr auto operator<=> (const S& a, const FieldVector& b) noexcept
        requires(SIZE == 1)
    {
      return a <=> b._data[0];
    }

    /// @}


    /// \name Vector space operations
    /// @{

    //! Vector space multiplication with scalar
    template<Concept::Number S>
    friend constexpr auto operator* (const FieldVector& a, const S& b) noexcept
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      FieldVector<ResultValueType,dimension> result;
      for (size_type i = 0; i < size(); ++i)
        result[i] = a[i] * b;
      return result;
    }

    //! Vector space multiplication with scalar
    template<Concept::Number S>
    friend constexpr auto operator* (const S& a, const FieldVector& b) noexcept
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      FieldVector<ResultValueType,dimension> result;
      for (size_type i = 0; i < size(); ++i)
        result[i] = a * b[i];
      return result;
    }

    //! Vector space division by scalar
    template<Concept::Number S>
    friend constexpr auto operator/ (const FieldVector& a, const S& b) noexcept
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      FieldVector<ResultValueType,dimension> result;
      for (size_type i = 0; i < size(); ++i)
        result[i] = a[i] / b;
      return result;
    }

    //! Binary division, when using FieldVector<K,1> like K
    template<Concept::Number S>
    friend constexpr FieldVector operator/ (const S& a, const FieldVector& b) noexcept
        requires(SIZE == 1)
    {
      return FieldVector{a / b[0]};
    }

    //! Binary addition, when using FieldVector<K,1> like K
    template<Concept::Number S>
    friend constexpr auto operator+ (const FieldVector& a, const S& b) noexcept
        requires(SIZE == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldVector<ResultValueType,dimension>{a[0] + b};
    }

    //! Binary addition, when using FieldVector<K,1> like K
    template<Concept::Number S>
    friend constexpr auto operator+ (const S& a, const FieldVector& b) noexcept
        requires(SIZE == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldVector<ResultValueType,dimension>{a + b[0]};
    }

    //! Binary subtraction, when using FieldVector<K,1> like K
    template<Concept::Number S>
    friend constexpr auto operator- (const FieldVector& a, const S& b) noexcept
        requires(SIZE == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldVector<ResultValueType,dimension>{a[0] - b};
    }

    //! Binary subtraction, when using FieldVector<K,1> like K
    template<Concept::Number S>
    friend constexpr auto operator- (const S& a, const FieldVector& b) noexcept
        requires(SIZE == 1)
    {
      using ResultValueType = typename PromotionTraits<K,S>::PromotedType;
      return FieldVector<ResultValueType,dimension>{a - b[0]};
    }

    /// @}
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
  std::istream& operator>> (std::istream& in, FieldVector<K, SIZE>& v)
  {
    FieldVector<K, SIZE> w;
    for (int i = 0; i < SIZE; ++i)
      in >> w[i];
    if (in)
      v = w;
    return in;
  }

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
