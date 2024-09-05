// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_LAYOUT_RIGHT_HH
#define DUNE_COMMON_STD_LAYOUT_RIGHT_HH

#include <array>
#include <type_traits>

#include <dune/common/indices.hh>
#include <dune/common/std/no_unique_address.hh>
#include <dune/common/std/impl/fwd_layouts.hh>

namespace Dune::Std {

/// \brief A layout mapping where the rightmost extent has stride 1.
template <class Extents>
class layout_right::mapping
{
  template <class> friend class mapping;

public:
  using extents_type = Extents;
  using size_type = typename extents_type::size_type;
  using rank_type = typename extents_type::rank_type;
  using index_type = typename extents_type::index_type;
  using layout_type = layout_right;

  /// \brief The default construction is possible for default constructible extents
  constexpr mapping () noexcept = default;

  /// \brief Copy constructor for the mapping
  constexpr mapping (const mapping&) noexcept = default;

  /// \brief Construct the mapping with from given extents
  constexpr mapping (const extents_type& e) noexcept
    : extents_(e)
  {}

  /// \brief Construct the mapping from another mapping with a different extents type
  template <class OtherExtents,
    std::enable_if_t<std::is_constructible_v<extents_type, OtherExtents>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(!std::is_convertible_v<OtherExtents, extents_type>)
  #endif
  constexpr mapping (const mapping<OtherExtents>& m) noexcept
    : extents_(m.extents())
  {}

  /// \brief Construct the mapping from a layout_left
  template <class OtherExtents, class E = extents_type,
    std::enable_if_t<(E::rank() <= 1), int> = 0,
    std::enable_if_t<std::is_constructible_v<extents_type, OtherExtents>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(!std::is_convertible_v<OtherExtents, extents_type>)
  #endif
  constexpr mapping (const layout_left::mapping<OtherExtents>& m) noexcept
    : extents_(m.extents())
  {}

  /// \brief Construct the mapping from a layout_stride
  template <class OtherExtents,
    std::enable_if_t<std::is_constructible_v<extents_type, OtherExtents>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(extents_type::rank() > 0)
  #endif
  constexpr mapping (const layout_stride::mapping<OtherExtents>& m)
    : extents_(m.extents())
  {
#ifndef NDEBUG
    if constexpr(extents_type::rank() > 0) {
      index_type prod = 1;
      for (rank_type r = extents_type::rank()-1; r > 0; --r) {
        assert(m.strides(r) == prod);
        prod *= m.extents().extent(r);
      }
      assert(m.strides(0) == prod);
    }
#endif
  }

  /// \brief Copy-assignment for the mapping
  constexpr mapping& operator= (const mapping&) noexcept = default;

  constexpr const extents_type& extents () const noexcept { return extents_; }
  constexpr index_type required_span_size () const noexcept { return extents_.product();  }

  /// \brief Compute the offset i3 + E(3)*(i2 + E(2)*(i1 + E(1)*i0))
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
    std::enable_if_t<(std::is_convertible_v<Indices, index_type> && ...), int> = 0,
    std::enable_if_t<(std::is_nothrow_constructible_v<Indices, index_type> && ...), int> = 0>
  constexpr index_type operator() (Indices... ii) const noexcept
  {
    const std::array indices{index_type(std::move(ii))...};
    index_type value = indices.front();
    for (rank_type j = 0; j < extents_type::rank()-1; ++j) {
      value = indices[j+1] + extents_.extent(j+1) * value;
    }
    return value;
  }

  /// \brief The default offset for rank-0 tensors is 0
  constexpr index_type operator() () const noexcept
  {
    return 0;
  }

  static constexpr bool is_always_unique () noexcept { return true; }
  static constexpr bool is_always_exhaustive () noexcept { return true; }
  static constexpr bool is_always_strided () noexcept { return true; }

  static constexpr bool is_unique () noexcept { return true; }
  static constexpr bool is_exhaustive () noexcept { return true; }
  static constexpr bool is_strided () noexcept { return true; }

  /// \brief The stride is the product of the extents `E(n)*E(n-1)*...*E(i+1)`
  template <class E = extents_type,
    std::enable_if_t<(E::rank() > 0), int> = 0>
  constexpr index_type stride (rank_type i) const noexcept
  {
    assert(i < extents_type::rank());
    index_type prod = 1;
    for (rank_type r = i+1; r < extents_type::rank(); ++r)
      prod *= extents().extent(r);
    return prod;
  }

  template <class OtherExtents>
  friend constexpr bool operator== (const mapping& a, const mapping<OtherExtents>& b) noexcept
  {
    return a.extents_ == b.extents_;
  }

private:
  DUNE_NO_UNIQUE_ADDRESS extents_type extents_;
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_LAYOUT_RIGHT_HH
