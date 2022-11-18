// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_CONCEPTS_CONTAINER_HH
#define DUNE_COMMON_CONCEPTS_CONTAINER_HH

#include <concepts>
#include <iterator>

#if __has_include(<ranges>)
#include <ranges>
#endif

namespace Dune::Concept {

/**
 * \brief A `Container` can store objects of the same type and provides function to iterate over its elements.
 * \ingroup CxxConcepts
 *
 * A type is a model of `Container` if it provides iterators over its range of elements and a notion of size, i.e.,
 * the number of elements stored in the container.
 *
 * \par Notation:
 * - `c`: a container of type `T`
 *
 * \par Valid Expressions:
 * - `c.begin(), a.end()`: an iterator to the first (or past-last) element in the container
 * - `c.cbegin(), a.cend()`: a const iterator to the first (or past-last) element in the container
 * - `c.size()`: The number of elements stored in the container, i.e., `std::distance(c.begin(), c.end())`
 * - `c.max_size()`: The size of the largest possible container
 * - `c.empty()`: true if no elements are stored in the container, i.e., equivalent to `c.size()==0`
 *
 * \par Models:
 * - `std::vector`
 * - `std::list`
 * - `Dune::ReservedVector`
 *
 * \note
 * The data structure `Dune::FieldVector` it not a model of this concept, since some methods are missing.
 *
 * \experimental
 * \hideinitializer
 **/
template<class T>
concept Container =
  std::regular<T> &&
  std::swappable<T> &&
#if __has_include(<ranges>)
  std::ranges::range<T> &&
#endif
requires(T a, const T ca)
{
  typename T::value_type;
  requires std::unsigned_integral<typename T::size_type>;
  requires std::forward_iterator<typename T::iterator>;
  requires std::forward_iterator<typename T::const_iterator>;
  { a.begin()    } -> std::same_as<typename T::iterator>;
  { a.end()      } -> std::same_as<typename T::iterator>;
  { ca.begin()   } -> std::same_as<typename T::const_iterator>;
  { ca.end()     } -> std::same_as<typename T::const_iterator>;
  { a.cbegin()   } -> std::same_as<typename T::const_iterator>;
  { a.cend()     } -> std::same_as<typename T::const_iterator>;
  { a.size()     } -> std::same_as<typename T::size_type>;
  { a.max_size() } -> std::same_as<typename T::size_type>;
  { a.empty()    } -> std::convertible_to<bool>;
};

/**
 * \brief A \ref Container with direct element access and random-access iterators
 * \ingroup CxxConcepts
 *
 * \par Refinement of:
 * - \ref Container<T>
 * - \ref std::ranges::random_access_range<T>
 *
 * \par Notation:
 * - `c`: a container of type `T`
 * - `i`: an unsigned integer
 *
 * \par Valid Expressions:
 * - `c[i]`: access the i'th element in the container
 * - `c.begin() + i`: random access of iterators
 *
 * \par Models:
 * - `std::vector`
 *
 * \experimental
 * \hideinitializer
 **/
template<class T>
concept RandomAccessContainer =
  Container<T> &&
#if __has_include(<ranges>)
  std::ranges::random_access_range<T> &&
#endif
requires(T a, const T ca, typename T::size_type i)
{
  requires std::same_as<typename T::reference, typename T::value_type&>;
  requires std::same_as<typename T::const_reference, const typename T::value_type&>;
  requires std::random_access_iterator<typename T::iterator>;
  requires std::random_access_iterator<typename T::const_iterator>;
  { a[i]  } -> std::same_as<typename T::reference>;
  { ca[i] } -> std::same_as<typename T::const_reference>;
};

} // end namespace Dune::Concept

#endif // DUNE_COMMON_CONCEPTS_CONTAINER_HH
