#ifndef DUNE_COMMON_RANGE_UTILITIES_HH
#define DUNE_COMMON_RANGE_UTILITIES_HH

#include <dune/common/typetraits.hh>
#include <utility>
#include <type_traits>
#include <bitset>

namespace Dune
{
  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  const T & max_value(const T & v) { return v; };

  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  typename T::value_type
  max_value(const T & v) {
    using std::max;
    typename T::value_type m;
    for (const auto & e : v)
      m = max(e,m);
    return m;
  };

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  T & min_value(const T & v) { return v; };

  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  typename T::value_type
  min_value(const T & v) {
    using std::min;
    typename T::value_type m;
    for (const auto & e : v)
      m = min(e,m);
    return m;
  };

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  bool any_true(const T & v) { return v; };

  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  bool any_true(const T & v) {
    bool b = false;
    for (const auto & e : v)
      b = b or bool(e);
    return b;
  };

  template<std::size_t N>
  bool any_true(const std::bitset<N> & b)
  {
    return b.any();
  }

  template <typename T,
            typename std::enable_if<!is_range<T>::value, int>::type = 0>
  bool all_true(const T & v) { return v; };

  template <typename T,
            typename std::enable_if<is_range<T>::value, int>::type = 0>
  bool all_true(const T & v) {
    bool b = true;
    for (const auto & e : v)
      b = b and bool(e);
    return b;
  };

  template<std::size_t N>
  bool all_true(const std::bitset<N> & b)
  {
    return b.all();
  }

}

#endif // DUNE_COMMON_RANGE_UTILITIES_HH
