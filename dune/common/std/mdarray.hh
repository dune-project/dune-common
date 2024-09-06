// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_MDARRAY_HH
#define DUNE_COMMON_STD_MDARRAY_HH

#include <algorithm>
#include <array>
#include <memory>
#include <vector>
#include <tuple>
#include <type_traits>
#if __has_include(<version>)
  #include <version>
#endif

#include <dune/common/indices.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/std/default_accessor.hh>
#include <dune/common/std/mdspan.hh>
#include <dune/common/std/memory.hh>
#include <dune/common/std/no_unique_address.hh>
#include <dune/common/std/span.hh>
#include <dune/common/std/impl/containerconstructiontraits.hh>

namespace Dune::Std {

/**
 * \brief An owning multi-dimensional array analog of mdspan.
 * \ingroup CxxUtilities
 * \nosubgrouping
 *
 * The implementation is inspired by the mdarray c++ standard proposals
 * <a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1684r5.html">P1684R5</a>.
 *
 * \b Example:
 * \code{.cpp}
    // two-dimensional array with 2 rows and 6 columns of static shape
    Dune::Std::mdarray<double, Dune::Std::extents<std::size_t,2,6>> m1{};

    // same two-dimensional array but with dynamic shape
    Dune::Std::mdarray<double, Dune::Std::dextents<std::size_t,2>> m2{2,6};

    // storage type similar to the Dune::FieldMatrix
    template <class T, int m, int n>
    using FieldMatrix = Dune::Std::mdarray<T,
      Dune::Std::extents<int,m,n>, Dune::Std::layout_right, std::array<T,m*n>>;

    // store data into the array using two indices
    for (std::size_t i = 0; i != m1.extent(0); i++)
        for (std::size_t j = 0; j != m1.extent(1); j++)
            m1(i, j) = i + j;  // or m1[i, j]
 * \endcode
 *
 * \note The interface of `Std::mdarray` provides the `operator[]` with multiple
 *       indices overload only if available in c++23. For older c++ versions
 *       the `operator()` overload is implemented.
 *
 * \tparam Element  The element type; a complete object type that is neither an abstract
 *                  class type nor an array type.
 * \tparam Extents  Specifies number of dimensions, their sizes, and which are known at
 *                  compile time. Must be a specialization of `Std::extents`.
 * \tparam LayoutPolicy   Specifies how to convert multi-dimensional index to underlying flat index.
 * \tparam Container      A container type accessible by a single index provided by the layout mapping.
 **/
template <class Element, class Extents, class LayoutPolicy = Std::layout_right,
          class Container = std::vector<Element>>
class mdarray
{
  template <class,class,class,class> friend class mdarray;

  static_assert(std::is_object_v<Element>);
  static_assert(!std::is_abstract_v<Element>);
  static_assert(!std::is_array_v<Element>);
  static_assert(std::is_same_v<Element, typename Container::value_type>);

public:
  using element_type = Element;
  using extents_type = Extents;
  using layout_type = LayoutPolicy;
  using container_type = Container;

  using value_type = element_type;
  using mapping_type = typename layout_type::template mapping<extents_type>;

  using index_type = typename extents_type::index_type;
  using size_type = typename extents_type::size_type;
  using rank_type = typename extents_type::rank_type;

  using mdspan_type = mdspan<element_type,extents_type,layout_type>;
  using const_mdspan_type = mdspan<const element_type,extents_type,layout_type>;

  using pointer = decltype(Std::to_address(std::declval<container_type>().begin()));
  using reference = typename container_type::reference;
  using const_pointer = decltype(Std::to_address(std::declval<container_type>().cbegin()));
  using const_reference = typename container_type::const_reference;

  static_assert(std::is_constructible_v<mapping_type, extents_type>);

private:
  // helper function to construct the container
  template <class C, class... Args>
  static constexpr auto construct_container (Args&&... args)
    -> decltype(Impl::ContainerConstructionTraits<C>::construct(std::forward<Args>(args)...))
  {
    return Impl::ContainerConstructionTraits<C>::construct(std::forward<Args>(args)...);
  }

public:
  /// \name mdarray constructors
  /// @{

  /// \brief A default constructor; needed only if the constructor for dynamic extents does not apply
  template <class E = extents_type, class C = container_type, class M = mapping_type,
    std::enable_if_t<(E::rank_dynamic() != 0), int> = 0,
    std::enable_if_t<std::is_default_constructible_v<C>, int> = 0,
    std::enable_if_t<std::is_default_constructible_v<M>, int> = 0>
  constexpr mdarray ()
    : container_{}
    , mapping_{}
  {}


  // -------------------------------------
  // constructors from extents or mappings


  /// \brief Construct from the dynamic extents
  template <class... IndexTypes,
    std::enable_if_t<(... && std::is_convertible_v<IndexTypes,index_type>), int> = 0,
    std::enable_if_t<std::is_constructible_v<extents_type,IndexTypes...>, int> = 0,
    std::enable_if_t<(... && std::is_nothrow_constructible_v<index_type,IndexTypes>), int> = 0>
  explicit constexpr mdarray (IndexTypes... exts)
    : mdarray(extents_type(index_type(std::move(exts))...))
  {}

  /// \brief Construct from the extents of the array
  explicit constexpr mdarray (const extents_type& e)
    : mdarray(mapping_type(e))
  {}

  /// \brief Construct from the layout mapping of the array
  template <class C = container_type,
    decltype(construct_container<C>(std::declval<std::size_t>()), bool{}) = true>
  explicit constexpr mdarray (const mapping_type& m)
    : container_(construct_container<C>(m.required_span_size()))
    , mapping_(m)
  {}


  // ---------------------------------------
  // constructors with a given initial value

  /// \brief Construct from extents and initial value
  constexpr mdarray (const extents_type& e, const value_type& v)
    : mdarray(mapping_type(e), v)
  {}

  /// \brief Construct from layout mapping and initial value
  template <class C = container_type,
    decltype(construct_container<C>(std::declval<std::size_t>(),std::declval<const value_type&>()), bool{}) = true>
  constexpr mdarray (const mapping_type& m, const value_type& v)
    : container_(construct_container<C>(m.required_span_size(), v))
    , mapping_(m)
  {}


  // -----------------------------------
  // constructors with a given container

  /// \brief Construct from extents and the storage container
  template <class E = extents_type,
    std::enable_if_t<std::is_constructible_v<mapping_type,const E&>, int> = 0>
  constexpr mdarray (const E& e, const container_type& c)
    : container_(c)
    , mapping_(e)
  {}

  /// \brief Construct from extents and the storage container
  template <class E = extents_type,
    std::enable_if_t<std::is_constructible_v<mapping_type,const E&>, int> = 0>
  constexpr mdarray (const E& e, container_type&& c)
    : container_(std::move(c))
    , mapping_(e)
  {}

  /// \brief Construct from layout mapping and the storage container
  constexpr mdarray (const mapping_type& m, const container_type& c)
    : container_(c)
    , mapping_(m)
  {}

  /// \brief Construct from layout mapping and the storage container
  constexpr mdarray (const mapping_type& m, container_type&& c)
    : container_(std::move(c))
    , mapping_(m)
  {}


  // -----------------------
  // converting constructors

  /// \brief Converting constructor from other mdarray
  template <class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class OtherContainer,
    std::enable_if_t<std::is_constructible_v<Container,const OtherContainer&>, int> = 0,
    std::enable_if_t<std::is_constructible_v<extents_type,OtherExtents>, int> = 0,
    std::enable_if_t<std::is_constructible_v<mapping_type,const typename OtherLayoutPolicy::template mapping<OtherExtents>&>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(
    !std::is_convertible_v<const typename OtherLayoutPolicy::template mapping<OtherExtents>&, mapping_type> ||
    !std::is_convertible_v<const OtherContainer&, container_type>)
  #endif
  constexpr mdarray (const mdarray<OtherElementType,OtherExtents,OtherLayoutPolicy,OtherContainer>& other)
    : container_(other.container_)
    , mapping_(other.mapping_)
  {}

  /// \brief Converting constructor from mdspan
  template <class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class Accessor,
    std::enable_if_t<std::is_constructible_v<value_type,typename Accessor::reference>, int> = 0,
    std::enable_if_t<std::is_assignable_v<typename Accessor::reference, value_type>, int> = 0,
    std::enable_if_t<std::is_constructible_v<mapping_type, const typename OtherLayoutPolicy::template mapping<OtherExtents>&>, int> = 0,
    decltype(construct_container<container_type>(std::declval<std::size_t>()), bool{}) = true>
  #if __cpp_conditional_explicit >= 201806L
  explicit(
    !std::is_convertible_v<const typename OtherLayoutPolicy::template mapping<OtherExtents>&, mapping_type> ||
    !std::is_convertible_v<typename Accessor::reference, value_type>)
  #endif
  constexpr mdarray (const mdspan<OtherElementType,OtherExtents,OtherLayoutPolicy,Accessor>& other)
    : container_(construct_container<container_type>(other.size()))
    , mapping_(other.mapping())
  {
    init_from_mdspan(other);
  }


  // ----------------------------
  // constructors with allocators

  /// \brief Construct from the extents of the array and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, std::size_t, Alloc>, int> = 0>
  constexpr mdarray (const extents_type& e, const Alloc& a)
    : mdarray(mapping_type(e), a)
  {}

  /// \brief Construct from the layout mapping of the array and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, std::size_t, Alloc>, int> = 0>
  constexpr mdarray (const mapping_type& m, const Alloc& a)
    : container_(m.required_span_size(), a)
    , mapping_(m)
  {}

  /// \brief Construct from extents, initial value and allocator
  template <class Alloc>
  constexpr mdarray (const extents_type& e, const value_type& v, const Alloc& a)
    : mdarray(mapping_type(e), v, a)
  {}

  /// \brief Construct from layout mapping, initial value and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, std::size_t, value_type, Alloc>, int> = 0>
  constexpr mdarray (const mapping_type& m, const value_type& v, const Alloc& a)
    : container_(m.required_span_size(), v, a)
    , mapping_(m)
  {}

  /// \brief Construct from extents, container and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, container_type, Alloc>, int> = 0>
  constexpr mdarray (const extents_type& e, const container_type& c, const Alloc& a)
    : container_(c, a)
    , mapping_(e)
  {}

  /// \brief Construct from extents, container and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, container_type, Alloc>, int> = 0>
  constexpr mdarray (const extents_type& e, container_type&& c, const Alloc& a)
    : container_(std::move(c), a)
    , mapping_(e)
  {}

  /// \brief Construct from layout mapping, container and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, container_type, Alloc>, int> = 0>
  constexpr mdarray (const mapping_type& m, const container_type& c, const Alloc& a)
    : container_(c, a)
    , mapping_(m)
  {}

  /// \brief Construct from layout mapping, container and allocator
  template <class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, container_type, Alloc>, int> = 0>
  constexpr mdarray (const mapping_type& m, container_type&& c, const Alloc& a)
    : container_(std::move(c), a)
    , mapping_(m)
  {}

  /// \brief Converting constructor with alternative allocator
  template <class V, class E, class L, class C, class Alloc,
    std::enable_if_t<std::is_constructible_v<container_type, C, Alloc>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(
    !std::is_convertible_v<const typename L::template mapping<E>&, mapping_type> ||
    !std::is_convertible_v<const C&, container_type>)
  #endif
  constexpr mdarray (const mdarray<V,E,L,C>& other, const Alloc& a) noexcept
    : container_(other.container_, a)
    , mapping_(other.mapping_)
  {}

  /// \brief Converting constructor from mdspan
  template <class V, class E, class L, class A, class Alloc,
    class C = container_type,
    class Al = typename C::allocator_type,
    std::enable_if_t<std::is_constructible_v<C, std::size_t, Alloc>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(
    !std::is_convertible_v<const typename L::template mapping<E>&, mapping_type> ||
    !std::is_convertible_v<typename A::reference, value_type> ||
    !std::is_convertible_v<Alloc, Al>)
  #endif
  constexpr mdarray (const mdspan<V,E,L,A>& other, const Alloc& a)
    : container_(other.size(), a)
    , mapping_(other.mapping_)
  {
    init_from_mdspan(other);
  }

  /// @}

private:

  template <class V, class E, class L, class A, class... Indices>
  void init_from_mdspan (const mdspan<V,E,L,A>& other, Indices... ii)
  {
    constexpr rank_type pos = sizeof...(Indices);
    if constexpr(pos < rank()) {
      for (typename E::index_type i = 0; i < other.extent(pos); ++i)
        init_from_mdspan(other,ii...,i);
    } else {
      using I = std::array<typename E::index_type,E::rank()>;
      container_[mapping_(index_type(ii)...)] = other[I{ii...}];
    }
  }

public:

  /// \name Multi index access
  /// @{

  /**
   * \brief Access element at position (i0,i1,...)
   * \note The `operator()` is not in the std proposal, but is provided for using mdspan without c++23.
   * For compatibility reasons it should only be used if the macro DUNE_HAVE_CXX_STD_MDARRAY is explicitly
   * set to 0.
   **/
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<Indices,index_type>), int> = 0>
  constexpr reference operator() (Indices... indices)
  {
    return container_[mapping_(index_type(std::move(indices))...)];
  }

  /**
   * \brief Access element at position (i0,i1,...)
   * \note The `operator()` is not in the std proposal, but is provided for using mdspan without c++23.
   **/
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<Indices,index_type>), int> = 0>
  constexpr const_reference operator() (Indices... indices) const
  {
    return container_[mapping_(index_type(std::move(indices))...)];
  }


#if __cpp_multidimensional_subscript >= 202110L

  /// \brief Access element at position [i0,i1,...]
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<Indices,index_type>), int> = 0>
  constexpr reference operator[] (Indices... indices)
  {
    return container_[mapping_(index_type(std::move(indices))...)];
  }

  /// \brief Access element at position [i0,i1,...]
  template <class... Indices,
    std::enable_if_t<(sizeof...(Indices) == extents_type::rank()), int> = 0,
    std::enable_if_t<(... && std::is_convertible_v<Indices,index_type>), int> = 0>
  constexpr const_reference operator[] (Indices... indices) const
  {
    return container_[mapping_(index_type(std::move(indices))...)];
  }

#else

  /// \brief Access specified element at position [i0]
  /// For a rank one mdarray, the operator[i] is added to support bracket access before __cpp_multidimensional_subscript is supported.
  template <class Index, class E = extents_type,
    std::enable_if_t<std::is_convertible_v<Index,index_type>, int> = 0,
    std::enable_if_t<(E::rank() == 1), int> = 0>
  constexpr reference operator[] (Index index)
  {
    return container_[mapping_(index_type(std::move(index)))];
  }

  /// \brief Access specified element at position [i0]
  /// For a rank one mdarray, the operator[i] is added to support bracket access before __cpp_multidimensional_subscript is supported.
  template <class Index, class E = extents_type,
    std::enable_if_t<std::is_convertible_v<Index,index_type>, int> = 0,
    std::enable_if_t<(E::rank() == 1), int> = 0>
  constexpr const_reference operator[] (Index index) const
  {
    return container_[mapping_(index_type(std::move(index)))];
  }

#endif


  /// \brief Access element at position [{i0,i1,...}]
  template <class Index,
    std::enable_if_t<std::is_convertible_v<const Index&, index_type>, int> = 0>
  constexpr reference operator[] (Std::span<Index,extents_type::rank()> indices)
  {
    return unpackIntegerSequence([&](auto... ii) -> reference {
      return container_[mapping_(index_type(indices[ii])...)]; },
      std::make_index_sequence<extents_type::rank()>{});
  }

  /// \brief Access element at position [{i0,i1,...}]
  template <class Index,
    std::enable_if_t<std::is_convertible_v<const Index&, index_type>, int> = 0>
  constexpr const_reference operator[] (Std::span<Index,extents_type::rank()> indices) const
  {
    return unpackIntegerSequence([&](auto... ii) -> const_reference {
      return container_[mapping_(index_type(indices[ii])...)]; },
      std::make_index_sequence<extents_type::rank()>{});
  }

  /// \brief Access element at position [{i0,i1,...}]
  template <class Index,
    std::enable_if_t<std::is_convertible_v<const Index&, index_type>, int> = 0>
  constexpr reference operator[] (const std::array<Index,extents_type::rank()>& indices)
  {
    return std::apply([&](auto... ii) -> reference {
      return container_[mapping_(index_type(ii)...)]; }, indices);
  }

  /// \brief Access element at position [{i0,i1,...}]
  template <class Index,
    std::enable_if_t<std::is_convertible_v<const Index&, index_type>, int> = 0>
  constexpr const_reference operator[] (const std::array<Index,extents_type::rank()>& indices) const
  {
    return std::apply([&](auto... ii) -> const_reference {
      return container_[mapping_(index_type(ii)...)]; }, indices);
  }

  /// @}


  /// \brief Number of elements in all dimensions of the array, \related extents
  constexpr const extents_type& extents () const noexcept { return mapping_.extents(); }

  /// \brief Index mapping of a layout policy
  constexpr const mapping_type& mapping () const noexcept { return mapping_; }

  /// \brief The underlying storage container
  constexpr const container_type& container () const noexcept { return container_; }

  /**
   * \brief Move the container out of the mdarray.
   *
   * Note that after moving out the container the mdarray remains in a valid but an unspecified state.
   * One can destruct it or move-assign to it (i.e. restore the container in some way).
   **/
  constexpr container_type&& extract_container () && noexcept { return std::move(container_); }


  /// \name Size information
  /// @{

  /// \brief Number of dimensions of the array
  static constexpr rank_type rank () noexcept { return extents_type::rank(); }

  /// \brief Number of dimension with dynamic size
  static constexpr rank_type rank_dynamic () noexcept { return extents_type::rank_dynamic(); }

  /// \brief Number of elements in the r'th dimension of the tensor
  static constexpr std::size_t static_extent (rank_type r) noexcept { return extents_type::static_extent(r); }

  /// \brief Number of elements in the r'th dimension of the tensor
  constexpr index_type extent (rank_type r) const noexcept { return extents().extent(r); }

  /// \brief Size of the multi-dimensional index space
  constexpr size_type size () const noexcept
  {
    size_type s = 1;
    for (rank_type r = 0; r < rank(); ++r)
      s *= extent(r);
    return s;
  }

  /// \brief Size of the underlying container
  constexpr std::size_t container_size () const { return container_.size(); }

  /// \brief Check whether the index space is empty
  [[nodiscard]] constexpr bool empty () const noexcept { return size() == 0; }

  /// \brief The stride along the specified dimension
  constexpr index_type stride (rank_type r) const { return mapping().stride(r); };

  /// @}


  static constexpr bool is_always_unique () noexcept { return mapping_type::is_always_unique(); }
  static constexpr bool is_always_exhaustive () noexcept { return mapping_type::is_always_exhaustive(); }
  static constexpr bool is_always_strided () noexcept { return mapping_type::is_always_strided(); }

  constexpr bool is_unique () const noexcept { return mapping_.is_unique(); }
  constexpr bool is_exhaustive () const noexcept { return mapping_.is_exhaustive(); }
  constexpr bool is_strided () const noexcept { return mapping_.is_strided(); }


  /// \name Direct access to the data
  /// @{

  /// \brief Direct access to the underlying data in the container
  constexpr pointer container_data () noexcept { return Std::to_address(container_.begin()); }

  /// \brief Direct access to the underlying const data in the container
  constexpr const_pointer container_data () const noexcept { return Std::to_address(container_.begin()); }

  /// @}


  friend constexpr void swap (mdarray& x, mdarray& y) noexcept
  {
    using std::swap;
    swap(x.container_, y.container_);
    swap(x.mapping_, y.mapping_);
  }


  /// \name Comparison operators
  /// @{

  friend constexpr bool operator== (const mdarray& lhs, const mdarray& rhs) noexcept
  {
    return lhs.mapping() == rhs.mapping() && lhs.container() == rhs.container();
  }

  /// @}

  /// \name Conversion into mdspan
  /// @{

  /// \brief Conversion operator to mdspan
  template <class V, class E, class L, class A,
    std::enable_if_t<std::is_assignable_v<mdspan<V,E,L,A>, mdspan_type>, int> = 0>
  constexpr operator mdspan<V,E,L,A> ()
  {
    return mdspan_type(container_data(), mapping());
  }

  /// \brief Conversion operator to mdspan
  template <class V, class E, class L, class A,
    std::enable_if_t<std::is_assignable_v<mdspan<V,E,L,A>, const_mdspan_type>, int> = 0>
  constexpr operator mdspan<V,E,L,A> () const
  {
    return const_mdspan_type(container_data(), mapping());
  }

  /// \brief Conversion function to mdspan
  template <class AccessorPolicy = Std::default_accessor<element_type>,
    std::enable_if_t<
      std::is_assignable_v<mdspan_type, mdspan<element_type,extents_type,layout_type,AccessorPolicy>>, int> = 0>
  constexpr mdspan<element_type,extents_type,layout_type,AccessorPolicy>
  to_mdspan (const AccessorPolicy& a = AccessorPolicy{})
  {
    return mdspan<element_type,extents_type,layout_type,AccessorPolicy>(container_data(), mapping(), a);
  }

  /// \brief Conversion function to mdspan
  template <class AccessorPolicy = Std::default_accessor<const element_type>,
    std::enable_if_t<
      std::is_assignable_v<const_mdspan_type, mdspan<const element_type,extents_type,layout_type,AccessorPolicy>>, int> = 0>
  constexpr mdspan<const element_type,extents_type,layout_type,AccessorPolicy>
  to_mdspan (const AccessorPolicy& a = AccessorPolicy{}) const
  {
    return mdspan<const element_type,extents_type,layout_type,AccessorPolicy>(container_data(), mapping(), a);
  }

protected:
  container_type container_;
  DUNE_NO_UNIQUE_ADDRESS mapping_type mapping_;
};

/// \name Deduction guides
/// \relates mdarray
/// @{

template <class IndexType, std::size_t... exts, class Container>
mdarray (const Std::extents<IndexType, exts...>&, const Container&)
  -> mdarray<typename Container::value_type, Std::extents<IndexType, exts...>, layout_right, Container>;

template <class Mapping, class Container>
mdarray (const Mapping&, const Container&)
  -> mdarray<typename Container::value_type, typename Mapping::extents_type, typename Mapping::layout_type, Container>;

template <class IndexType, std::size_t... exts, class Container>
mdarray (const Std::extents<IndexType, exts...>&, Container&&)
  -> mdarray<typename Container::value_type, Std::extents<IndexType, exts...>, layout_right, Container>;

template <class Mapping, class Container>
mdarray (const Mapping&, Container&&)
  -> mdarray<typename Container::value_type, typename Mapping::extents_type, typename Mapping::layout_type, Container>;

template <class Element, class Extents, class Layout, class Accessor>
mdarray (const mdspan<Element, Extents, Layout, Accessor>&)
  -> mdarray<std::remove_cv_t<Element>, Extents, Layout>;

template <class IndexType, std::size_t... exts, class Container, class Alloc>
mdarray (const Std::extents<IndexType, exts...>&, const Container&, const Alloc&)
  -> mdarray<typename Container::value_type, Std::extents<IndexType, exts...>, layout_right, Container>;

template <class Mapping, class Container, class Alloc>
mdarray (const Mapping&, const Container&, const Alloc&)
  -> mdarray<typename Container::value_type, typename Mapping::extents_type, typename Mapping::layout_type, Container>;

template <class IndexType, std::size_t... exts, class Container, class Alloc>
mdarray (const Std::extents<IndexType, exts...>&, Container&&, const Alloc&)
  -> mdarray<typename Container::value_type, Std::extents<IndexType, exts...>, layout_right, Container>;

template <class Mapping, class Container, class Alloc>
mdarray (const Mapping&, Container&&, const Alloc&)
  -> mdarray<typename Container::value_type, typename Mapping::extents_type, typename Mapping::layout_type, Container>;

template <class Element, class Extents, class Layout, class Accessor, class Alloc>
mdarray (const mdspan<Element, Extents, Layout, Accessor>&, const Alloc&)
  -> mdarray<std::remove_cv_t<Element>, Extents, Layout>;

/// @}

/// \name Additional deduction guide for mdspan
/// \relates mdspan
/// @{

template <class Element, class Extents, class Layout, class Container>
mdspan (mdarray<Element, Extents, Layout, Container>) -> mdspan<
  typename decltype(std::declval<mdarray<Element, Extents, Layout, Container>>().to_mdspan())::element_type,
  typename decltype(std::declval<mdarray<Element, Extents, Layout, Container>>().to_mdspan())::extents_type,
  typename decltype(std::declval<mdarray<Element, Extents, Layout, Container>>().to_mdspan())::layout_type,
  typename decltype(std::declval<mdarray<Element, Extents, Layout, Container>>().to_mdspan())::accessor_type>;

/// @}

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_MDARRAY_HH
