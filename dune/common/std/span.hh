// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_SPAN_HH
#define DUNE_COMMON_STD_SPAN_HH

#include <cassert>
#include <cstddef>
#include <exception>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#if __has_include(<version>)
  #include <version>
#endif

#include <dune/common/exceptions.hh>
#include <dune/common/std/memory.hh>

namespace Dune::Std {

/// \brief A constant of type std::size_t that is used to differentiate std::span of static and dynamic extent.
inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

namespace Impl {

template <std::size_t Extent>
class SpanSize
{
public:
  using size_type = std::size_t;

public:
  constexpr SpanSize () = default;

  constexpr SpanSize ([[maybe_unused]] size_type size) noexcept
  {
    assert(Extent == Std::dynamic_extent || Extent == size);
  }

  template <class Iter>
  constexpr SpanSize ([[maybe_unused]] Iter first, [[maybe_unused]] Iter last) noexcept
  {
    assert((std::distance(first,last) == Extent));
  }

  constexpr size_type size () const noexcept { return Extent; }
};

template <>
class SpanSize<Std::dynamic_extent>
{
public:
  using size_type = std::size_t;

public:
  constexpr SpanSize (size_type size = 0) noexcept
    : size_(size)
  {}

  template <class Iter>
  constexpr SpanSize (Iter first, Iter last) noexcept
    : size_(std::distance(first,last))
  {}

  constexpr size_type size () const noexcept { return size_; }

private:
  size_type size_;
};

template <class T>
struct TypeIdentity { using type = T; };

template <class T>
using TypeIdentity_t = typename TypeIdentity<T>::type;

} // end namespace Impl


/**
 * \brief A contiguous sequence of elements with static or dynamic extent.
 * \ingroup CxxUtilities
 * \nosubgrouping
 *
 * The class template span describes an object that can refer to a contiguous sequence
 * of objects with the first element of the sequence at position zero. A span can either
 * have a static extent, in which case the number of elements in the sequence is known
 * at compile-time and encoded in the type, or a dynamic extent.
 *
 * If a span has dynamic extent, a typical implementation holds two members: a pointer
 * to `Element` and a size. A span with static extent may have only one member: a pointer
 * to `Element`.
 *
 * The implementation is based on the C++ standard working draft
 * <a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/n4971.pdf">N4971</a> and
 * the documentation provided in
 * <a href="https://en.cppreference.com/w/cpp/container/span">cppreference</a>.
 *
 * \b Example:
 * \code{.cpp}
    std::vector v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // view data as contiguous memory representing 10 elements
    auto s1 = Dune::Std::span(v.data(), 10);

    // view data as contiguous memory with static size
    auto s2 = Dune::Std::span<int,10>(v.data());

    // write data using 2D view
    for (std::size_t i = 0; i != s1.size(); i++)
      s1[i] = 2*i;
 * \endcode
 *
 * \tparam Element  The element type; a complete object type that is not an abstract class type.
 * \tparam Extent   Specifies number of elements in the sequence, or `Std::dynamic_extent` if dynamic.
 *
 * \related Std::dynamic_extent
 **/
template <class Element, std::size_t Extent = Std::dynamic_extent>
class span
    : public Impl::SpanSize<Extent>
{
  using base_type = Impl::SpanSize<Extent>; // base_type implements the member variable size()

  static_assert(std::is_object_v<Element> && !std::is_abstract_v<Element>);

public:
  using element_type = Element;
  using value_type = std::remove_cv_t<element_type>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = element_type*;
  using reference = element_type&;
  using const_reference  = const element_type&;
  using iterator = pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
#if __cpp_lib_ranges_as_const >202311L
  using const_iterator = std::const_iterator<iterator>;
  using const_reverse_iterator = std::const_iterator<reverse_iterator>;
#else
  using const_iterator = const iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
#endif


  static constexpr size_type extent = Extent;

public:
  /// \name Span constructors
  /// @{

  /// \brief Default construct an empty span
  template <std::size_t e = extent,
    std::enable_if_t<(e == dynamic_extent || e == 0), int> = 0>
  constexpr span () noexcept
    : base_type{}
    , data_{}
  {}

  /// \brief Constructs a span that is a view over the range `[first, first+size)`
  template <class Iter,
    class U = std::remove_reference_t<decltype(*std::declval<Iter>())>,
    std::enable_if_t<std::is_convertible_v<U(*)[], element_type(*)[]>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(extent != Std::dynamic_extent)
  #endif
  constexpr span (Iter first, size_type size)
    : base_type(size)
    , data_(Std::to_address(first))
  {}

  /// \brief Constructs a span that is a view over the range `[first, last)`
  template <class Iter,
    class U = std::remove_reference_t<decltype(*std::declval<Iter>())>,
    std::enable_if_t<std::is_convertible_v<U(*)[], element_type(*)[]>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(extent != Std::dynamic_extent)
  #endif
  constexpr span (Iter first, Iter last)
    : base_type(first,last)
    , data_(Std::to_address(first))
  {}

  /// \brief Constructs a span that is a view over the range `[range.begin(), range.end())`
  template <class Range,
    decltype(std::begin(std::declval<Range>()), std::end(std::declval<Range>()), bool{}) = true,
    std::enable_if_t<not std::is_array_v<Range>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(extent != Std::dynamic_extent)
  #endif
  constexpr span (Range& range)
    : span(std::begin(range), std::end(range))
  {}

  /// \brief Constructs a span that is a view over the C-array
  template <std::size_t N, std::size_t e = extent,
    std::enable_if_t<(e == Std::dynamic_extent || e == N), int> = 0>
  constexpr span (Impl::TypeIdentity_t<element_type> (&data)[N]) noexcept
    : base_type(N)
    , data_(data)
  {}

  /// \brief Constructs a span that is a view over the array
  template <class T, size_t N, std::size_t e = extent,
    std::enable_if_t<(e == Std::dynamic_extent || e == N), int> = 0,
    std::enable_if_t<std::is_convertible_v<T(*)[], element_type(*)[]>, int> = 0>
  constexpr span (std::array<T, N>& arr) noexcept
    : base_type(N)
    , data_(arr.data())
  {}

  /// \brief Constructs a span that is a view over the const array
  template <class T, size_t N, std::size_t e = extent,
    std::enable_if_t<(e == Std::dynamic_extent || e == N), int> = 0,
    std::enable_if_t<std::is_convertible_v<const T(*)[], element_type(*)[]>, int> = 0>
  constexpr span (const std::array<T, N>& arr) noexcept
    : base_type(N)
    , data_(arr.data())
  {}

  /// \brief Constructs a span that is a view over the initializer-list
  template <class E = element_type,
    std::enable_if_t<std::is_const_v<E>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(extent != Std::dynamic_extent)
  #endif
  constexpr span (std::initializer_list<value_type> il)
    : base_type(il.size())
    , data_(il.begin())
  {}

  /// \brief Copy constructor
  constexpr span (const span& other) noexcept = default;

  /// \brief Converting copy constructor
  template <class OtherElementType, std::size_t OtherExtent,
    std::enable_if_t<(extent == Std::dynamic_extent || OtherExtent == Std::dynamic_extent || extent == OtherExtent), int> = 0,
    std::enable_if_t<std::is_convertible_v<OtherElementType(*)[], element_type(*)[]>, int> = 0>
  #if __cpp_conditional_explicit >= 201806L
  explicit(extent != Std::dynamic_extent && OtherExtent == Std::dynamic_extent)
  #endif
  constexpr span (const span<OtherElementType, OtherExtent>& s) noexcept
    : base_type(s.size())
    , data_(s.data())
  {}

  /// \brief Copy assignment operator
  constexpr span& operator= (const span& other) noexcept = default;

  /// @}


  /// \name Iterators
  /// @{

  /// \brief Returns an iterator to the beginning.
  constexpr iterator begin () const noexcept { return data_; }

  /// \brief Returns an iterator to the end.
  constexpr iterator end () const noexcept { return data_ + size(); }

  /// \brief Returns an iterator to the beginning.
  constexpr const_iterator cbegin () const noexcept { return data_; }

  /// \brief Returns an iterator to the end.
  constexpr const_iterator cend () const noexcept { return data_ + size(); }

  /// \brief Returns a reverse iterator starting at the end.
  constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{end()}; }

  /// \brief Returns a reverse iterator ending at the beginning.
  constexpr reverse_iterator rend() const noexcept { return reverse_iterator{begin()}; }

  /// \brief Returns a reverse iterator starting at the end.
  constexpr const_reverse_iterator  crbegin() const noexcept { return reverse_iterator{end()}; }

  /// \brief Returns a reverse iterator ending at the beginning.
  constexpr const_reverse_iterator  crend() const noexcept { return reverse_iterator{begin()}; }

  /// @}


  /// \name Element and data access
  /// @{

  /// \brief Access the first element.
  constexpr reference front () const
  {
    assert(not empty() && "front of empty span does not exist");
    return data_[0];
  }

  /// \brief Access the last element.
  constexpr reference back () const
  {
    assert(not empty() && "front of empty span does not exist");
    return data_[size()-1];
  }

  /// \brief Access specified element with bounds checking.
  constexpr reference at (size_type i) const
  {
    if (i >= size())
      throw std::out_of_range("Index " + std::to_string(i) + " out of range.");
    return data_[i];
  }

  /// \brief Access specified element.
  constexpr reference operator[] (size_type i) const { return data_[i]; }

  /// \brief Direct access to the underlying contiguous storage
  constexpr pointer data () const noexcept { return data_; }

  /// @}


  /// \name Subspans
  /// @{

  /// \brief Obtains a subspan consisting of the first `Count` elements of the sequence
  template <std::size_t Count>
  constexpr span<element_type, Count> first () const
  {
    static_assert(Count <= Extent);
    assert(Count <= size());
    return span<element_type, Count>{data(), Count};
  }

  /// \brief Obtains a subspan consisting of the last `Count` elements of the sequence
  template <std::size_t Count>
  constexpr span<element_type, Count> last () const
  {
    static_assert(Count <= Extent);
    assert(Count <= size());
    return span<element_type, Count>{data()+ (size() - Count), Count};
  }

private:

  static constexpr std::size_t subspan_extent (std::size_t O, std::size_t C) noexcept
  {
    return (C != Std::dynamic_extent) ? C :
      (Extent != Std::dynamic_extent) ? Extent - O : Std::dynamic_extent;
  }

public:

  /// \brief Obtains a subspan consisting of `Count` elements of the sequence starting at `Offset`.
  /**
   * \note If `Count == Std::dynamic_extent`, the subspan starting at `Offset` goes
   * until the end of the current span.
   */
  template <std::size_t Offset, std::size_t Count = Std::dynamic_extent>
  constexpr span<element_type, subspan_extent(Offset,Count)> subspan () const
  {
    static_assert(Offset <= Extent && (Count == Std::dynamic_extent || Count <= Extent - Offset));
    assert(Offset <= size() && (Count == Std::dynamic_extent || Count <= size() - Offset));
    return span<element_type, subspan_extent(Offset,Count)>{
      data() + Offset, Count != Std::dynamic_extent ? Count : size() - Offset};
  }

  /// \brief Obtains a subspan consisting of the first `count` elements of the sequence
  constexpr span<element_type, Std::dynamic_extent> first (size_type count) const
  {
    assert(count <= size());
    return span<element_type, Std::dynamic_extent>{data(), count};
  }

  /// \brief Obtains a subspan consisting of the last `count` elements of the sequence
  constexpr span<element_type, Std::dynamic_extent> last (size_type count) const
  {
    assert(count <= size());
    return span<element_type, Std::dynamic_extent>{data()+ (size() - count), count};
  }

  /// \brief Obtains a subspan consisting of `count` elements of the sequence starting at `offset`.
  /**
   * \note If `count == Std::dynamic_extent`, the subspan starting at `offset` goes
   * until the end of the current span.
   */
  constexpr span<element_type, Std::dynamic_extent> subspan (size_type offset, size_type count = Std::dynamic_extent) const
  {
    assert(offset <= size() && (count == Std::dynamic_extent || count <= size() - offset));
    return span<element_type, Std::dynamic_extent>{
      data() + offset, count == Std::dynamic_extent ? size() - offset : count};
  }

  /// @}


  /// \name Size information
  /// @{

  /// \brief Returns the number of elements.
  using base_type::size;

  /// \brief Returns the size of the sequence in bytes.
  constexpr size_type size_bytes () const noexcept { return size() * sizeof(element_type); }

  /// \brief Checks if the sequence is empty.
  [[nodiscard]] constexpr bool empty () const noexcept { return size() == 0; }

  /// @}

private:
  pointer data_;
};

// deduction guide
// @{

template <class T, std::size_t N>
span (T (&)[N])
  -> span<T, N>;

template <class ElementType, class I, std::size_t Extent,
  std::enable_if_t<std::is_convertible_v<I,std::size_t>, int> = 0>
span (ElementType*, std::integral_constant<I,Extent>)
  -> span<ElementType, Extent>;

template <class ElementType, class I,
  std::enable_if_t<std::is_integral_v<I>, int> = 0,
  std::enable_if_t<std::is_convertible_v<I,std::size_t>, int> = 0>
span (ElementType*, I)
  -> span<ElementType, Std::dynamic_extent>;

template <class Iter,
  class Element = std::remove_reference_t<decltype(*std::declval<Iter>())>>
span (Iter,Iter)
  -> span<Element, Std::dynamic_extent>;

template <class Range,
  class First = decltype(std::begin(std::declval<Range>())),
  class Last = decltype(std::end(std::declval<Range>())),
  class Element = std::remove_reference_t<decltype(*std::declval<First>())>>
span (Range&)
  -> span<Element, Std::dynamic_extent>;

template <class T, size_t N>
span (std::array<T, N>&) -> span<T, N>;

template <class T, size_t N>
span (const std::array<T, N>&) -> span<const T, N>;

// @}

} // end namespace Dune::Std

#endif // DUNE_COMMON_STD_SPAN_HH
