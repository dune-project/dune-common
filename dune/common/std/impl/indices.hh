// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_IMPL_INDICES_HH
#define DUNE_COMMON_STD_IMPL_INDICES_HH

#include <array>
#include <span>
#include <type_traits>

#include <dune/common/indices.hh>

namespace Dune::Std::Impl {

template <class Tensor, class Index>
[[nodiscard]] constexpr bool indexInDimension (const Tensor& tensor,
                                               typename Tensor::rank_type r,
                                               const Index& index) noexcept
{
  using index_type = typename Tensor::index_type;
  if constexpr(std::is_signed_v<std::remove_cv_t<Index>>)
    if (index < 0)
      return false;
  return index_type(index) < tensor.extent(r);
}

/// \brief Check whether a tuple of indices is in the index space `[0,extent_0)x...x[0,extent_{r-1})`.
template <class Tensor, class... Indices>
[[nodiscard]] constexpr bool indexInIndexSpace (const Tensor& tensor, const Indices&... indices) noexcept
{
  return unpackIntegerSequence([&](auto... ii) {
    return (indexInDimension(tensor, ii, indices) && ...);
  }, std::make_index_sequence<sizeof...(Indices)>{});
}

/// \brief Check whether an array of indices is in the index space `[0,extent_0)x...x[0,extent_{r-1})`.
template <class Tensor, class Index>
[[nodiscard]] constexpr bool indexInIndexSpace (
    const Tensor& tensor,
    const std::array<Index,Tensor::extents_type::rank()>& indices) noexcept
{
  return unpackIntegerSequence([&](auto... ii) {
    return (indexInDimension(tensor, ii, indices[ii]) && ...);
  }, std::make_index_sequence<Tensor::extents_type::rank()>{});
}

/// \brief Check whether a span of indices is in the index space `[0,extent_0)x...x[0,extent_{r-1})`.
template <class Tensor, class Index>
[[nodiscard]] constexpr bool indexInIndexSpace (
    const Tensor& tensor,
    std::span<Index,Tensor::extents_type::rank()> indices) noexcept
{
  return unpackIntegerSequence([&](auto... ii) {
    return (indexInDimension(tensor, ii, indices[ii]) && ...);
  }, std::make_index_sequence<Tensor::extents_type::rank()>{});
}

} // end namespace Dune::Std::Impl

#endif // DUNE_COMMON_STD_IMPL_INDICES_HH
