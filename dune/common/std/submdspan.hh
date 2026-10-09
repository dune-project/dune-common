// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_SUBMDSPAN_HH
#define DUNE_COMMON_STD_SUBMDSPAN_HH

#include <array>
#include <tuple>
#include <type_traits>

#include <dune/common/indices.hh>
#include <dune/common/std/extents.hh>
#include <dune/common/std/impl/submdspan.hh>
#include <dune/common/std/mdspan.hh>
#include <dune/common/std/submdspan_slices.hh>

/**
 * \file
 * \brief Slicing utilities for `Dune::Std::mdspan`.
 *
 * `submdspan` creates a non-owning view into an existing `Dune::Std::mdspan`.
 * It follows the interface of `std::submdspan` from
 * <a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2630r4.html">P2630R4</a>
 * and the C++ standard working draft
 * <a href="https://eel.is/c++draft/views.multidim#mdspan.sub">[mdspan.sub]</a>.
 *
 * A slice specifier is provided for every source dimension. Each slice either
 * fixes a dimension and removes it from the result rank, or keeps a dimension
 * with a full, contiguous, or strided index set.
 *
 * Supported slice specifiers:
 * - An integral value or `std::integral_constant` fixes one source index and
 *   removes that dimension.
 * - `Dune::Std::full_extent` keeps the full source dimension.
 * - `Dune::Std::range_slice{first,last}` keeps the half-open range
 *   `[first,last)`.
 * - `Dune::Std::range_slice{first,last,stride}` keeps the strided range
 *   `first, first+stride, ...`.
 * - `Dune::Std::extent_slice{offset,extent,stride}` keeps `extent` entries
 *   starting at `offset` with the given stride.
 * - Pair-like objects such as `std::pair{first,last}` are accepted as
 *   half-open ranges.
 *
 * The returned `mdspan` aliases the source data. The implementation preserves
 * `layout_right` for fixed leading indices followed by full dimensions, e.g.
 * row slices of a row-major tensor, and preserves `layout_left` for the
 * symmetric fixed trailing-index case. Other regular slices use
 * `layout_stride`.
 *
 * \b Examples:
 * \code{.cpp}
 * std::array<double, 12> data{};
 * Dune::Std::mdspan a(data.data(), 3, 4);
 *
 * // Fix the first index: a rank-1 row view.
 * auto row = Dune::Std::submdspan(a, 1, Dune::Std::full_extent);
 *
 * // Keep a half-open range of columns.
 * auto cols = Dune::Std::submdspan(a, Dune::Std::full_extent,
 *                                  Dune::Std::range_slice{1, 4});
 *
 * // Keep every second column.
 * auto evenCols = Dune::Std::submdspan(a, Dune::Std::full_extent,
 *                                      Dune::Std::extent_slice{0, 2, 2});
 *
 * // Fix both indices: a rank-0 mdspan.
 * auto entry = Dune::Std::submdspan(a, 1, 2);
 * \endcode
 */

namespace Dune::Std {

/**
 * \brief Return the extents produced by slicing a source extents object.
 * \ingroup CxxUtilities
 *
 * See the file documentation for the supported slice specifiers.
 */
template <class IndexType, std::size_t... exts, class... SliceSpecifiers,
  std::enable_if_t<(sizeof...(SliceSpecifiers) == sizeof...(exts)), int> = 0>
constexpr auto subextents (const extents<IndexType,exts...>& src, SliceSpecifiers... slices)
{
  using sub_extents_type = Impl::SubextentsType<extents<IndexType,exts...>,SliceSpecifiers...>;
  std::array<IndexType,sub_extents_type::rank()> sub_exts{};
  std::size_t j = 0;
  auto tuple = std::tuple<SliceSpecifiers...>{slices...};
  Dune::unpackIntegerSequence([&](auto... k) {
    auto add = [&](auto kk) {
      constexpr std::size_t i = decltype(kk)::value;
      using slice_type = std::tuple_element_t<i,decltype(tuple)>;
      if constexpr(Impl::is_kept_slice_v<slice_type>)
        sub_exts[j++] = Impl::slice_extent<i>(src, std::get<i>(tuple));
    };
    (add(std::integral_constant<std::size_t,k>{}), ...);
  }, std::index_sequence_for<SliceSpecifiers...>{});
  return sub_extents_type{sub_exts};
}

/**
 * \brief Create the mapping and source-data offset produced by slicing a layout mapping.
 * \ingroup CxxUtilities
 *
 * This is the mapping-level counterpart of `submdspan`.
 */
template <class Mapping, class... SliceSpecifiers,
  std::enable_if_t<(sizeof...(SliceSpecifiers) == Mapping::extents_type::rank()), int> = 0>
constexpr auto submdspan_mapping (const Mapping& mapping, SliceSpecifiers... slices)
{
  auto tuple = std::tuple<SliceSpecifiers...>{slices...};
  return Impl::make_sub_mapping(mapping, tuple, std::index_sequence_for<SliceSpecifiers...>{});
}

/**
 * \brief Create an `mdspan` view by slicing an existing `mdspan`.
 * \ingroup CxxUtilities
 *
 * See the file documentation for the supported slice specifiers, layout
 * preservation rules, and examples.
 */
template <class ElementType, class Extents, class LayoutPolicy, class AccessorPolicy,
          class... SliceSpecifiers,
  std::enable_if_t<(sizeof...(SliceSpecifiers) == Extents::rank()), int> = 0>
constexpr auto submdspan (const mdspan<ElementType,Extents,LayoutPolicy,AccessorPolicy>& src,
                          SliceSpecifiers... slices)
{
  auto result = submdspan_mapping(src.mapping(), slices...);
  using mapping_type = decltype(result.mapping);
  using accessor_type = typename AccessorPolicy::offset_policy;
  return mdspan<ElementType, typename mapping_type::extents_type, typename mapping_type::layout_type, accessor_type>{
    src.accessor().offset(src.data_handle(), result.offset),
    result.mapping,
    accessor_type(src.accessor())};
}

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_SUBMDSPAN_HH
