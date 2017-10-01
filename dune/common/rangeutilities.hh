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
    IntegralRangeIterator(T value)
    : value_(value) {}
    bool operator!=(IntegralRangeIterator const& other) const
    {
      return value_ != other.value_;
    }
    T const& operator*() const
    {
      return value_;
    }
    IntegralRangeIterator& operator++()
    {
      ++value_;
      return *this;
    }
  private:
    T value_;
  };
  template <class T>
  class IntegralRange
  {
  public:
    IntegralRange(T from, T to)
    : from_(from), to_(to) {}
    IntegralRange(std::pair<T,T> range)
    : from_(range.first), to_(range.second) {}
    IntegralRange(T to)
    : from_(0), to_(to){}
    IntegralRangeIterator<T> begin() const
    {
      return IntegralRangeIterator<T>(from_);
    }
    IntegralRangeIterator<T> end() const
    {
      return IntegralRangeIterator<T>(to_);
    }
  private:
    T const from_;
    T const to_;
  };

  /**
     \brief free standing function for setting up a range based for loop
     over an integer range
     for (auto i: integralRange(0,10))
     or
     for (auto i: integralRange<int>(-10,10))
   */
  template<typename T=unsigned>
  IntegralRange<T> integralRange(T from, T to)
  {
    static_assert(std::is_integral<T>::value, "range only accepts integral values");
    return { from, to };
  }

}

#endif // DUNE_COMMON_RANGE_UTILITIES_HH
