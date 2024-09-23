// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_EXTENTS_HH
#define DUNE_COMMON_STD_EXTENTS_HH

#include <array>
#include <cassert>
#include <limits>
#include <type_traits>
#if __has_include(<version>)
  #include <version>
#endif

#include <dune/common/indices.hh>
#include <dune/common/std/no_unique_address.hh>
#include <dune/common/std/span.hh>
#include <dune/common/std/impl/fwd_layouts.hh>

namespace Dune::Std {
namespace Impl {

template <class IndexType, std::size_t n>
struct DynamicExtentsArray
{
  using type = std::array<IndexType,n>;
};

template <class IndexType>
struct DynamicExtentsArray<IndexType,0>
{
  // empty type with minimal array-like interface
  struct type {
    IndexType operator[](std::size_t /*i*/) const { return 0; }
  };
};

} // end namespace Impl


/**
 * \brief Multidimensional index space with dynamic and static extents.
 * \ingroup CxxUtilities
 *
 * This class template represents a multidimensional index space of rank equal to `sizeof...(exts)`.
 * Each extent might be specified as a template parameter or as a dynamic parameter in the constructor.
 *
 * \tparam IndexType  An integral type other than `bool`
 * \tparam exts...    Each element of exts is either `Std::dynamic_extent` or a representable
 *                    value of type `IndexType`.
 **/
template <class IndexType, std::size_t... exts>
class extents
{
  static_assert(std::is_integral_v<IndexType>);

private:
  static constexpr std::size_t rank_ = sizeof...(exts);
  static constexpr std::size_t rank_dynamic_ = ((exts == Std::dynamic_extent) + ... + 0);

  // this type is used internally to extract the static extents by index
  using array_type = std::array<std::size_t,rank_>;

  // store at position i how many extents in {exts[0],...,exts[i]} are dynamic_extent
  static constexpr std::array<std::size_t,rank_+1> make_dynamic_index()
  {
    std::array<std::size_t,rank_+1> di{{}};
    for (std::size_t i = 0; i < rank_; ++i)
      di[i+1] = di[i] + (array_type{exts...}[i] == Std::dynamic_extent);
    return di;
  }

  // An index mapping computed by `make_dynamic_index()` to get the position of a dynamic
  // extent in {exts...} within the array dynamic_extents.
  static constexpr std::array<std::size_t,rank_+1> dynamic_index_{make_dynamic_index()};

public:
  using rank_type = std::size_t;
  using index_type = IndexType;
  using size_type = std::make_unsigned_t<index_type>;

  /// \name Observers
  /// [mdspan.extents.obs], observers of the multidimensional index space
  /// @{

  /// \brief The total number of dimensions
  static constexpr rank_type rank () noexcept { return rank_; }

  /// \brief The number of dimensions with dynamic extent
  static constexpr rank_type rank_dynamic () noexcept { return rank_dynamic_; }

  /// \brief Return the static extent of dimension `r` or `Std::dynamic_extent`
  static constexpr std::size_t static_extent (rank_type r) noexcept
  {
    assert(rank() > 0 && r < rank());
    return array_type{exts...}[r];
  }

  /// \brief Return the extent of dimension `i`
  constexpr index_type extent (rank_type r) const noexcept
  {
    assert(rank() > 0 && r < rank());
    if (std::size_t e = static_extent(r); e != Std::dynamic_extent)
      return index_type(e);
    else
      return dynamic_extents_[dynamic_index_[r]];
  }

  /// @}

public:
  /// \name extents constructors
  /// @{

  /// \brief The default constructor requires that all exts are not `Std::dynamic_extent`.
  constexpr extents () noexcept = default;

  /// \brief Set all extents by the given integral sequence
  /// [[pre: all static extents correspond to the given value e]]
  template <class... IndexTypes,
    std::enable_if_t<(... && std::is_convertible_v<IndexTypes,index_type>), int> = 0,
    std::enable_if_t<(sizeof...(IndexTypes) == rank() || sizeof...(IndexTypes) == rank_dynamic()), int> = 0,
    std::enable_if_t<(... && std::is_nothrow_constructible_v<index_type, IndexTypes>), int> = 0>
  constexpr explicit extents (IndexTypes... e) noexcept
  {
    init_dynamic_extents<sizeof...(e)>(std::array<index_type,sizeof...(e)>{index_type(e)...});
  }

  /// \brief Set all dynamic extents by the given integral array
  /// [[pre: all static extents correspond to the given values in e]]
  template <class I, std::size_t N,
    std::enable_if_t<std::is_convertible_v<I, index_type>, int> = 0,
    std::enable_if_t<(N == rank() || N == rank_dynamic()), int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(N != rank_dynamic())
  #endif
  constexpr extents (const std::array<I,N>& e) noexcept
  {
    init_dynamic_extents<N>(e);
  }

  /// \brief Set all dynamic extents by the given integral array
  /// [[pre: all static extents correspond to the given values in e]]
  template <class I, std::size_t N,
    std::enable_if_t<std::is_convertible_v<I, index_type>, int> = 0,
    std::enable_if_t<(N == rank() || N == rank_dynamic()), int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type, const I&>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(N != rank_dynamic())
  #endif
  constexpr extents (Std::span<I,N> e) noexcept
  {
    init_dynamic_extents<N>(e);
  }

  template <class I, std::size_t... e,
    std::enable_if_t<(sizeof...(e) == rank()), int> = 0,
    std::enable_if_t<((e == Std::dynamic_extent || exts == Std::dynamic_extent || e == exts) &&...), int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(
    (( (exts != Std::dynamic_extent) && (e == Std::dynamic_extent)) || ... ) ||
    (std::numeric_limits<index_type>::max() < std::numeric_limits<I>::max()))
  #endif
  constexpr extents (const extents<I,e...>& other) noexcept
  {
    init_dynamic_extents<sizeof...(e)>(as_array(other));
  }

  /// @}


  /// \brief Compare two extents by their rank and all individual extents
  template <class OtherIndexType, std::size_t... otherExts>
  friend constexpr bool operator== (const extents& a, const extents<OtherIndexType, otherExts...>& b) noexcept
  {
    if (a.rank() != b.rank())
      return false;
    using I = std::common_type_t<index_type, OtherIndexType>;
    for (rank_type i = 0; i < rank(); ++i)
      if (I(a.extent(i)) != I(b.extent(i)))
        return false;
    return true;
  }

private:
#ifndef DOXYGEN
  // The product of all extents
  constexpr size_type product () const noexcept
  {
    size_type prod = 1;
    for (rank_type i = 0; i < rank(); ++i)
      prod *= extent(i);
    return prod;
  }

  // A representation of all extents as an array
  template <class OtherIndexType, std::size_t... otherExts>
  static constexpr std::array<index_type,sizeof...(otherExts)>
  as_array (const Std::extents<OtherIndexType,otherExts...>& e) noexcept
  {
    return unpackIntegerSequence([&](auto... ii) {
      return std::array<index_type,sizeof...(otherExts)>{index_type(e.extent(ii))...}; },
      std::make_index_sequence<sizeof...(otherExts)>{});
  }

  // Copy only the dynamic extents from the container `e` into the `dynamic_extents_` storage
  template <std::size_t N, class Container>
  constexpr void init_dynamic_extents (const Container& e) noexcept
  {
    if constexpr(rank_dynamic() > 0) {
      if constexpr(N == rank_dynamic()) {
        assert(e.size() == rank_dynamic());
        for (rank_type i = 0; i < rank_dynamic(); ++i)
          dynamic_extents_[i] = e[i];
      } else {
        assert(e.size() == rank());
        for (rank_type i = 0, j = 0; i < rank(); ++i) {
          if (static_extent(i) == Std::dynamic_extent)
            dynamic_extents_[j++] = e[i];
        }
      }
    }
  }
#endif // DOXYGEN

private:
  using dynamic_extents_type = typename Impl::DynamicExtentsArray<index_type,rank_dynamic()>::type;
  DUNE_NO_UNIQUE_ADDRESS dynamic_extents_type dynamic_extents_;

  template <class, std::size_t...> friend class extents;
  friend struct layout_left;
  friend struct layout_right;
  friend struct layout_stride;
};


namespace Impl {

template <class IndexType, class Seq>
struct DExtentsImpl;

template <class IndexType, std::size_t... I>
struct DExtentsImpl<IndexType, std::integer_sequence<std::size_t,I...>>
{
  using type = Std::extents<IndexType, (void(I), Std::dynamic_extent)...>;
};

} // end namespace Impl


/**
 * \brief Alias of `extents` of given rank `R` and purely dynamic extents.
 * See [mdspan.extents.dextents]
 * \ingroup CxxUtilities
 **/
template <class IndexType, std::size_t R>
using dextents = typename Impl::DExtentsImpl<IndexType, std::make_integer_sequence<std::size_t,R>>::type;

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_EXTENTS_HH
