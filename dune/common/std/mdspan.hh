// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_MDSPAN_HH
#define DUNE_COMMON_STD_MDSPAN_HH

#include <array>
#include <type_traits>
#include <utility>
#if __has_include(<version>)
  #include <version>
#endif

#include <dune/common/indices.hh>
#include <dune/common/std/default_accessor.hh>
#include <dune/common/std/extents.hh>
#include <dune/common/std/layout_right.hh>
#include <dune/common/std/no_unique_address.hh>
#include <dune/common/std/span.hh>

namespace Dune::Std {

/**
 * \brief A multi-dimensional non-owning array view.
 * \ingroup CxxUtilities
 * \nosubgrouping
 *
 * The class `mdspan` is a view into a contiguous sequence of objects that reinterprets
 * it as a multidimensional array.
 *
 * The implementation is based in the standard proposal
 * <a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0009r17.html">P0009r17</a>
 * and the C++ standard working draft
 * <a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4971.pdf">N4971</a>.
 *
 * \b Example:
 * \code{.cpp}
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // view data as contiguous memory representing 2 rows and 5 columns
    // with row-major ordering.
    auto ms = Dune::Std::mdspan(v.data(), 2, 5);

    // write data using 2D view
    for (std::size_t i = 0; i != ms.extent(0); i++)
        for (std::size_t j = 0; j != ms.extent(1); j++)
            ms(i, j) = i + j;   // or ms[i, j]
 * \endcode
 *
 * \note The interface of `Std::mdspan` provides the `operator[]` with multiple
 *       indices overload only if available in c++23. For older c++ versions
 *       the `operator()` overload is implemented.
 *
 * \tparam Element  The element type; a complete object type that is neither an abstract
 *                  class type nor an array type.
 * \tparam Extents  Specifies number of dimensions, their sizes, and which are known at
 *                  compile time. Must be a specialization of `Std::extents`.
 * \tparam LayoutPolicy   Specifies how to convert multi-dimensional index to underlying flat index.
 * \tparam AccessorPolicy Specifies how to convert underlying 1D index to a reference to `Element`.
 **/
template <class Element, class Extents, class LayoutPolicy = Std::layout_right,
          class AccessorPolicy = Std::default_accessor<Element>>
class mdspan
{
  static_assert(std::is_object_v<Element>);
  static_assert(!std::is_abstract_v<Element>);
  static_assert(!std::is_array_v<Element>);
  static_assert(std::is_same_v<Element, typename AccessorPolicy::element_type>);

public:
  using element_type =	Element;
  using extents_type = Extents;
  using layout_type = LayoutPolicy;
  using accessor_type = AccessorPolicy;

  using value_type =	std::remove_cv_t<Element>;
  using mapping_type = typename layout_type::template mapping<extents_type>;
  using index_type = typename extents_type::index_type;
  using size_type = typename extents_type::size_type;
  using rank_type = typename extents_type::rank_type;
  using data_handle_type = typename accessor_type::data_handle_type;
  using reference = typename accessor_type::reference;

private:
  // [mdspan.layout.reqmts]
  static_assert(std::is_nothrow_move_constructible_v<mapping_type>);
  static_assert(std::is_nothrow_move_assignable_v<mapping_type>);
  static_assert(std::is_nothrow_swappable_v<mapping_type>);

  // [mdspan.accessor.reqmts]
  static_assert(std::is_nothrow_move_constructible_v<accessor_type>);
  static_assert(std::is_nothrow_move_assignable_v<accessor_type>);
  static_assert(std::is_nothrow_swappable_v<accessor_type>);

public:
  /// \name mdspan constructors [mdspan.mdspan.cons]
  /// @{

  /// \brief Default constructor value-initializes all members
  template <class E = extents_type, class D = data_handle_type, class M = mapping_type, class A = accessor_type,
    std::enable_if_t<(E::rank_dynamic() > 0), int> = 0,
    std::enable_if_t<std::is_default_constructible_v<D>, int> = 0,
    std::enable_if_t<std::is_default_constructible_v<M>, int> = 0,
    std::enable_if_t<std::is_default_constructible_v<A>, int> = 0>
  constexpr mdspan ()
    : data_handle_{}
    , mapping_{}
    , accessor_{}
  {}

  /// \brief Construct from the dynamic extents given as variadic list
  template <class... IndexTypes,
    class E = extents_type, class M = mapping_type, class A = accessor_type,
    std::enable_if_t<(sizeof...(IndexTypes) == E::rank() || sizeof...(IndexTypes) == E::rank_dynamic()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<IndexTypes, index_type>), int> = 0,
    std::enable_if_t<(... && std::is_nothrow_constructible_v<index_type, IndexTypes>), int> = 0,
    std::enable_if_t<std::is_constructible_v<M, E>, int> = 0,
    std::enable_if_t<std::is_default_constructible_v<A>, int> = 0>
  explicit constexpr mdspan (data_handle_type p, IndexTypes... exts)
    : mdspan(std::move(p), extents_type(index_type(std::move(exts))...))
  {}

  /// \brief Construct from the dynamic extents given as an array
  template <class IndexType, std::size_t N,
    std::enable_if_t<std::is_convertible_v<const IndexType&, index_type>, int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type,const IndexType&>, int> = 0,
    std::enable_if_t<(N == extents_type::rank_dynamic() || N == extents_type::rank()), int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(N != extents_type::rank_dynamic())
  #endif
  constexpr mdspan (data_handle_type p, Std::span<IndexType,N> exts)
    : mdspan(std::move(p), extents_type(exts))
  {}

  /// \brief Construct from the dynamic extents given as an array
  template <class IndexType, std::size_t N,
    std::enable_if_t<std::is_convertible_v<IndexType, index_type>, int> = 0,
    std::enable_if_t<(N == extents_type::rank_dynamic() || N == extents_type::rank()), int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(N != extents_type::rank_dynamic())
  #endif
  constexpr mdspan (data_handle_type p, const std::array<IndexType,N>& exts)
    : mdspan(std::move(p), extents_type(exts))
  {}

  /// \brief Construct from the pointer to the data of the tensor and its extents
  template <class M = mapping_type,
    std::enable_if_t<std::is_constructible_v<M, const extents_type&>, int> = 0>
  constexpr mdspan (data_handle_type p, const extents_type& e)
    : mdspan(std::move(p), mapping_type(e))
  {}

  /// \brief Construct from the pointer to the data of the tensor and an index mapping
  template <class A = accessor_type,
    std::enable_if_t<std::is_default_constructible_v<A>, int> = 0>
  constexpr mdspan (data_handle_type p, const mapping_type& m)
    : mdspan(std::move(p), m, accessor_type{})
  {}

  /// \brief Construct from the pointer to the data of the tensor, an index mapping, and an accessor.
  constexpr mdspan (data_handle_type p, const mapping_type& m, const accessor_type& a)
    : data_handle_(std::move(p))
    , mapping_(m)
    , accessor_(a)
  {}


  /// \brief Converting constructor
  template <class OtherElementType, class OtherExtends, class OtherLayoutPolicy, class OtherAccessor,
    std::enable_if_t<std::is_constructible_v<mapping_type, const typename OtherElementType::template mapping<OtherExtends>&>, int> = 0,
    std::enable_if_t<std::is_constructible_v<accessor_type, const OtherAccessor&>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(!std::is_convertible_v<const typename OtherElementType::template mapping<OtherExtends>&, mapping_type>
    || !std::is_convertible_v<const OtherAccessor&, accessor_type>)
  #endif
  constexpr mdspan (const mdspan<OtherElementType,OtherExtends,OtherLayoutPolicy,OtherAccessor>& other) noexcept
    : mdspan(data_handle_type(other.data_handle()), mapping_type(other.mapping()),
        accessor_type(other.accessor()))
  {}

  /// @}


  /// \name Multi index access
  /// @{

  /**
   * \brief Access specified element at position i0,i1,...
   * \note The `operator()` is not in the std proposal, but is provided for using mdspan without c++23.
   **/
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<Indices, index_type>), int> = 0,
    std::enable_if_t<(... && std::is_nothrow_constructible_v<index_type,Indices>), int> = 0>
  constexpr reference operator() (Indices... indices) const
  {
    return accessor_.access(data_handle_, mapping_(index_type(std::move(indices))...));
  }

#if __cpp_multidimensional_subscript >= 202110L

  /// \brief Access specified element at position i0,i1,...
  template <class... Indices,
  std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
  std::enable_if_t<(... && std::is_convertible_v<Indices, index_type>), int> = 0,
  std::enable_if_t<(... && std::is_nothrow_constructible_v<index_type,Indices>), int> = 0>
  constexpr reference operator[] (Indices... indices) const
  {
    return accessor_.access(data_handle_, mapping_(index_type(std::move(indices))...));
  }

#else // __cpp_multidimensional_subscript

  /// \brief Access specified element at position [i0]
  /// For a rank one mdspan, the operator[i] is added to support bracket access before __cpp_multidimensional_subscript is supported.
  template <class Index, class E = extents_type,
    std::enable_if_t<std::is_convertible_v<Index,index_type>, int> = 0,
    std::enable_if_t<(E::rank() == 1), int> = 0>
  constexpr reference operator[] (Index index) const
  {
    return accessor_.access(data_handle_, mapping_(index_type(std::move(index))));
  }

#endif // __cpp_multidimensional_subscript

  /// \brief Access specified element at position [i0,i1,...]
  template <class Index,
    std::enable_if_t<std::is_convertible_v<const Index&, index_type>, int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type, const Index&>, int> = 0>
  constexpr reference operator[] (Std::span<Index,extents_type::rank()> indices) const
  {
    return unpackIntegerSequence([&](auto... ii) -> reference {
      return accessor_.access(data_handle_, mapping_(index_type(indices[ii])...)); },
      std::make_index_sequence<extents_type::rank()>{});
  }

  /// \brief Access specified element at position [i0,i1,...]
  template <class Index,
    std::enable_if_t<std::is_convertible_v<const Index&, index_type>, int> = 0,
    std::enable_if_t<std::is_nothrow_constructible_v<index_type, const Index&>, int> = 0>
  constexpr reference operator[] (const std::array<Index,extents_type::rank()>& indices) const
  {
    return std::apply([&](auto... ii) -> reference {
      return accessor_.access(data_handle_, mapping_(index_type(ii)...)); }, indices);
  }

  /// @}

  /// \brief Number of elements in all dimensions of the tensor, \related extents
  constexpr const extents_type& extents () const noexcept { return mapping_.extents(); }

  /// \brief Index mapping of a layout policy
  constexpr const mapping_type& mapping () const noexcept { return mapping_; }

  /// \brief The accessor policy object
  constexpr const accessor_type& accessor () const noexcept { return accessor_; }

  /// \brief The pointer to the underlying flat sequence
  constexpr const data_handle_type& data_handle () const noexcept { return data_handle_; };


  /// \name Size information
  /// @{

  /// \brief Number of dimensions of the tensor
  static constexpr rank_type rank () noexcept { return extents_type::rank(); }

  /// \brief Number of dimensions of the tensor
  static constexpr rank_type rank_dynamic () noexcept { return extents_type::rank_dynamic(); }

  /// \brief Number of elements in the r'th dimension of the tensor
  static constexpr std::size_t static_extent (rank_type r) noexcept { return extents_type::static_extent(r); }

  /// \brief Number of elements in the r'th dimension of the tensor
  constexpr index_type extent (rank_type r) const noexcept { return extents().extent(r); }

  /// \brief The number of elements accessible by this multi-dimensional span
  constexpr size_type size () const noexcept
  {
    size_type s = 1;
    for (rank_type r = 0; r < rank(); ++r)
      s *= extent(r);
    return s;
  }

  /// \brief Checks if the size of the index space is zero
  [[nodiscard]] constexpr bool empty () const noexcept { return size() == 0; }

  /// @}


  /// \copydoc is_unique()
  static constexpr bool is_always_unique () { return mapping_type::is_always_unique(); }

  /// \copydoc is_exhaustive()
  static constexpr bool is_always_exhaustive () { return mapping_type::is_always_exhaustive(); }

  /// \copydoc is_strided()
  static constexpr bool is_always_strided () { return mapping_type::is_always_strided(); }

  /// \brief Return true only if for every i and j where (i != j || ...) => mapping(i...) != mapping(j...).
  constexpr bool is_unique () const { return mapping_.is_unique(); }

  /// \brief Return true only if for all k in the range [0, mapping.required_span_size() ) there
  /// exists an i such that mapping(i...) equals k.
  constexpr bool is_exhaustive () const { return mapping_.is_exhaustive(); }

  /**
   * \brief Return true only if for every rank index r of extents there exists an integer sr such that,
   * for all i where (i+dr) is a multidimensional index in extents, mapping((i+dr)...) - mapping(i...) equals sr. *
   * \note This implies that for a strided layout mapping(i0, ..., ik) = mapping(0, ..., 0) + i0 * s0 + ... + ik * sk.
   **/
  constexpr bool is_strided () const { return mapping_.is_strided(); }

  /// \brief The stride along the specified dimension
  constexpr index_type stride (rank_type r) const { return mapping_.stride(r); }


  /// \brief Overloads the `std::swap` algorithm for `std::mdspan`. Exchanges the state of `x` with that of `y`.
  friend constexpr void swap (mdspan& x, mdspan& y) noexcept
  {
    using std::swap;
    swap(x.data_handle_, y.data_handle_);
    swap(x.mapping_, y.mapping_);
    swap(x.accessor_, y.accessor_);
  }


private:
  data_handle_type data_handle_;
  DUNE_NO_UNIQUE_ADDRESS mapping_type mapping_;
  DUNE_NO_UNIQUE_ADDRESS accessor_type accessor_;
};

// deduction guides
// @{

template <class CArray,
  std::enable_if_t<std::is_array_v<CArray>, int> = 0,
  std::enable_if_t<(std::rank_v<CArray> == 1), int> = 0>
mdspan (CArray&)
  -> mdspan<std::remove_all_extents_t<CArray>, Std::extents<std::size_t, std::extent_v<CArray,0>>>;

template <class Pointer,
  std::enable_if_t<std::is_pointer_v<std::remove_reference_t<Pointer>>, int> = 0>
mdspan (Pointer&&)
  -> mdspan<std::remove_pointer_t<std::remove_reference_t<Pointer>>, Std::extents<std::size_t>>;

template <class ElementType, class... II,
  std::enable_if_t<(... && std::is_convertible_v<II,std::size_t>), int> = 0,
  std::enable_if_t<(sizeof...(II) > 0), int> = 0>
mdspan (ElementType*, II...)
  -> mdspan<ElementType, Std::dextents<std::size_t, sizeof...(II)>>;

template <class ElementType, class SizeType, std::size_t N>
mdspan (ElementType*, Std::span<SizeType,N>&)
  -> mdspan<ElementType, Std::dextents<std::size_t, N>>;

template <class ElementType, class SizeType, std::size_t N>
mdspan (ElementType*, const std::array<SizeType,N>&)
  -> mdspan<ElementType, Std::dextents<std::size_t, N>>;

template <class ElementType, class IndexType, std::size_t... exts>
mdspan (ElementType*, const Std::extents<IndexType,exts...>&)
  -> mdspan<ElementType, Std::extents<IndexType,exts...>>;

template <class ElementType, class Mapping,
  class Extents = typename Mapping::extents_type,
  class Layout = typename Mapping::layout_type>
mdspan (ElementType*, const Mapping&)
  -> mdspan<ElementType, Extents, Layout>;

template <class Mapping, class Accessor,
  class DataHandle = typename Accessor::data_handle_type,
  class Element = typename Accessor::element_type,
  class Extents = typename Mapping::extents_type,
  class Layout = typename Mapping::layout_type>
mdspan (const DataHandle&, const Mapping&, const Accessor&)
  -> mdspan<Element, Extents, Layout, Accessor>;

// @}

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_MDSPAN_HH
