// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_RANGE_UTILITIES_HH
#define DUNE_COMMON_RANGE_UTILITIES_HH

#include <dune/common/typetraits.hh>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <bitset>

/**
 * \file
 * \brief Utilities for reduction like operations on ranges
 * \author Christian Engwer
 */

/**
 * @addtogroup RangeUtilities
 * @{
 */

namespace Dune
{
  /**
     \brief compute the maximum value over a range

     overloads for scalar values, and ranges exist
  */
  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  typename T::value_type
  max_value(const T & v) {
    using std::max_element;
    return *max_element(v.begin(), v.end());
  }

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  const T & max_value(const T & v) { return v; }

  /**
     \brief compute the minimum value over a range

     overloads for scalar values, and ranges exist
   */
  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  typename T::value_type
  min_value(const T & v) {
    using std::min_element;
    return *min_element(v.begin(), v.end());
  }

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  const T & min_value(const T & v) { return v; }

  /**
     \brief similar to std::bitset<N>::any() return true, if any entries is true

     overloads for scalar values, ranges, and std::bitset<N> exist
   */
  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  bool any_true(const T & v) {
    bool b = false;
    for (const auto & e : v)
      b = b or bool(e);
    return b;
  }

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
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
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  bool all_true(const T & v) {
    bool b = true;
    for (const auto & e : v)
      b = b and bool(e);
    return b;
  }

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  bool all_true(const T & v) { return v; }

  template<std::size_t N>
  bool all_true(const std::bitset<N> & b)
  {
    return b.all();
  }



  /**
     \brief iterator for implementing range based for loops over an integer range
   */
  template <class T>
  class IntegralRangeIterator
  {
  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::make_signed_t<T> difference_type;
    typedef const T *pointer;
    typedef T reference;

    constexpr IntegralRangeIterator() noexcept = default;
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

  template <class T>
  class IntegralRange
  {
  public:
    typedef T value_type;
    typedef IntegralRangeIterator<T> iterator;
    typedef std::make_unsigned_t<T> size_type;

    constexpr IntegralRange(value_type from, value_type to) noexcept : from_(from), to_(to) {}
    constexpr IntegralRange(value_type to) noexcept : from_(0), to_(to) {}
    constexpr IntegralRange(std::pair<value_type, value_type> range) noexcept : from_(range.first), to_(range.second) {}

    constexpr iterator begin() const noexcept { return iterator(from_); }
    constexpr iterator end() const noexcept { return iterator(to_); }

    constexpr value_type operator[](const value_type i) const noexcept { return (from_ + i); }

    constexpr bool empty() const noexcept { return (from_ == to_); }
    constexpr size_type size() const noexcept { return (static_cast<size_type>(to_) - static_cast<size_type>(from_)); }

  private:
    value_type from_, to_;
  };

  template <class T, T to, T from = 0>
  class StaticIntegralRange
  {
    template <T ofs, T... i>
    static std::integer_sequence<T, (i+ofs)...> shift_integer_sequence(std::integer_sequence<T, i...>);

  public:
    typedef T value_type;
    typedef IntegralRangeIterator<T> iterator;
    typedef std::make_unsigned_t<T> size_type;

    typedef decltype(shift_integer_sequence<from>(std::make_integer_sequence<T, to-from>())) integer_sequence;

    constexpr StaticIntegralRange() noexcept = default;

    constexpr operator IntegralRange<T>() const noexcept { return {from, to}; }
    constexpr operator integer_sequence() const noexcept { return {}; }

    static constexpr iterator begin() noexcept { return iterator(from); }
    static constexpr iterator end() noexcept { return iterator(to); }

    constexpr value_type operator[](const value_type i) const noexcept { return (from + i); }

    static constexpr bool empty() noexcept { return (from == to); }
    static constexpr size_type size() noexcept { return (static_cast<size_type>(to) - static_cast<size_type>(from)); }
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
    return {std::forward<T>(from), std::forward<U>(to)};
  }

  template<class T, std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  inline static IntegralRange<std::decay_t<T>> range(T &&to) noexcept
  {
    return {std::forward<T>(to)};
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

}

#endif // DUNE_COMMON_RANGE_UTILITIES_HH
