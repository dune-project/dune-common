// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_LAYOUT_LEFT_PADDED_HH
#define DUNE_COMMON_STD_LAYOUT_LEFT_PADDED_HH

#include <cassert>
#include <span>
#include <type_traits>

#include <dune/common/std/layout_left.hh>
#include <dune/common/std/layout_right.hh>
#include <dune/common/std/layout_stride.hh>
#include <dune/common/std/no_unique_address.hh>
#include <dune/common/std/impl/fwd_layouts.hh>

namespace Dune::Std {
namespace Impl {

template <class Layout>
struct IsLayoutLeftPadded : std::false_type {};

template <std::size_t PaddingValue>
struct IsLayoutLeftPadded<layout_left_padded<PaddingValue>> : std::true_type {};

template <class Layout>
inline constexpr bool is_layout_left_padded_v = IsLayoutLeftPadded<Layout>::value;

} // end namespace Impl

/**
 * \brief A left layout mapping with optional padding of the stride of dimension 1.
 * \ingroup CxxUtilities
 *
 * This layout follows the `layout_left_padded` mapping from
 * <a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2642r0.html">P2642R0</a>
 * and the C++ standard working draft
 * <a href="https://eel.is/c++draft/views.multidim#mdspan.layout.leftpad">[mdspan.layout.leftpad]</a>.
 * It is useful when the leftmost dimension should remain contiguous but the
 * leading stride of the remaining dimensions should be supplied explicitly, for
 * example to satisfy alignment, cache-line, SIMD, or accelerator
 * memory-coalescing requirements.
 *
 * For rank two, `stride(0) == 1` and `stride(1)` is either `extent(0)` or a
 * padded value. The padding can leave unused memory between columns while still
 * providing a regular strided mdspan mapping.
 */
template <std::size_t PaddingValue>
template <class Extents>
class layout_left_padded<PaddingValue>::mapping
{
public:
  using extents_type = Extents;
  using index_type = typename extents_type::index_type;
  using size_type = typename extents_type::size_type;
  using rank_type = typename extents_type::rank_type;
  using layout_type = layout_left_padded<PaddingValue>;

  static constexpr std::size_t padding_value = PaddingValue;

  constexpr mapping () noexcept = default;
  constexpr mapping (const mapping&) noexcept = default;

  constexpr mapping (const extents_type& e) noexcept
    : extents_(e)
    , stride1_(default_stride1(e))
  {}

  template <class OtherIndexType,
    std::enable_if_t<std::is_convertible_v<OtherIndexType,index_type>, int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type,OtherIndexType>, int> = 0>
  constexpr mapping (const extents_type& e, OtherIndexType padding) noexcept
    : extents_(e)
    , stride1_(index_type(padding))
  {
    assert(Extents::rank() <= 1 || index_type(padding) >= e.extent(0));
    if constexpr(PaddingValue != std::dynamic_extent)
      assert(index_type(PaddingValue) == index_type(padding));
  }

  template <class OtherExtents,
    std::enable_if_t<std::is_constructible_v<extents_type, OtherExtents>, int> = 0>
  constexpr mapping (const layout_left::mapping<OtherExtents>& m) noexcept
    : mapping(extents_type(m.extents()))
  {}

  template <class OtherExtents, class E = extents_type,
    std::enable_if_t<(E::rank() <= 1), int> = 0,
    std::enable_if_t<std::is_constructible_v<extents_type, OtherExtents>, int> = 0>
  constexpr mapping (const layout_right::mapping<OtherExtents>& m) noexcept
    : mapping(extents_type(m.extents()))
  {}

  template <class OtherExtents,
    std::enable_if_t<std::is_constructible_v<extents_type, OtherExtents>, int> = 0>
  constexpr mapping (const layout_stride::mapping<OtherExtents>& m) noexcept
    : extents_(m.extents())
    , stride1_(Extents::rank() > 1 ? m.stride(1) : index_type(0))
  {
#ifndef NDEBUG
    if constexpr(Extents::rank() > 0)
      assert(m.stride(0) == 1);
    for (rank_type r = 2; r < Extents::rank(); ++r)
      assert(m.stride(r) == stride1_ * fwd_product(r));
#endif
  }

  template <class OtherMapping,
    std::enable_if_t<Impl::is_layout_left_padded_v<typename OtherMapping::layout_type>, int> = 0,
    std::enable_if_t<std::is_constructible_v<extents_type, typename OtherMapping::extents_type>, int> = 0>
  constexpr mapping (const OtherMapping& m) noexcept
    : extents_(m.extents())
    , stride1_(Extents::rank() > 1 ? m.stride(1) : index_type(0))
  {}

  constexpr mapping& operator= (const mapping&) noexcept = default;

  constexpr const extents_type& extents () const noexcept { return extents_; }

  constexpr index_type required_span_size () const noexcept
  {
    if constexpr(Extents::rank() == 0)
      return 1;
    else {
      if (product() == 0)
        return 0;
      index_type result = 1;
      for (rank_type r = 0; r < Extents::rank(); ++r)
        result += (extents_.extent(r)-1) * stride(r);
      return result;
    }
  }

  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == Extents::rank()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<Indices,index_type>), int> = 0>
  constexpr index_type operator() (Indices... indices) const noexcept
  {
    const index_type ii[] = {index_type(indices)...};
    index_type result = 0;
    for (rank_type r = 0; r < Extents::rank(); ++r)
      result += ii[r] * stride(r);
    return result;
  }

  constexpr index_type operator() () const noexcept { return 0; }

  static constexpr bool is_always_unique () noexcept { return true; }
  static constexpr bool is_always_exhaustive () noexcept { return PaddingValue == std::dynamic_extent; }
  static constexpr bool is_always_strided () noexcept { return true; }

  constexpr bool is_unique () const noexcept { return true; }
  constexpr bool is_strided () const noexcept { return true; }
  constexpr bool is_exhaustive () const noexcept
  {
    return Extents::rank() <= 1 || stride1_ == extents_.extent(0);
  }

  template <class E = extents_type,
    std::enable_if_t<(E::rank() > 0), int> = 0>
  constexpr index_type stride (rank_type r) const noexcept
  {
    assert(r < Extents::rank());
    if (r == 0)
      return 1;
    if (r == 1)
      return stride1_;
    return stride1_ * fwd_product(r);
  }

  template <class OtherMapping,
    std::enable_if_t<(OtherMapping::extents_type::rank() == Extents::rank()), int> = 0,
    std::enable_if_t<(OtherMapping::is_always_strided()), int> = 0>
  friend constexpr bool operator== (const mapping& a, const OtherMapping& b) noexcept
  {
    if (a.extents_ != b.extents())
      return false;
    for (rank_type r = 0; r < Extents::rank(); ++r)
      if (a.stride(r) != b.stride(r))
        return false;
    return true;
  }

private:
  static constexpr index_type padded_stride (index_type padding, index_type extent) noexcept
  {
    assert(padding >= extent);
    return padding;
  }

  static constexpr index_type default_stride1 (const extents_type& e) noexcept
  {
    if constexpr(Extents::rank() <= 1)
      return 0;
    else if constexpr(PaddingValue == std::dynamic_extent)
      return e.extent(0);
    else
      return padded_stride(index_type(PaddingValue), e.extent(0));
  }

  constexpr index_type fwd_product (rank_type r) const noexcept
  {
    index_type prod = 1;
    for (rank_type k = 1; k < r; ++k)
      prod *= extents_.extent(k);
    return prod;
  }

  constexpr size_type product () const noexcept
  {
    size_type prod = 1;
    for (rank_type r = 0; r < Extents::rank(); ++r)
      prod *= extents_.extent(r);
    return prod;
  }

private:
  DUNE_NO_UNIQUE_ADDRESS extents_type extents_;
  index_type stride1_ = 0;
};

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_LAYOUT_LEFT_PADDED_HH
