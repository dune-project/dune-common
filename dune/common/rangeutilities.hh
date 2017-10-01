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
    : public std::iterator<std::random_access_iterator_tag, T, std::make_signed_t<T>, const T *, T>
  {
    typedef std::iterator<std::random_access_iterator_tag, T, std::make_signed_t<T>, const T *, T> Base;

  public:
    using typename Base::value_type;
    using typename Base::pointer;
    using typename Base::reference;
    using typename Base::difference_type;

    IntegralRangeIterator() noexcept = default;
    explicit IntegralRangeIterator(value_type value) noexcept : value_(value) {}

    pointer operator->() const noexcept { return &value_; }
    reference operator*() const noexcept { return value_; }

    reference operator[]( difference_type n ) const noexcept { return (value_ + n); }

    bool operator==(const IntegralRangeIterator & other) const noexcept { return (value_ == other.value_); }
    bool operator!=(const IntegralRangeIterator & other) const noexcept { return (value_ != other.value_); }

    bool operator<(const IntegralRangeIterator & other) const noexcept { return (value_ <= other.value_); }
    bool operator<=(const IntegralRangeIterator & other) const noexcept { return (value_ <= other.value_); }
    bool operator>(const IntegralRangeIterator & other) const noexcept { return (value_ >= other.value_); }
    bool operator>=(const IntegralRangeIterator & other) const noexcept { return (value_ >= other.value_); }

    IntegralRangeIterator& operator++() noexcept { ++value_; return *this; }
    IntegralRangeIterator operator++(int) noexcept { IntegralRangeIterator copy( *this ); ++(*this); return copy; }

    IntegralRangeIterator& operator--() noexcept { --value_; return *this; }
    IntegralRangeIterator operator--(int) noexcept { IntegralRangeIterator copy( *this ); --(*this); return copy; }

    IntegralRangeIterator& operator+=(difference_type n) noexcept { value_ += n; return *this; }
    IntegralRangeIterator& operator-=(difference_type n) noexcept { value_ -= n; return *this; }

    friend IntegralRangeIterator operator+(const IntegralRangeIterator &a, difference_type n) noexcept { return IntegralRangeIterator(a.value_ + n); }
    friend IntegralRangeIterator operator+(difference_type n, const IntegralRangeIterator &a) noexcept { return IntegralRangeIterator(a.value_ + n); }
    friend IntegralRangeIterator operator-(const IntegralRangeIterator &a, difference_type n) noexcept { return IntegralRangeIterator(a.value_ - n); }

    difference_type operator-(const IntegralRangeIterator &other) const noexcept { return (static_cast<difference_type>(value_) - static_cast<difference_type>(other.value_)); }

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

    IntegralRange(value_type from, value_type to) noexcept : from_(from), to_(to) {}
    IntegralRange(std::pair<value_type, value_type> range) noexcept : from_(range.first), to_(range.second) {}
    IntegralRange(value_type to) noexcept : from_(0), to_(to) {}

    iterator begin() const noexcept { return iterator(from_); }
    iterator end() const noexcept { return iterator(to_); }

    bool empty() const noexcept { return (from_ == to_); }
    size_type size() const noexcept { return (static_cast<size_type>(to_) - static_cast<size_type>(from_)); }

  private:
    value_type from_, to_;
  };

  /**
     \brief free standing function for setting up a range based for loop
     over an integer range
     for (auto i: integralRange(0,10))
     or
     for (auto i: integralRange<int>(-10,10))
   */
  template<class T, std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  inline static IntegralRange<std::decay_t<T>> range(T &&from, T &&to) noexcept
  {
    return {std::forward<T>(from), std::forward<T>(to)};
  }

  template<class T, std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  inline static IntegralRange<std::decay_t<T>> range(T &&to) noexcept
  {
    return {std::forward<T>(to)};
  }

}

#endif // DUNE_COMMON_RANGE_UTILITIES_HH
