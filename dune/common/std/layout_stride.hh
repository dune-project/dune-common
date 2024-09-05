// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_LAYOUT_STRIDE_HH
#define DUNE_COMMON_STD_LAYOUT_STRIDE_HH

#include <array>
#include <type_traits>

#include <dune/common/indices.hh>
#include <dune/common/std/no_unique_address.hh>
#include <dune/common/std/impl/fwd_layouts.hh>

namespace Dune::Std {

/// \brief A layout mapping where the strides are user-defined.
template <class Extents>
class layout_stride::mapping
{
  template <class> friend class mapping;
  static constexpr typename Extents::rank_type rank_ = Extents::rank();

public:
  using extents_type = Extents;
  using index_type = typename extents_type::index_type;
  using size_type = typename extents_type::size_type;
  using rank_type = typename extents_type::rank_type;
  using layout_type = layout_stride;

private:
  using strides_type = std::array<index_type,rank_>;

public:

  /// \brief The default construction initializes the strides from layout_right
  constexpr mapping () noexcept
    : mapping(layout_right::template mapping<extents_type>{})
  {}

  /// \brief Copy constructor for the mapping
  constexpr mapping (const mapping&) noexcept = default;

  /// \brief Construct the mapping from given extents and strides
  template <class OtherIndexType,
    std::enable_if_t<std::is_convertible_v<const OtherIndexType&, index_type>, int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type, const OtherIndexType&>, int> = 0>
  constexpr mapping (const extents_type& e, const std::array<OtherIndexType,rank_>& s) noexcept
    : extents_(e)
    , strides_{}
  {
    for (rank_type r = 0; r < rank_; ++r)
      strides_[r] = s[r];
  }

  /// \brief Construct the mapping from given extents and strides
  template <class OtherIndexType,
    std::enable_if_t<std::is_convertible_v<const OtherIndexType&, index_type>, int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type, const OtherIndexType&>, int> = 0>
  constexpr mapping (const extents_type& e, const span<OtherIndexType,rank_>& s) noexcept
    : extents_(e)
    , strides_{}
  {
    for (rank_type r = 0; r < rank_; ++r)
      strides_[r] = s[r];
  }

  /// \brief Construct the mapping from another mapping with different extents and different strides
  template <class M,
    std::enable_if_t<(M::extents_type::rank() == extents_type::rank()), int> = 0,
    std::enable_if_t<(M::is_always_unique()), int> = 0,
    std::enable_if_t<(M::is_always_strided()), int> = 0,
    decltype(std::declval<M>().extents(), bool{}) = true,
    decltype(std::declval<M>().stride(std::declval<rank_type>()), bool{}) = true>
  constexpr mapping (const M& m) noexcept
    : extents_(m.extents())
    , strides_{}
  {
    for (rank_type r = 0; r < rank_; ++r)
      strides_[r] = m.stride(r);
  }

  /// \brief Copy-assignment for the mapping
  constexpr mapping& operator= (const mapping&) noexcept = default;

  constexpr const extents_type& extents () const noexcept { return extents_; }

  /// \brief Return the sum `1 + (E(0)-1)*S(0) + (E(1)-1)*S(1) + ...`
  constexpr index_type required_span_size () const noexcept
  {
    return size(extents_,strides_);
  }

  /// \brief Compute the offset by folding with index-array with the strides array
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == rank_), int> = 0,
    std::enable_if_t<(std::is_convertible_v<Indices, index_type> && ...), int> = 0,
    std::enable_if_t<(std::is_nothrow_constructible_v<index_type, Indices> && ...), int> = 0>
  constexpr index_type operator() (Indices... ii) const noexcept
  {
    return unpackIntegerSequence([&](auto... r) {
      return ((static_cast<index_type>(ii)*strides_[r]) + ... + 0); },
      std::make_index_sequence<rank_>{});
  }

  /// \brief The default offset for rank-0 tensors is 0
  constexpr index_type operator() () const noexcept
  {
    return 0;
  }

  static constexpr bool is_always_unique () noexcept { return true; }
  static constexpr bool is_always_exhaustive () noexcept { return false; }
  static constexpr bool is_always_strided () noexcept { return true; }

  static constexpr bool is_unique () noexcept { return true; }
  static constexpr bool is_strided () noexcept { return true; }

  constexpr bool is_exhaustive () const noexcept
  {
    // Actually this could be improved. A strided layout can still be exhaustive.
    // This test is more complicated to implement, though. See §24.7.3.4.7.4 line (5.2)
    // in the C++ standard document N4971
    return extents_type::rank() == 0 || (required_span_size() > 0 && required_span_size() == extents().product());
  }

  /// \brief Get the array of all strides
  constexpr const strides_type& strides () const noexcept
  {
    return strides_;
  }

  /// \brief Get the single stride `i`
  template <class E = extents_type,
    std::enable_if_t<(E::rank() > 0), int> = 0>
  constexpr index_type stride (rank_type i) const noexcept
  {
    return strides_[i];
  }

  template <class OtherMapping,
    std::enable_if_t<(OtherMapping::extents_type::rank() == extents_type::rank()), int> = 0,
    std::enable_if_t<(OtherMapping::is_always_strided()), int> = 0>
  friend constexpr bool operator== (const mapping& a, const OtherMapping& b) noexcept
  {
    if (offset(b))
      return false;
    if constexpr(extents_type::rank() == 0)
      return true;
    return a.extents_ == b.extents_ && a.strides_ == b.strides_;
  }

private:
  template <class E, class S>
  static constexpr index_type size (const E& extents, const S& strides) noexcept
  {
    if constexpr (E::rank() == 0)
      return 1;
    else {
      if (extents.product() == 0)
        return 0;
      else {
        index_type result = 1;
        for (rank_type r = 0; r < E::rank(); ++r)
          result += (extents.extent(r)-1) * strides[r];
        return result;
      }
    }
  }

  template <class M>
  static constexpr size_type offset (const M& m) noexcept
  {
    if constexpr (M::extents_type::rank() == 0)
      return m();
    else {
      if (m.required_span_size() == 0)
        return 0;
      else {
        return unpackIntegerSequence([&](auto... r) {
          return m((r,0)...); }, // map the index tuple (0,0...)
          std::make_index_sequence<M::extents_type::rank()>{});
      }
    }
  }

private:
  DUNE_NO_UNIQUE_ADDRESS extents_type extents_;
  strides_type strides_;
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_LAYOUT_STRIDE_HH
