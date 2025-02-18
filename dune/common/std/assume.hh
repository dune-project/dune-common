// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set ts=8 sw=2 et sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_ASSUME_HH
#define DUNE_COMMON_STD_ASSUME_HH

/**
 * \file Provide the macro `DUNE_ASSUME(...)` that is a portable assume expression
 * as a hint for the compiler/optimizer. If no equivalent to the assume attribute
 * is available, it falls back to an `assert()` macro. A detailed explanation of
 * portable assumptions is provided in the C++ standard proposal https://wg21.link/p1774r8.
 *
 * \b Caution:
 * Only use simple boolean expressions inside the `DUNE_ASSUME` macro.
 * Especially, do not use expressions with side effects. Those could result in
 * compiler-dependent behavior.
 *
 * The implementation is inspired by
 * https://stackoverflow.com/questions/63493968/reproducing-clangs-builtin-assume-for-gcc
 **/

// If the C++ standard attribute assume is available
#ifdef __has_cpp_attribute
  #if __has_cpp_attribute(assume) >= 202207L
    #define DUNE_ASSUME(...) [[assume(__VA_ARGS__)]]
  #endif
#endif

// if compiler intrinsics/attributes for assumptions are available
#ifndef DUNE_ASSUME
  #if defined(__clang__) && defined(__has_builtin)
    #if __has_builtin(__builtin_assume)
      #define DUNE_ASSUME(...) __builtin_assume(__VA_ARGS__)
    #endif
  #elif defined(_MSC_VER)
    #define DUNE_ASSUME(...) __assume(__VA_ARGS__)
  #elif defined(__GNUC__)
    #if __GNUC__ >= 13
      #define DUNE_ASSUME(...) __attribute__((__assume__(__VA_ARGS__)))
    #endif
  #endif
#endif

// if we are in release mode, use undefined behavior as a way to enforce an assumption
#if !defined(DUNE_ASSUME) && defined(NDEBUG)
  #include <utility>
  #if __cpp_lib_unreachable >= 202202L
    #define DUNE_ASSUME(...) do { if (!bool(__VA_ARGS__)) ::std::unreachable(); } while(0)
  #elif defined(__GNUC__)
    #define DUNE_ASSUME(...) do { if (!bool(__VA_ARGS__)) __builtin_unreachable(); } while(0)
  #elif defined(__has_builtin)
    #if __has_builtin(__builtin_unreachable)
      #define DUNE_ASSUME(...) do { if (!bool(__VA_ARGS__)) __builtin_unreachable(); } while(0)
    #endif
  #else
    #include <cstdlib>
    #define DUNE_ASSUME(...) do { if (!bool(__VA_ARGS__)) std::abort(); } while(0)
  #endif
#endif

// in debug mode and if not defined before, use the assert macro
#ifndef DUNE_ASSUME
  #include <cassert>
  #define DUNE_ASSUME(...) assert(bool(__VA_ARGS__))
#endif

#endif // DUNE_COMMON_STD_ASSUME_HH