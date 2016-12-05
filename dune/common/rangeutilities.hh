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

}

#endif // DUNE_COMMON_RANGE_UTILITIES_HH
