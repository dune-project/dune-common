// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_COMPARE_HH
#define DUNE_COMMON_STD_COMPARE_HH

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

/**
 * \file This file provides some concepts introduced in the c++ standard library
 * <compare> and <concepts> not yet available in all library implementation.
 * The code is partially extracted from https://en.cppreference.com/w/cpp/utility/compare.
 */

namespace Dune::Std {
namespace Impl {

template <class T, class Cat>
concept comparesAs =
  std::same_as<std::common_comparison_category_t<T, Cat>, Cat>;

template <class T, class U>
concept weaklyEqualityComparableWith =
  requires(const std::remove_reference_t<T>& t,
           const std::remove_reference_t<U>& u)
  {
    { t == u } -> std::convertible_to<bool>;
    { t != u } -> std::convertible_to<bool>;
    { u == t } -> std::convertible_to<bool>;
    { u != t } -> std::convertible_to<bool>;
  };

template <class T, class U>
concept partiallyOrderedWith =
  requires(const std::remove_reference_t<T>& t,
           const std::remove_reference_t<U>& u)
  {
    { t <  u } -> std::convertible_to<bool>;
    { t >  u } -> std::convertible_to<bool>;
    { t <= u } -> std::convertible_to<bool>;
    { t >= u } -> std::convertible_to<bool>;
    { u <  t } -> std::convertible_to<bool>;
    { u >  t } -> std::convertible_to<bool>;
    { u <= t } -> std::convertible_to<bool>;
    { u >= t } -> std::convertible_to<bool>;
  };

template <class T, class U, class C = std::common_reference_t<const T&, const U&>>
concept comparisonCommonTypeWithImpl =
  std::same_as<std::common_reference_t<const T&, const U&>,
               std::common_reference_t<const U&, const T&>> &&
  requires
  {
    requires std::convertible_to<const T&, const C&> ||
      std::convertible_to<T, const C&>;
    requires std::convertible_to<const U&, const C&> ||
      std::convertible_to<U, const C&>;
  };

template <class T, class U>
concept comparisonCommonTypeWith =
  comparisonCommonTypeWithImpl<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

} // end namespace Impl

/**
 * \brief The concept `std::three_way_comparable` specifies that the three way
 * comparison `operator <=>` on `T` yield results consistent with the comparison
 * category implied by `Cat`.
 *
 * The standard implementation is available in libstdc++ >= 10 and libc++ >= 14.
 */
template <class T, class Cat = std::partial_ordering>
concept three_way_comparable =
  Impl::weaklyEqualityComparableWith<T, T> &&
  Impl::partiallyOrderedWith<T, T> &&
  requires(const std::remove_reference_t<T>& a,
           const std::remove_reference_t<T>& b)
  {
    { a <=> b } -> Impl::comparesAs<Cat>;
  };


/**
 * \brief The concept `std::three_way_comparable_with` specifies that the three
 * way comparison `operator <=>` on (possibly mixed) `T` and `U` operands yield
 * results consistent with the comparison category implied by `Cat`. Comparing
 * mixed operands yields results equivalent to comparing the operands converted
 * to their common type.
 *
 * The standard implementation is available in libstdc++ >= 10 and libc++ >= 14.
 */
template <class T, class U, class Cat = std::partial_ordering>
concept three_way_comparable_with =
  Std::three_way_comparable<T, Cat> &&
  Std::three_way_comparable<U, Cat> &&
  Impl::comparisonCommonTypeWith<T, U> &&
  Std::three_way_comparable<
    std::common_reference_t<
      const std::remove_reference_t<T>&,
      const std::remove_reference_t<U>&>, Cat> &&
  Impl::weaklyEqualityComparableWith<T, U> &&
  Impl::partiallyOrderedWith<T, U> &&
  requires(const std::remove_reference_t<T>& t,
           const std::remove_reference_t<U>& u)
  {
    { t <=> u } -> Impl::comparesAs<Cat>;
    { u <=> t } -> Impl::comparesAs<Cat>;
  };

//! A functor implementing the three-way comparison on the arguments
struct compare_three_way
{
  template <class T, class U>
  constexpr auto operator() (T&& t, U&& u) const
  {
    return std::forward<T>(t) <=> std::forward<U>(u);
  }
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_COMPARE_HH
