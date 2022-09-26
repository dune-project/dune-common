// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_INDICES_HH
#define DUNE_COMMON_INDICES_HH

#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/keywords.hh>

namespace Dune
{
  /** \addtogroup Common
   *  \{
   */

  /** \brief An index constant with value i
   *
   * An index constant is a simple type alias for an integral_constant.
   * Its main advantages are clarity (it is easier to see that code uses it
   * as an index) and the fact that the integral type is fixed, reducing verbosity
   * and avoiding the problem of maybe trying to overload / specialize using a different
   * integral type.
   */
  template<std::size_t i>
  using index_constant = std::integral_constant<std::size_t, i>;



  /** \brief Namespace with predefined compile time indices for the range [0,19]
   *
   * The predefined index objects in this namespace are `constexpr`, which allows them to
   * be used in situations where a compile time constant is needed, e.g. for a template
   * parameter. Apart from that, `constexpr` implies internal linkage, which helps to avoid
   * ODR problems.
   *
   * The constants implicitly convert to their contained value, so you can for example write
   *
   * \code{.cc}
   * std::array<int,_10> a;
   * // the above line is equivalent to
   * std::array<int,10> b;
   * \endcode
   *
   */
  namespace Indices
  {
    //! Compile time index with value 0.
    DUNE_INLINE_VARIABLE constexpr index_constant< 0>  _0 = {};

    //! Compile time index with value 1.
    DUNE_INLINE_VARIABLE constexpr index_constant< 1>  _1 = {};

    //! Compile time index with value 2.
    DUNE_INLINE_VARIABLE constexpr index_constant< 2>  _2 = {};

    //! Compile time index with value 3.
    DUNE_INLINE_VARIABLE constexpr index_constant< 3>  _3 = {};

    //! Compile time index with value 4.
    DUNE_INLINE_VARIABLE constexpr index_constant< 4>  _4 = {};

    //! Compile time index with value 5.
    DUNE_INLINE_VARIABLE constexpr index_constant< 5>  _5 = {};

    //! Compile time index with value 6.
    DUNE_INLINE_VARIABLE constexpr index_constant< 6>  _6 = {};

    //! Compile time index with value 7.
    DUNE_INLINE_VARIABLE constexpr index_constant< 7>  _7 = {};

    //! Compile time index with value 8.
    DUNE_INLINE_VARIABLE constexpr index_constant< 8>  _8 = {};

    //! Compile time index with value 9.
    DUNE_INLINE_VARIABLE constexpr index_constant< 9>  _9 = {};

    //! Compile time index with value 10.
    DUNE_INLINE_VARIABLE constexpr index_constant<10> _10 = {};

    //! Compile time index with value 11.
    DUNE_INLINE_VARIABLE constexpr index_constant<11> _11 = {};

    //! Compile time index with value 12.
    DUNE_INLINE_VARIABLE constexpr index_constant<12> _12 = {};

    //! Compile time index with value 13.
    DUNE_INLINE_VARIABLE constexpr index_constant<13> _13 = {};

    //! Compile time index with value 14.
    DUNE_INLINE_VARIABLE constexpr index_constant<14> _14 = {};

    //! Compile time index with value 15.
    DUNE_INLINE_VARIABLE constexpr index_constant<15> _15 = {};

    //! Compile time index with value 16.
    DUNE_INLINE_VARIABLE constexpr index_constant<16> _16 = {};

    //! Compile time index with value 17.
    DUNE_INLINE_VARIABLE constexpr index_constant<17> _17 = {};

    //! Compile time index with value 18.
    DUNE_INLINE_VARIABLE constexpr index_constant<18> _18 = {};

    //! Compile time index with value 19.
    DUNE_INLINE_VARIABLE constexpr index_constant<19> _19 = {};

  } // namespace Indices

  /**
   * \brief Unpack an std::integer_sequence<I,i...> to std::integral_constant<I,i>...
   *
   * This forward all entries of the given std::integer_sequence
   * as individual std::integral_constant arguments to the given callback.
   *
   * \param f Callback which has to accept unpacked values
   * \param sequence Packed std::integer_sequence of values
   * \returns Result of calling f with unpacked integers.
   */
  template<class F, class I, I... i>
  decltype(auto) constexpr unpackIntegerSequence(F&& f, std::integer_sequence<I, i...> sequence)
  {
    return f(std::integral_constant<I, i>()...);
  }

} //namespace Dune

#endif // DUNE_COMMON_INDICES_HH
