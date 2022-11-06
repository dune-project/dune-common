// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_COPYABLE_OPTIONAL_HH
#define DUNE_COMMON_COPYABLE_OPTIONAL_HH

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>

#include <dune/common/typeutilities.hh>

namespace Dune {

/**
 * \brief A copyable type wrapper that provides copy/move assignment
 * operations for types that are only copy/move constructible.
 *
 * Some types, like lambdas, LocalFiniteElementCache, Geometries,... provide
 * a copy/move constructor but do not provide a corresponding assignment
 * operator. Mostly, the assignment operators can be implemented in terms of
 * the corresponding constructors. This type wrapper provides these assignment
 * operators by internally wrapping the type into a std::optional.
 *
 * \tparam Type  The type to be wrapped and equipped with copy operations.
 *
 * \b Requirements: `Type` must be an object type that is copy-constructible
 **/
template <class Type>
class CopyableOptional
    : public std::optional<Type>
{
  static_assert(std::is_copy_constructible_v<Type>);
  static_assert(std::is_object_v<Type>);

  using Base = std::optional<Type>;

public:

  /**
   * \brief Implementation of a default constructor, if the `Type` is itself
   * default constructible. The internal optional contains a value-initialized `Type`.
   **/
  template <class T = Type,
    std::enable_if_t<std::is_default_constructible_v<T>, int> = 0>
  constexpr CopyableOptional ()
        noexcept(std::is_nothrow_default_constructible_v<T>)
    : Base{std::in_place}
  {}

  /**
   * \brief Construct the internal data from perfect forwarding of the passed arguments.
   * Participates in overload resolution if `T` is implicitly convertible to `Type`.
   **/
  template <class T = Type,
    disableCopyMove<CopyableOptional,T> = 0,
    std::enable_if_t<std::is_constructible_v<Type,T&&>, int> = 0,
    std::enable_if_t<std::is_convertible_v<T&&,Type>, int> = 0>
  constexpr CopyableOptional (T&& value)
        noexcept(std::is_nothrow_constructible_v<Type,T&&>)
    : Base{std::in_place, std::forward<T>(value)}
  {}

  /**
   * \brief Construct the internal data from perfect forwarding of the passed arguments.
   * Participates in overload resolution if `T` is \b not implicitly convertible to `Type`.
   **/
  template <class T = Type,
    disableCopyMove<CopyableOptional,T> = 0,
    std::enable_if_t<std::is_constructible_v<Type,T&&>, int> = 0,
    std::enable_if_t<not std::is_convertible_v<T&&,Type>, int> = 0>
  explicit constexpr CopyableOptional (T&& value)
        noexcept(std::is_nothrow_constructible_v<Type,T&&>)
    : Base{std::in_place, std::forward<T>(value)}
  {}

  /// \brief Construct the internal data from perfect forwarding of the passed arguments.
  template <class... Args,
    disableCopyMove<CopyableOptional, Args...> = 0,
    std::enable_if_t<(sizeof...(Args) > 1), int> = 0,
    std::enable_if_t<std::is_constructible_v<Type,Args&&...>, int> = 0>
  constexpr CopyableOptional (Args&&... args)
        noexcept(std::is_nothrow_constructible_v<Type,Args&&...>)
    : Base{std::in_place, std::forward<Args>(args)...}
  {}

  /// \brief Copy construct the contained value
  constexpr CopyableOptional (const CopyableOptional&) = default;

  /// \brief Move construct the contained value
  constexpr CopyableOptional (CopyableOptional&&) = default;

  /// \brief Default destructor
  ~CopyableOptional () = default;

  /// \brief Copy assignment in terms of copy constructor
  constexpr CopyableOptional& operator= (const CopyableOptional& that)
        noexcept(std::is_nothrow_copy_assignable_v<Type> ||
          (!std::is_copy_assignable_v<Type> && std::is_nothrow_copy_constructible_v<Type>))
  {
    if constexpr(std::is_copy_assignable_v<Type>)
      Base::operator=(that);
    else {
      // no self-assignment
      if (this != std::addressof(that)) {
        if (that)
          Base::emplace(*that);
        else
          Base::reset();
      }
    }
    return *this;
  }

  /// \brief Move assignment in terms of move constructor
  template <class T = Type,
    std::enable_if_t<std::is_move_constructible_v<T>, int> = 0>
  constexpr CopyableOptional& operator= (CopyableOptional&& that)
        noexcept(std::is_nothrow_move_assignable_v<Type> ||
          (!std::is_move_assignable_v<Type> && std::is_nothrow_move_constructible_v<Type>))
  {
    if constexpr(std::is_move_assignable_v<Type>)
      Base::operator=(std::move(that));
    else {
      // no self-assignment
      if (this != std::addressof(that)) {
        if (that)
          Base::emplace(std::move(*that));
        else
          Base::reset();
      }
    }
    return *this;
  }

  /// \brief Perfect-forwarded assignment or construction
  template <class T = Type,
    std::enable_if_t<not std::is_same_v<std::decay_t<T>, CopyableOptional>, int> = 0,
    std::enable_if_t<(std::is_assignable_v<Type&,T> || std::is_constructible_v<Type,T>), int> = 0>
  constexpr CopyableOptional& operator= (T&& value)
        noexcept(std::is_nothrow_assignable_v<Type&,T> ||
          (!std::is_assignable_v<Type&,T> && std::is_nothrow_constructible_v<Type,T>))
  {
    if constexpr(std::is_assignable_v<Type&,T>)
      Base::operator=(std::forward<T>(value));
    else
      Base::emplace(std::forward<T>(value));
    return *this;
  }
};

} // end namespace Dune

#endif // DUNE_COMMON_COPYABLE_OPTIONAL_HH
