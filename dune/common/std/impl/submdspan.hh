// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_IMPL_SUBMDSPAN_HH
#define DUNE_COMMON_STD_IMPL_SUBMDSPAN_HH

#include <array>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/std/extents.hh>
#include <dune/common/std/layout_left_padded.hh>
#include <dune/common/std/layout_left.hh>
#include <dune/common/std/layout_right_padded.hh>
#include <dune/common/std/layout_right.hh>
#include <dune/common/std/layout_stride.hh>
#include <dune/common/std/submdspan_slices.hh>

namespace Dune::Std::Impl {

// Treat std::integral_constant slice values as their encoded value and all
// other slice values as runtime values.
template <class T>
inline constexpr bool is_integral_constant_v = Dune::IsIntegralConstant<T>::value;

template <class T>
constexpr auto slice_index (T value)
{
  if constexpr(is_integral_constant_v<T>)
    return T::value;
  else
    return value;
}

// Classification of the slice specifiers supported by the Dune backport.
// "Kept" slices contribute one dimension to the result; integral slices fix a
// coordinate and remove that dimension.
template <class T>
struct IsFullExtent : std::is_same<std::remove_cv_t<T>,full_extent_t> {};

template <class T>
inline constexpr bool is_full_extent_v = IsFullExtent<T>::value;

template <class T>
struct IsExtentSlice : std::false_type {};

template <class O, class E, class S>
struct IsExtentSlice<extent_slice<O,E,S>> : std::true_type {};

template <class T>
inline constexpr bool is_extent_slice_v = IsExtentSlice<std::remove_cv_t<T>>::value;

template <class T>
struct IsRangeSlice : std::false_type {};

template <class F, class L, class S>
struct IsRangeSlice<range_slice<F,L,S>> : std::true_type {};

template <class T>
inline constexpr bool is_range_slice_v = IsRangeSlice<std::remove_cv_t<T>>::value;

template <class T, class = void>
struct IsPairLike : std::false_type {};

template <class T>
struct IsPairLike<T, std::void_t<
  decltype(std::tuple_size<std::remove_cv_t<T>>::value),
  typename std::tuple_element<0,std::remove_cv_t<T>>::type,
  typename std::tuple_element<1,std::remove_cv_t<T>>::type>>
  : std::bool_constant<(std::tuple_size<std::remove_cv_t<T>>::value == 2)> {};

template <class T>
inline constexpr bool is_pair_like_v = IsPairLike<T>::value;

template <class Slice>
inline constexpr bool is_kept_slice_v =
  is_full_extent_v<Slice> || is_extent_slice_v<Slice> || is_range_slice_v<Slice> || is_pair_like_v<Slice>;

// Slices that keep a dimension and do not change the stride of that dimension.
// These are the slice forms that can preserve a padded layout for submatrix views.
template <class Slice, class = void>
struct IsContiguousKeptSlice : IsFullExtent<Slice> {};

template <class T>
struct IsContiguousKeptSlice<T, std::enable_if_t<is_pair_like_v<T>>> : std::true_type {};

template <class Stride, bool = is_integral_constant_v<Stride>>
struct IsUnitStride : std::false_type {};

template <class Stride>
struct IsUnitStride<Stride,true> : std::bool_constant<(Stride::value == 1)> {};

template <class First, class Last, class Stride>
struct IsContiguousKeptSlice<range_slice<First,Last,Stride>>
  : IsUnitStride<Stride>
{};

template <class Offset, class Extent, class Stride>
struct IsContiguousKeptSlice<extent_slice<Offset,Extent,Stride>>
  : IsUnitStride<Stride>
{};

template <class Slice>
inline constexpr bool is_contiguous_kept_slice_v =
  IsContiguousKeptSlice<std::remove_cv_t<std::remove_reference_t<Slice>>>::value;

// Compute the source index selected by a slice at relative result index zero.
template <class Slice, class IndexType>
constexpr IndexType slice_offset (const Slice& slice)
{
  if constexpr(is_full_extent_v<Slice>)
    return 0;
  else if constexpr(is_extent_slice_v<Slice>)
    return IndexType(slice_index(slice.offset));
  else if constexpr(is_range_slice_v<Slice>)
    return IndexType(slice_index(slice.first));
  else if constexpr(is_pair_like_v<Slice>)
    return IndexType(std::get<0>(slice));
  else
    return IndexType(slice_index(slice));
}

// Compute the source-index stride represented by one increment in a kept slice.
template <class Slice, class IndexType>
constexpr IndexType slice_stride (const Slice& slice)
{
  if constexpr(is_extent_slice_v<Slice>)
    return IndexType(slice_index(slice.stride));
  else if constexpr(is_range_slice_v<Slice>)
    return IndexType(slice_index(slice.stride));
  else
    return 1;
}

// Compute the runtime extent of a kept slice.
template <std::size_t k, class Extents, class Slice>
constexpr typename Extents::index_type slice_extent (const Extents& extents, const Slice& slice)
{
  using index_type = typename Extents::index_type;
  if constexpr(is_full_extent_v<Slice>)
    return extents.extent(k);
  else if constexpr(is_extent_slice_v<Slice>)
    return index_type(slice_index(slice.extent));
  else if constexpr(is_range_slice_v<Slice>)
    return (index_type(slice_index(slice.last)) - index_type(slice_index(slice.first))
      + index_type(slice_index(slice.stride)) - 1) / index_type(slice_index(slice.stride));
  else
    return index_type(std::get<1>(slice)) - index_type(std::get<0>(slice));
}

// Compute the static extent of a kept slice. Dynamic slice bounds or extents
// produce std::dynamic_extent.
template <std::size_t k, class Extents, class Slice>
struct StaticSliceExtent
  : std::integral_constant<std::size_t,std::dynamic_extent>
{};

template <std::size_t k, class Extents>
struct StaticSliceExtent<k,Extents,full_extent_t>
  : std::integral_constant<std::size_t,Extents::static_extent(k)>
{};

template <class T>
struct StaticValueOrDynamic
  : std::integral_constant<std::size_t,std::dynamic_extent>
{};

template <class T, T value>
struct StaticValueOrDynamic<std::integral_constant<T,value>>
  : std::integral_constant<std::size_t,std::size_t(value)>
{};

template <class First, class Last, class Stride, bool = is_integral_constant_v<First> && is_integral_constant_v<Last> && is_integral_constant_v<Stride>>
struct StaticRangeExtent
  : std::integral_constant<std::size_t,std::dynamic_extent>
{};

template <class First, class Last, class Stride>
struct StaticRangeExtent<First,Last,Stride,true>
  : std::integral_constant<std::size_t,
      std::size_t((Last::value - First::value + Stride::value - 1) / Stride::value)>
{};

template <std::size_t k, class Extents, class Offset, class SliceExtent, class Stride>
struct StaticSliceExtent<k,Extents,extent_slice<Offset,SliceExtent,Stride>>
  : StaticValueOrDynamic<SliceExtent>
{};

template <std::size_t k, class Extents, class First, class Last, class Stride>
struct StaticSliceExtent<k,Extents,range_slice<First,Last,Stride>>
  : StaticRangeExtent<First,Last,Stride>
{};

// Build the result extents type by walking source dimensions and appending only
// the dimensions represented by kept slices.
template <class IndexType, class ExtentsSeq, class SrcExtents, std::size_t k, class... Slices>
struct SubextentsImpl;

template <class IndexType, std::size_t... exts, class SrcExtents, std::size_t k>
struct SubextentsImpl<IndexType, std::integer_sequence<std::size_t,exts...>, SrcExtents, k>
{
  using type = Std::extents<IndexType,exts...>;
};

template <class IndexType, std::size_t... exts, class SrcExtents, std::size_t k, class Slice0, class... Slices>
struct SubextentsImpl<IndexType, std::integer_sequence<std::size_t,exts...>, SrcExtents, k, Slice0, Slices...>
{
  using next_seq = std::conditional_t<
    is_kept_slice_v<Slice0>,
    std::integer_sequence<std::size_t,exts...,StaticSliceExtent<k,SrcExtents,Slice0>::value>,
    std::integer_sequence<std::size_t,exts...>>;

  using type = typename SubextentsImpl<IndexType,next_seq,SrcExtents,k+1,Slices...>::type;
};

template <class Extents, class... Slices>
using SubextentsType = typename SubextentsImpl<
  typename Extents::index_type,
  std::integer_sequence<std::size_t>,
  Extents,
  0,
  std::remove_cv_t<std::remove_reference_t<Slices>>...>::type;

template <class Layout>
struct LayoutLeftPadding
  : std::integral_constant<std::size_t,std::dynamic_extent>
{};

template <std::size_t PaddingValue>
struct LayoutLeftPadding<layout_left_padded<PaddingValue>>
  : std::integral_constant<std::size_t,PaddingValue>
{};

template <class Layout>
struct LayoutRightPadding
  : std::integral_constant<std::size_t,std::dynamic_extent>
{};

template <std::size_t PaddingValue>
struct LayoutRightPadding<layout_right_padded<PaddingValue>>
  : std::integral_constant<std::size_t,PaddingValue>
{};

template <class Layout>
inline constexpr bool is_padded_layout_v =
  is_layout_left_padded_v<Layout> || is_layout_right_padded_v<Layout>;

// Conservative layout preservation: keep layout_right only for fixed leading
// dimensions followed by full dimensions, and layout_left for full dimensions
// followed by fixed trailing dimensions.
template <class Layout, class SlicesTuple, std::size_t... i>
constexpr bool preserve_layout_right (std::index_sequence<i...>)
{
  if constexpr(!std::is_same_v<Layout,layout_right>)
    return false;
  else {
    constexpr bool kept[] = {is_kept_slice_v<std::tuple_element_t<i,SlicesTuple>>...};
    constexpr bool full[] = {is_full_extent_v<std::tuple_element_t<i,SlicesTuple>>...};
    bool seenKept = false;
    for (std::size_t k = 0; k < sizeof...(i); ++k) {
      if (kept[k]) {
        seenKept = true;
        if (!full[k])
          return false;
      } else if (seenKept)
        return false;
    }
    return true;
  }
}

template <class Layout, class SlicesTuple, std::size_t... i>
constexpr bool preserve_layout_left (std::index_sequence<i...>)
{
  if constexpr(!std::is_same_v<Layout,layout_left>)
    return false;
  else {
    constexpr bool kept[] = {is_kept_slice_v<std::tuple_element_t<i,SlicesTuple>>...};
    constexpr bool full[] = {is_full_extent_v<std::tuple_element_t<i,SlicesTuple>>...};
    bool seenFixed = false;
    for (std::size_t k = 0; k < sizeof...(i); ++k) {
      if (kept[k]) {
        if (seenFixed || !full[k])
          return false;
      } else
        seenFixed = true;
    }
    return true;
  }
}

// Padded-left submatrix preservation from P2642. For rank two, contiguous
// ranges in both dimensions are representable. For higher ranks, changing the
// second dimension would also change the stride of later dimensions, so only
// the first dimension may be a proper range while the others remain full.
template <class Layout, class SlicesTuple, std::size_t... i>
constexpr bool preserve_layout_left_padded (std::index_sequence<i...>)
{
  constexpr std::size_t rank = sizeof...(i);
  if constexpr(!(std::is_same_v<Layout,layout_left> || is_layout_left_padded_v<Layout>) || rank < 2)
    return false;
  else {
    constexpr bool contiguous[] = {is_contiguous_kept_slice_v<std::tuple_element_t<i,SlicesTuple>>...};
    constexpr bool full[] = {is_full_extent_v<std::tuple_element_t<i,SlicesTuple>>...};
    if (!contiguous[0] || !contiguous[1])
      return false;
    if constexpr(rank > 2 && !full[1])
      return false;
    for (std::size_t k = 2; k < rank; ++k)
      if (!full[k])
        return false;
    return true;
  }
}

// Padded-right counterpart. For rank two, contiguous ranges in both dimensions
// are representable. For higher ranks, the next-to-last dimension must remain
// full so that strides of earlier dimensions stay unchanged.
template <class Layout, class SlicesTuple, std::size_t... i>
constexpr bool preserve_layout_right_padded (std::index_sequence<i...>)
{
  constexpr std::size_t rank = sizeof...(i);
  if constexpr(!(std::is_same_v<Layout,layout_right> || is_layout_right_padded_v<Layout>) || rank < 2)
    return false;
  else {
    constexpr bool contiguous[] = {is_contiguous_kept_slice_v<std::tuple_element_t<i,SlicesTuple>>...};
    constexpr bool full[] = {is_full_extent_v<std::tuple_element_t<i,SlicesTuple>>...};
    for (std::size_t k = 0; k + 2 < rank; ++k)
      if (!full[k])
        return false;
    if constexpr(rank > 2 && !full[rank-2])
      return false;
    return contiguous[rank-2] && contiguous[rank-1];
  }
}

template <class Layout, class Extents>
struct SubLayoutLeftPadded
{
  static constexpr std::size_t padding = [] {
    if constexpr(is_layout_left_padded_v<Layout>)
      return LayoutLeftPadding<Layout>::value;
    else if constexpr(Extents::rank() > 0 && Extents::static_extent(0) != std::dynamic_extent)
      return Extents::static_extent(0);
    else
      return std::dynamic_extent;
  }();

  using type = layout_left_padded<padding>;
};

template <class Layout, class Extents>
struct SubLayoutRightPadded
{
  static constexpr std::size_t padding = [] {
    if constexpr(is_layout_right_padded_v<Layout>)
      return LayoutRightPadding<Layout>::value;
    else if constexpr(Extents::rank() > 0 && Extents::static_extent(Extents::rank()-1) != std::dynamic_extent)
      return Extents::static_extent(Extents::rank()-1);
    else
      return std::dynamic_extent;
  }();

  using type = layout_right_padded<padding>;
};

template <class Layout, class Extents, class... Slices>
struct SubLayout
{
  using slices_tuple = std::tuple<std::remove_cv_t<std::remove_reference_t<Slices>>...>;
  static constexpr bool right = preserve_layout_right<Layout,slices_tuple>(std::index_sequence_for<Slices...>{});
  static constexpr bool left = preserve_layout_left<Layout,slices_tuple>(std::index_sequence_for<Slices...>{});
  static constexpr bool right_padded = preserve_layout_right_padded<Layout,slices_tuple>(std::index_sequence_for<Slices...>{});
  static constexpr bool left_padded = preserve_layout_left_padded<Layout,slices_tuple>(std::index_sequence_for<Slices...>{});

  using type = std::conditional_t<right, layout_right,
    std::conditional_t<left, layout_left,
    std::conditional_t<right_padded, typename SubLayoutRightPadded<Layout,Extents>::type,
    std::conditional_t<left_padded, typename SubLayoutLeftPadded<Layout,Extents>::type,
    layout_stride>>>>;
};

// Construct the result mapping plus the source data offset. This is the central
// implementation behind both submdspan_mapping and submdspan.
template <class Mapping, class... Slices, std::size_t... k>
constexpr auto make_sub_mapping (const Mapping& mapping, const std::tuple<Slices...>& slices, std::index_sequence<k...>)
{
  using src_extents_type = typename Mapping::extents_type;
  using index_type = typename Mapping::index_type;
  using sub_extents_type = SubextentsType<src_extents_type,Slices...>;
  using sub_layout_type = typename SubLayout<typename Mapping::layout_type,src_extents_type,Slices...>::type;
  using sub_mapping_type = typename sub_layout_type::template mapping<sub_extents_type>;

  constexpr std::size_t rank = src_extents_type::rank();
  constexpr std::size_t subrank = sub_extents_type::rank();

  const auto& src_extents = mapping.extents();
  std::array<index_type,rank> offsets{slice_offset<std::tuple_element_t<k,std::tuple<Slices...>>,index_type>(std::get<k>(slices))...};
  std::array<index_type,subrank> sub_exts{};
  std::array<index_type,subrank> sub_strides{};

  std::size_t j = 0;
  auto add_kept = [&](auto kk) {
    constexpr std::size_t i = decltype(kk)::value;
    using slice_type = std::tuple_element_t<i,std::tuple<Slices...>>;
    if constexpr(is_kept_slice_v<slice_type>) {
      const auto& slice = std::get<i>(slices);
      sub_exts[j] = slice_extent<i>(src_extents, slice);
      sub_strides[j] = mapping.stride(i) * slice_stride<slice_type,index_type>(slice);
      ++j;
    }
  };
  (add_kept(std::integral_constant<std::size_t,k>{}), ...);

  const index_type offset = unpackIntegerSequence([&](auto... i) {
    return mapping(offsets[i]...);
  }, std::make_index_sequence<rank>{});

  sub_extents_type sub_extents{sub_exts};

  if constexpr(std::is_same_v<sub_layout_type,layout_stride>) {
    return submdspan_mapping_result<sub_mapping_type>{
      sub_mapping_type{sub_extents, sub_strides}, offset};
  } else if constexpr(is_padded_layout_v<sub_layout_type>) {
    using strided_mapping_type = typename layout_stride::template mapping<sub_extents_type>;
    return submdspan_mapping_result<sub_mapping_type>{
      sub_mapping_type{strided_mapping_type{sub_extents, sub_strides}}, offset};
  } else {
    return submdspan_mapping_result<sub_mapping_type>{
      sub_mapping_type{sub_extents}, offset};
  }
}

} // end namespace Dune::Std::Impl

#endif // DUNE_COMMON_STD_IMPL_SUBMDSPAN_HH
