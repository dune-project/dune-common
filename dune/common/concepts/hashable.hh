// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_CONCEPTS_HASHABLE_HH
#define DUNE_COMMON_CONCEPTS_HASHABLE_HH

// check whether c++20 concept can be used
#if __has_include(<version>) && __has_include(<concepts>)
  #include <version>
  #if  __cpp_concepts >= 201907L && __cpp_lib_concepts >= 202002L
    #ifndef DUNE_ENABLE_CONCEPTS
    #define DUNE_ENABLE_CONCEPTS 1
    #endif
  #endif
#endif

#if DUNE_ENABLE_CONCEPTS

#include <concepts>
#include <cstddef>
#include <functional>

namespace Dune::Concept {

/**
 * \brief Detects hashable types usable as keys in unordered containers.
 * \ingroup CxxConcepts
 *
 * A type is a model of `Hashable` if it can be used directly as key type in
 * `std::unordered_map` or `std::unordered_set`.
 *
 * \par Notation:
 * - `a`: an object of type `T`
 *
 * \par Valid Expressions:
 * - `std::hash<T>{}(a)`: the standard hash function can be called on objects of type `T`
 *
 * \par Models:
 * - `int`, `double`
 * - `Dune::ReservedVector`
 * - `Dune::bigunsignedint`
 *
 * \experimental
 * \hideinitializer
 **/
template<class T>
concept Hashable = requires(T a)
{
  { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

} // end namespace Dune::Concept

#endif // DUNE_ENABLE_CONCEPTS

#endif // DUNE_COMMON_CONCEPTS_HASHABLE_HH
