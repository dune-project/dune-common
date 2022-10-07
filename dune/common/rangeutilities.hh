// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_RANGE_UTILITIES_HH
#define DUNE_COMMON_RANGE_UTILITIES_HH

#include <dune/common/typetraits.hh>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <bitset>

/**
 * \file
 *
 * \brief Utilities for reduction like operations on ranges
 * \author Christian Engwer
 */

namespace Dune
{

  /**
   * @addtogroup RangeUtilities
   * @{
   */

  /**
     \brief compute the maximum value over a range

     overloads for scalar values, and ranges exist
  */
  template <typename T,
            typename std::enable_if<IsIterable<T>::value, int>::type = 0>
  typename T::value_type
  max_value(const T & v) {
    using std::max_element;
    return *max_element(v.begin(), v.end());
  }

  template <typename T,
            typename std::enable_if<!IsIterable<T>::value, int>::type = 0>
  const T & max_value(const T & v) { return v; }

  /**
     \brief compute the minimum value over a range

     overloads for scalar values, and ranges exist
   */
  template <typename T,
            typename std::enable_if<IsIterable<T>::value, int>::type = 0>
  typename T::value_type
  min_value(const T & v) {
    using std::min_element;
    return *min_element(v.begin(), v.end());
  }

  template <typename T,
            typename std::enable_if<!IsIterable<T>::value, int>::type = 0>
  const T & min_value(const T & v) { return v; }

  /**
     \brief similar to std::bitset<N>::any() return true, if any entries is true

     overloads for scalar values, ranges, and std::bitset<N> exist
   */
  template <typename T,
            typename std::enable_if<IsIterable<T>::value, int>::type = 0>
  bool any_true(const T & v) {
    bool b = false;
    for (const auto & e : v)
      b = b or bool(e);
    return b;
  }

  template <typename T,
            typename std::enable_if<!IsIterable<T>::value, int>::type = 0>
  bool any_true(const T & v) { return v; }

  template<std::size_t N>
  bool any_true(const std::bitset<N> & b)
  {
    return b.any();
  }

  /**
     \brief similar to std::bitset<N>::all() return true, if any entries is true

     overloads for scalar values, ranges, and std::bitset<N> exist
   */
  template <typename T,
            typename std::enable_if<IsIterable<T>::value, int>::type = 0>
  bool all_true(const T & v) {
    bool b = true;
    for (const auto & e : v)
      b = b and bool(e);
    return b;
  }

  template <typename T,
            typename std::enable_if<!IsIterable<T>::value, int>::type = 0>
  bool all_true(const T & v) { return v; }

  template<std::size_t N>
  bool all_true(const std::bitset<N> & b)
  {
    return b.all();
  }



  namespace Impl
  {

    template <class T>
    class IntegralRangeIterator
    {
    public:
      typedef std::random_access_iterator_tag iterator_category;
      typedef T value_type;
      typedef std::make_signed_t<T> difference_type;
      typedef const T *pointer;
      typedef T reference;

      constexpr IntegralRangeIterator() noexcept : value_(0) {}
      constexpr explicit IntegralRangeIterator(value_type value) noexcept : value_(value) {}

      pointer operator->() const noexcept { return &value_; }
      constexpr reference operator*() const noexcept { return value_; }

      constexpr reference operator[]( difference_type n ) const noexcept { return (value_ + n); }

      constexpr bool operator==(const IntegralRangeIterator & other) const noexcept { return (value_ == other.value_); }
      constexpr bool operator!=(const IntegralRangeIterator & other) const noexcept { return (value_ != other.value_); }

      constexpr bool operator<(const IntegralRangeIterator & other) const noexcept { return (value_ <= other.value_); }
      constexpr bool operator<=(const IntegralRangeIterator & other) const noexcept { return (value_ <= other.value_); }
      constexpr bool operator>(const IntegralRangeIterator & other) const noexcept { return (value_ >= other.value_); }
      constexpr bool operator>=(const IntegralRangeIterator & other) const noexcept { return (value_ >= other.value_); }

      IntegralRangeIterator& operator++() noexcept { ++value_; return *this; }
      IntegralRangeIterator operator++(int) noexcept { IntegralRangeIterator copy( *this ); ++(*this); return copy; }

      IntegralRangeIterator& operator--() noexcept { --value_; return *this; }
      IntegralRangeIterator operator--(int) noexcept { IntegralRangeIterator copy( *this ); --(*this); return copy; }

      IntegralRangeIterator& operator+=(difference_type n) noexcept { value_ += n; return *this; }
      IntegralRangeIterator& operator-=(difference_type n) noexcept { value_ -= n; return *this; }

      friend constexpr IntegralRangeIterator operator+(const IntegralRangeIterator &a, difference_type n) noexcept { return IntegralRangeIterator(a.value_ + n); }
      friend constexpr IntegralRangeIterator operator+(difference_type n, const IntegralRangeIterator &a) noexcept { return IntegralRangeIterator(a.value_ + n); }
      friend constexpr IntegralRangeIterator operator-(const IntegralRangeIterator &a, difference_type n) noexcept { return IntegralRangeIterator(a.value_ - n); }

      constexpr difference_type operator-(const IntegralRangeIterator &other) const noexcept { return (static_cast<difference_type>(value_) - static_cast<difference_type>(other.value_)); }

    private:
      value_type value_;
    };

  } // namespace Impl



  /**
   * \brief dynamic integer range for use in range-based for loops
   *
   * \note This range can also be used in Hybrid::forEach, resulting in a dynamic
   *       for loop over the contained integers.
   *
   * \tparam  T  type of integers contained in the range
   **/
  template <class T>
  class IntegralRange
  {
  public:
    /** \brief type of integers contained in the range **/
    typedef T value_type;
    /** \brief type of iterator **/
    typedef Impl::IntegralRangeIterator<T> iterator;
    /** \brief unsigned integer type corresponding to value_type **/
    typedef std::make_unsigned_t<T> size_type;

    /** \brief construct integer range [from, to) **/
    constexpr IntegralRange(value_type from, value_type to) noexcept : from_(from), to_(to) {}
    /** \brief construct integer range [0, to) **/
    constexpr explicit IntegralRange(value_type to) noexcept : from_(0), to_(to) {}
    /** \brief construct integer range std::pair **/
    constexpr IntegralRange(std::pair<value_type, value_type> range) noexcept : from_(range.first), to_(range.second) {}

    /** \brief obtain a random-access iterator to the first element **/
    constexpr iterator begin() const noexcept { return iterator(from_); }
    /** \brief obtain a random-access iterator past the last element **/
    constexpr iterator end() const noexcept { return iterator(to_); }

    /** \brief access specified element **/
    constexpr value_type operator[](const value_type &i) const noexcept { return (from_ + i); }

    /** \brief check whether the range is empty **/
    constexpr bool empty() const noexcept { return (from_ == to_); }
    /** \brief obtain number of elements in the range **/
    constexpr size_type size() const noexcept { return (static_cast<size_type>(to_) - static_cast<size_type>(from_)); }

  private:
    value_type from_, to_;
  };


  /**
   * \brief static integer range for use in range-based for loops
   *
   * This is a compile-time static variant of the IntegralRange. Apart from
   * returning all range information statically, it casts into the corresponding
   * std::integer_sequence.
   *
   * \note This range can also be used in Hybrid::forEach, resulting in a static
   *       for loop over the contained integers like a std::integer_sequence.
   *
   * \tparam  T     type of integers contained in the range
   * \tparam  to    first element not contained in the range
   * \tparam  from  first element contained in the range, defaults to 0
   **/
  template <class T, T to, T from = 0>
  class StaticIntegralRange
  {
    template <T ofs, T... i>
    static std::integer_sequence<T, (i+ofs)...> shift_integer_sequence(std::integer_sequence<T, i...>);

  public:
    /** \brief type of integers contained in the range **/
    typedef T value_type;
    /** \brief type of iterator **/
    typedef Impl::IntegralRangeIterator<T> iterator;
    /** \brief unsigned integer type corresponding to value_type **/
    typedef std::make_unsigned_t<T> size_type;

    /** \brief type of corresponding std::integer_sequence **/
    typedef decltype(shift_integer_sequence<from>(std::make_integer_sequence<T, to-from>())) integer_sequence;

    /** \brief default constructor **/
    constexpr StaticIntegralRange() noexcept = default;

    /** \brief cast into dynamic IntegralRange **/
    constexpr operator IntegralRange<T>() const noexcept { return {from, to}; }
    /** \brief cast into corresponding std::integer_sequence **/
    constexpr operator integer_sequence() const noexcept { return {}; }

    /** \brief obtain a random-access iterator to the first element **/
    static constexpr iterator begin() noexcept { return iterator(from); }
    /** \brief obtain a random-access iterator past the last element **/
    static constexpr iterator end() noexcept { return iterator(to); }

    /** \brief access specified element (static version) **/
    template <class U, U i>
    constexpr auto operator[](const std::integral_constant<U, i> &) const noexcept
      -> std::integral_constant<value_type, from + static_cast<value_type>(i)>
    {
      return {};
    }

    /** \brief access specified element (dynamic version) **/
    constexpr value_type operator[](const size_type &i) const noexcept { return (from + static_cast<value_type>(i)); }

    /** \brief check whether the range is empty **/
    static constexpr std::integral_constant<bool, from == to> empty() noexcept { return {}; }
    /** \brief obtain number of elements in the range **/
    static constexpr std::integral_constant<size_type, static_cast<size_type>(to) - static_cast<size_type>(from) > size() noexcept { return {}; }
  };

  /**
     \brief free standing function for setting up a range based for loop
     over an integer range
     for (auto i: range(0,10)) // 0,1,2,3,4,5,6,7,8,9
     or
     for (auto i: range(-10,10)) // -10,-9,..,8,9
     or
     for (auto i: range(10)) // 0,1,2,3,4,5,6,7,8,9
   */
  template<class T, class U,
           std::enable_if_t<std::is_same<std::decay_t<T>, std::decay_t<U>>::value, int> = 0,
           std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  inline static IntegralRange<std::decay_t<T>> range(T &&from, U &&to) noexcept
  {
    return IntegralRange<std::decay_t<T>>(std::forward<T>(from), std::forward<U>(to));
  }

  template<class T, std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  inline static IntegralRange<std::decay_t<T>> range(T &&to) noexcept
  {
    return IntegralRange<std::decay_t<T>>(std::forward<T>(to));
  }

  template<class T, std::enable_if_t<std::is_enum<std::decay_t<T>>::value, int> = 0>
  inline static IntegralRange<std::underlying_type_t<std::decay_t<T>>> range(T &&to) noexcept
  {
    return IntegralRange<std::underlying_type_t<std::decay_t<T>>>(std::forward<T>(to));
  }

  template<class T, T from, T to>
  inline static StaticIntegralRange<T, to, from> range(std::integral_constant<T, from>, std::integral_constant<T, to>) noexcept
  {
    return {};
  }

  template<class T, T to>
  inline static StaticIntegralRange<T, to> range(std::integral_constant<T, to>) noexcept
  {
    return {};
  }



  /**
   * \brief Tag to enable value based transformations in TransformedRangeView
   */
  struct ValueTransformationTag {};

  /**
   * \brief Tag to enable iterator based transformations in TransformedRangeView
   */
  struct IteratorTransformationTag {};

  namespace Impl
  {

    // Helper class to mimic a pointer for proxy objects.
    // This is needed to implement operator-> on an iterator
    // using proxy-values. It stores the proxy value but
    // provides operator-> like a pointer.
    template<class ProxyType>
    class PointerProxy
    {
    public:
      PointerProxy(ProxyType&& p) : p_(p)
      {}

      ProxyType* operator->()
      {
        return &p_;
      }

      ProxyType p_;
    };

    // An iterator transforming a wrapped iterator using
    // an unary function. It inherits the iterator-category
    // of the underlying iterator.
    template <class I, class F, class TransformationType, class C = typename std::iterator_traits<I>::iterator_category>
    class TransformedRangeIterator;

    template <class I, class F, class TransformationType>
    class TransformedRangeIterator<I,F,TransformationType,std::forward_iterator_tag>
    {
    protected:

      static decltype(auto) transform(const F& f, const I& it) {
        if constexpr (std::is_same_v<TransformationType,IteratorTransformationTag>)
          return f(it);
        else
          return f(*it);
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using reference = decltype(transform(std::declval<F>(), std::declval<I>()));
      using value_type = std::decay_t<reference>;
      using pointer = PointerProxy<value_type>;

      // If we later want to allow standalone TransformedRangeIterators,
      // we could customize the FunctionPointer to be a default-constructible,
      // copy-assignable type storing a function but acting like a pointer
      // to function.
      using FunctionPointer = const F*;

      constexpr TransformedRangeIterator(const I& it, FunctionPointer f) noexcept :
        it_(it),
        f_(f)
      {}

      // Explicitly initialize members. Using a plain
      //
      //   constexpr TransformedRangeIterator() noexcept {}
      //
      // would default-initialize the members while
      //
      //   constexpr TransformedRangeIterator() noexcept : it_(), f_() {}
      //
      // leads to value-initialization. This is a case where
      // both are really different. If it_ is a raw pointer (i.e. POD)
      // then default-initialization leaves it uninitialized while
      // value-initialization zero-initializes it.
      constexpr TransformedRangeIterator() noexcept :
        it_(),
        f_()
      {}

      // Dereferencing returns a value created by the function
      constexpr reference operator*() const noexcept {
        return transform(*f_, it_);
      }

      // Dereferencing returns a value created by the function
      pointer operator->() const noexcept {
        return transform(*f_, it_);
      }

      constexpr TransformedRangeIterator& operator=(const TransformedRangeIterator& other) = default;

      constexpr bool operator==(const TransformedRangeIterator& other) const noexcept {
        return (it_ == other.it_);
      }

      constexpr bool operator!=(const TransformedRangeIterator& other) const noexcept {
        return (it_ != other.it_);
      }

      TransformedRangeIterator& operator++() noexcept {
        ++it_;
        return *this;
      }

      TransformedRangeIterator operator++(int) noexcept {
        TransformedRangeIterator copy(*this);
        ++(*this);
        return copy;
      }

    protected:
      I it_;
      FunctionPointer f_;
    };



    template <class I, class F, class T>
    class TransformedRangeIterator<I,F,T,std::bidirectional_iterator_tag> :
      public TransformedRangeIterator<I,F,T,std::forward_iterator_tag>
    {
    protected:
      using Base = TransformedRangeIterator<I,F,T,std::forward_iterator_tag>;
      using Base::it_;
      using Base::f_;
    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using reference = typename Base::reference;
      using value_type = typename Base::value_type;
      using pointer = typename Base::pointer;

      using FunctionPointer = typename Base::FunctionPointer;

      using Base::Base;

      // Member functions of the forward_iterator that need
      // to be redefined because the base class methods return a
      // forward_iterator.
      constexpr TransformedRangeIterator& operator=(const TransformedRangeIterator& other) = default;

      TransformedRangeIterator& operator++() noexcept {
        ++it_;
        return *this;
      }

      TransformedRangeIterator operator++(int) noexcept {
        TransformedRangeIterator copy(*this);
        ++(*this);
        return copy;
      }

      // Additional member functions of bidirectional_iterator
      TransformedRangeIterator& operator--() noexcept {
        --(this->it_);
        return *this;
      }

      TransformedRangeIterator operator--(int) noexcept {
        TransformedRangeIterator copy(*this);
        --(*this);
        return copy;
      }
    };



    template <class I, class F, class T>
    class TransformedRangeIterator<I,F,T,std::random_access_iterator_tag> :
      public TransformedRangeIterator<I,F,T,std::bidirectional_iterator_tag>
    {
    protected:
      using Base = TransformedRangeIterator<I,F,T,std::bidirectional_iterator_tag>;
      using Base::it_;
      using Base::f_;
    public:
      using iterator_category = std::random_access_iterator_tag;
      using reference = typename Base::reference;
      using value_type = typename Base::value_type;
      using pointer = typename Base::pointer;
      using difference_type = typename std::iterator_traits<I>::difference_type;

      using FunctionPointer = typename Base::FunctionPointer;

      using Base::Base;

      // Member functions of the forward_iterator that need
      // to be redefined because the base class methods return a
      // forward_iterator.
      constexpr TransformedRangeIterator& operator=(const TransformedRangeIterator& other) = default;

      TransformedRangeIterator& operator++() noexcept {
        ++it_;
        return *this;
      }

      TransformedRangeIterator operator++(int) noexcept {
        TransformedRangeIterator copy(*this);
        ++(*this);
        return copy;
      }

      // Member functions of the bidirectional_iterator that need
      // to be redefined because the base class methods return a
      // bidirectional_iterator.
      TransformedRangeIterator& operator--() noexcept {
        --(this->it_);
        return *this;
      }

      TransformedRangeIterator operator--(int) noexcept {
        TransformedRangeIterator copy(*this);
        --(*this);
        return copy;
      }

      // Additional member functions of random_access_iterator
      TransformedRangeIterator& operator+=(difference_type n) noexcept {
        it_ += n;
        return *this;
      }

      TransformedRangeIterator& operator-=(difference_type n) noexcept {
        it_ -= n;
        return *this;
      }

      bool operator<(const TransformedRangeIterator& other) noexcept {
        return it_<other.it_;
      }

      bool operator<=(const TransformedRangeIterator& other) noexcept {
        return it_<=other.it_;
      }

      bool operator>(const TransformedRangeIterator& other) noexcept {
        return it_>other.it_;
      }

      bool operator>=(const TransformedRangeIterator& other) noexcept {
        return it_>=other.it_;
      }

      reference operator[](difference_type n) noexcept {
        return Base::transform(*f_, it_+n);
      }

      friend
      TransformedRangeIterator operator+(const TransformedRangeIterator& it, difference_type n) noexcept {
        return TransformedRangeIterator(it.it_+n, it.f_);
      }

      friend
      TransformedRangeIterator operator+(difference_type n, const TransformedRangeIterator& it) noexcept {
        return TransformedRangeIterator(n+it.it_, it.f_);
      }

      friend
      TransformedRangeIterator operator-(const TransformedRangeIterator& it, difference_type n) noexcept {
        return TransformedRangeIterator(it.it_-n, it.f_);
      }

      friend
      difference_type operator-(const TransformedRangeIterator& first, const TransformedRangeIterator& second) noexcept {
        return first.it_-second.it_;
      }
    };


  } // namespace Impl



  /**
   * \brief A range transforming the values of another range on-the-fly
   *
   * This behaves like a range providing `begin()` and `end()`.
   * The iterators over this range internally iterate over
   * the wrapped range. When dereferencing the iterator,
   * the value is transformed on-the-fly using a given
   * transformation function leaving the underlying range
   * unchanged.
   *
   * The transformation may either return temporary values
   * or l-value references. In the former case the range behaves
   * like a proxy-container. In the latter case it forwards these
   * references allowing, e.g., to sort a subset of some container
   * by applying a transformation to an index-range for those values.
   *
   * The iterators of the TransformedRangeView have the same
   * iterator_category as the ones of the wrapped container.
   *
   * If range is given as r-value, then the returned TransformedRangeView
   * stores it by value, if range is given as (const) l-value, then the
   * TransformedRangeView stores it by (const) reference.
   *
   * If R is a value type, then the TransformedRangeView stores the wrapped range by value,
   * if R is a reference type, then the TransformedRangeView stores the wrapped range by reference.
   *
   * \tparam R Underlying range.
   * \tparam F Unary function used to transform the values in the underlying range.
   * \tparam T Class for describing how to apply the transformation
   *
   * T has to be either ValueTransformationTag (default) or IteratorTransformationTag.
   * In the former case, the transformation is applied to the values
   * obtained by dereferencing the wrapped iterator. In the latter case
   * it is applied to the iterator directly, allowing to access non-standard
   * functions of the iterator.
   **/
  template <class R, class F, class T=ValueTransformationTag>
  class TransformedRangeView
  {
    using  RawConstIterator = std::decay_t<decltype(std::declval<const R>().begin())>;
    using  RawIterator = std::decay_t<decltype(std::declval<R>().begin())>;

  public:

    /**
     * \brief Const iterator type
     *
     * This inherits the iterator_category of the iterators
     * of the underlying range.
     */
    using const_iterator = Impl::TransformedRangeIterator<RawConstIterator, F, T>;

    /**
     * \brief Iterator type
     *
     * This inherits the iterator_category of the iterators
     * of the underlying range.
     */
    using iterator = Impl::TransformedRangeIterator<RawIterator, F, T>;

    /**
     * \brief Export type of the wrapped untransformed range.
     *
     * Notice that this will always be the raw type with references
     * removed, even if a reference is stored.
     */
    using RawRange = std::remove_reference_t<R>;

    /**
     * \brief Construct from range and function
     */
    template<class RR>
    constexpr TransformedRangeView(RR&& rawRange, const F& f) noexcept :
      rawRange_(std::forward<RR>(rawRange)),
      f_(f)
    {
      static_assert(std::is_same_v<T, ValueTransformationTag> or std::is_same_v<T, IteratorTransformationTag>,
          "The TransformationType passed to TransformedRangeView has to be either ValueTransformationTag or IteratorTransformationTag.");
    }

    /**
     * \brief Obtain a iterator to the first element
     *
     * The life time of the returned iterator is bound to
     * the life time of the range since it only contains a
     * pointer to the transformation function stored
     * in the range.
     */
    constexpr const_iterator begin() const noexcept {
      return const_iterator(rawRange_.begin(), &f_);
    }

    constexpr iterator begin() noexcept {
      return iterator(rawRange_.begin(), &f_);
    }

    /**
     * \brief Obtain a iterator past the last element
     *
     * The life time of the returned iterator is bound to
     * the life time of the range since it only contains a
     * pointer to the transformation function stored
     * in the range.
     */
    constexpr const_iterator end() const noexcept {
      return const_iterator(rawRange_.end(), &f_);
    }

    constexpr iterator end() noexcept {
      return iterator(rawRange_.end(), &f_);
    }

    /**
     * \brief Obtain the size of the range
     *
     * This is only available if the underlying range
     * provides a size() method. In this case size()
     * just forwards to the underlying range's size() method.
     *
     * Attention: Don't select the template parameters explicitly.
     * They are only used to implement SFINAE.
     */
    template<class Dummy=R,
      class = std::void_t<decltype(std::declval<Dummy>().size())>>
    auto size() const
    {
      return rawRange_.size();
    }

    /**
     * \brief Export the wrapped untransformed range.
     */
    const RawRange& rawRange() const
    {
      return rawRange_;
    }

    /**
     * \brief Export the wrapped untransformed range.
     */
    RawRange& rawRange()
    {
      return rawRange_;
    }

  private:
    R rawRange_;
    F f_;
  };

  /**
   * \brief Create a TransformedRangeView
   *
   * \param range The range to transform
   * \param f Unary function that should the applied to the entries of the range.
   *
   * This behaves like a range providing `begin()` and `end()`.
   * The iterators over this range internally iterate over
   * the wrapped range. When dereferencing the iterator,
   * the wrapped iterator is dereferenced,
   * the given transformation function is applied on-the-fly,
   * and the result is returned.
   * I.e, if \code it \endcode is the wrapped iterator
   * and \code f \endcode is the transformation function,
   * then the result of \code f(*it) \endcode is returned
   *
   * The transformation may either return temporary values
   * or l-value references. In the former case the range behaves
   * like a proxy-container. In the latter case it forwards these
   * references allowing, e.g., to sort a subset of some container
   * by applying a transformation to an index-range for those values.
   *
   * The iterators of the TransformedRangeView have the same
   * iterator_category as the ones of the wrapped container.
   *
   * If range is an r-value, then the TransformedRangeView stores it by value,
   * if range is an l-value, then the TransformedRangeView stores it by reference.
   **/
  template <class R, class F>
  auto transformedRangeView(R&& range, const F& f)
  {
    return TransformedRangeView<R, F, ValueTransformationTag>(std::forward<R>(range), f);
  }

  /**
   * \brief Create a TransformedRangeView using an iterator transformation
   *
   * \param range The range to transform
   * \param f Unary function that should the applied to the entries of the range.
   *
   * This behaves like a range providing `begin()` and `end()`.
   * The iterators over this range internally iterate over
   * the wrapped range. When dereferencing the iterator,
   * the given transformation function is applied to the wrapped
   * iterator on-the-fly and the result is returned.
   * I.e, if \code it \endcode is the wrapped iterator
   * and \code f \endcode is the transformation function,
   * then the result of \code f(it) \endcode is returned.
   *
   * The transformation may either return temporary values
   * or l-value references. In the former case the range behaves
   * like a proxy-container. In the latter case it forwards these
   * references allowing, e.g., to sort a subset of some container
   * by applying a transformation to an index-range for those values.
   *
   * The iterators of the TransformedRangeView have the same
   * iterator_category as the ones of the wrapped container.
   *
   * If range is an r-value, then the TransformedRangeView stores it by value,
   * if range is an l-value, then the TransformedRangeView stores it by reference.
   **/
  template <class R, class F>
  auto iteratorTransformedRangeView(R&& range, const F& f)
  {
    return TransformedRangeView<R, F, IteratorTransformationTag>(std::forward<R>(range), f);
  }


  /**
   * \brief Allow structured-binding for-loops for sparse iterators
   *
   * Given a sparse range `R` whose iterators `it`
   * provide (additionally to dereferencing) a method
   * `it->index()` for accessing the index of the current entry in the
   * sparse range, this allows to write code like
   * \code
   * for(auto&& [A_i, i] : sparseRange(R))
   *   doSomethingWithValueAndIndex(A_i, i);
   * \endcode
   */
  template<class Range>
  auto sparseRange(Range&& range) {
    return Dune::iteratorTransformedRangeView(std::forward<Range>(range), [](auto&& it) {
        return std::tuple<decltype(*it), decltype(it.index())>(*it, it.index());
    });
  }

  /**
   * @}
   */

}

#endif // DUNE_COMMON_RANGE_UTILITIES_HH
