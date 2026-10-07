// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_SUBMDSPAN_SLICES_HH
#define DUNE_COMMON_STD_SUBMDSPAN_SLICES_HH

#include <cstddef>
#include <type_traits>

#include <dune/common/std/no_unique_address.hh>

namespace Dune::Std {

/**
 * \brief Slice specifier selecting the full extent in one mdspan dimension.
 * \ingroup CxxUtilities
 *
 * A `full_extent_t` slice keeps the corresponding source dimension in the
 * submdspan and copies its extent unchanged. The inline object `full_extent`
 * is the intended spelling:
 * \code{.cpp}
 * auto row = Dune::Std::submdspan(matrix, i, Dune::Std::full_extent);
 * \endcode
 */
struct full_extent_t
{
  explicit full_extent_t () = default;
};

/// \brief Slice specifier selecting the full extent in one mdspan dimension.
/// \ingroup CxxUtilities
inline constexpr full_extent_t full_extent{};

/**
 * \brief Slice specifier selecting `extent` entries starting at `offset` with a stride.
 * \ingroup CxxUtilities
 *
 * The resulting submdspan keeps this dimension. Its new index `i` maps to the
 * source index `offset + i*stride`.
 */
template <class OffsetType, class ExtentType, class StrideType>
struct extent_slice
{
  using offset_type = OffsetType;
  using extent_type = ExtentType;
  using stride_type = StrideType;

  DUNE_NO_UNIQUE_ADDRESS offset_type offset{};
  DUNE_NO_UNIQUE_ADDRESS extent_type extent{};
  DUNE_NO_UNIQUE_ADDRESS stride_type stride{};
};

template <class OffsetType, class ExtentType, class StrideType>
extent_slice(OffsetType, ExtentType, StrideType)
  -> extent_slice<OffsetType,ExtentType,StrideType>;

/**
 * \brief Slice specifier selecting the half-open range `[first,last)` with a stride.
 * \ingroup CxxUtilities
 *
 * A `range_slice` keeps this dimension. The default stride is one, so
 * `range_slice{first,last}` selects the usual contiguous half-open range.
 */
template <class FirstType, class LastType,
          class StrideType = std::integral_constant<std::size_t,1>>
struct range_slice
{
  using first_type = FirstType;
  using last_type = LastType;
  using stride_type = StrideType;

  DUNE_NO_UNIQUE_ADDRESS first_type first{};
  DUNE_NO_UNIQUE_ADDRESS last_type last{};
  DUNE_NO_UNIQUE_ADDRESS stride_type stride{};
};

template <class FirstType, class LastType>
range_slice(FirstType, LastType)
  -> range_slice<FirstType,LastType>;

template <class FirstType, class LastType, class StrideType>
range_slice(FirstType, LastType, StrideType)
  -> range_slice<FirstType,LastType,StrideType>;

/**
 * \brief Compatibility spelling for a strided extent slice.
 * \ingroup CxxUtilities
 */
template <class OffsetType, class ExtentType, class StrideType>
using strided_slice = extent_slice<OffsetType,ExtentType,StrideType>;

/**
 * \brief Result type returned by `submdspan_mapping`.
 * \ingroup CxxUtilities
 *
 * Stores the mapping of the submdspan and the offset into the source data
 * handle. `submdspan` applies this offset through the source accessor.
 */
template <class LayoutMapping>
struct submdspan_mapping_result
{
  DUNE_NO_UNIQUE_ADDRESS LayoutMapping mapping;
  typename LayoutMapping::index_type offset{};
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_SUBMDSPAN_SLICES_HH
