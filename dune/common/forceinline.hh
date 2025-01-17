// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_FORCEINLINE_HH
#define DUNE_COMMON_FORCEINLINE_HH

/**
 * \file Provide the  macro `DUNE_FORCE_INLINE` that expands to attribute `always_inline`
 * or similar depending on the compiler (version). Some information is extracted from
 * https://meghprkh.github.io/blog/posts/c++-force-inline.
 *
 * The effect of DUNE_FORCE_INLINE is typically as follows:
 * - Inlining heuristics are disabled and inlining is always attempted regardless
 *   of optimization level.
 * - Ignore `-fno-inline`
 * - Ignore the inlining limits hence inlining the function regardless. It also
 *   inlines functions with allocation calls, which `inline` keyword never does.
 *
 * Even when marked with DUNE_FORCE_INLINE, the compiler cannot always inline a
 * function. Examples for such exceptions are
 * - The function is virtual and is called virtually.
 * - The program calls a function indirectly via a pointer to the function.
 * - The function uses a variable argument list.
 *
 * \b Example:
 * \code{.cpp}
   DUNE_FORCE_INLINE int maxInt (int a, int b) { return a < b ? b : a; }
 * \endcode
 **/

#if defined(__clang__)
  // Clang does not generate an error for non-inlinable `always_inline` functions.
  #define DUNE_FORCE_INLINE __attribute__((always_inline)) inline

#elif defined(__GNUC__)
  // GCC would generate an error if it can't always_inline
  #define DUNE_FORCE_INLINE __attribute__((always_inline)) inline

#elif defined(_MSC_VER)
  // MSVC generates a warning for for non-inlinable `__forceinline` functions. But
  // only if compiled with any "inline expansion" optimization (`/Ob<n>`). This is
  // present with `/O1` or `/O2`. We promote this warning to an error.
  #pragma warning(error: 4714)
  #define DUNE_FORCE_INLINE [[msvc::forceinline]]

#else
  // fallback to `inline` in case the compiler cannot be detected.
  #define DUNE_FORCE_INLINE inline
#endif

#endif // DUNE_COMMON_FORCEINLINE_HH
