// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_CMATH_HH
#define DUNE_COMMON_STD_CMATH_HH

#include <cmath>

#if __has_include(<version>)
#include <version>
#if (__cpp_lib_constexpr_cmath < 202306L)
#include <limits>
#include <type_traits>
#include <utility>
#endif
#endif

namespace Dune {

namespace Std {

#if (__cpp_lib_constexpr_cmath < 202202L)
// backport for constexpr functions between C++20 and C++23
template<class T>
constexpr T
abs(T t)
{
  if (std::is_constant_evaluated()) {
    return (t < 0) ? -t : t;
  } else {
    using std::abs;
    return abs(t);
  }
}

template<class T>
constexpr auto
sqrt(T t)
  requires(std::is_floating_point_v<T> || std::is_integral_v<T>)
{
  if (std::is_constant_evaluated()) {
    using TT = std::conditional_t<std::is_floating_point_v<T>, T, double>;
    if (t >= TT{ 0 } and t < std::numeric_limits<TT>::infinity()) {
      // use Heron's method:
      // https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Heron's_method
      TT curr = t, prev = 0;
      while (curr != prev)
        prev = std::exchange(curr, TT{ 0.5 } * (curr + TT{ t } / curr));
      return curr;
    } else {
      return std::numeric_limits<TT>::quiet_NaN();
    }
  } else {
    using std::sqrt;
    return sqrt(t);
  }
};
#else
using std::abs;
using std::sqrt;
#endif

} // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_CMATH_HH
