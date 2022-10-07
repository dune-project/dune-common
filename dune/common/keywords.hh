// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_KEYWORDS_HH
#define DUNE_COMMON_KEYWORDS_HH

/** \file
 * \brief Definitions of several macros that conditionally make C++ syntax
 *      available.
 *
 * This header contains several macros that enable C++ features depending on your
 * compiler. Most of these features are optional and provide additional functionality
 * like making code constexpr.
 *
 * \ingroup CxxUtilities
 */


#if __cpp_inline_variables >= 201606
#define DUNE_INLINE_VARIABLE inline
#else
//! Preprocessor macro used for marking variables inline on supported compilers.
/**
 * \ingroup CxxUtilities
 */
#define DUNE_INLINE_VARIABLE
#endif


#if __cpp_constexpr >= 201304
#define DUNE_GENERALIZED_CONSTEXPR constexpr
#else
//! Preprocessor macro used for marking code as constexpr under the relaxed rules of C++14 if supported by the compiler.
/**
 * \ingroup CxxUtilities
 */
#define DUNE_GENERALIZED_CONSTEXPR
#endif


#endif // DUNE_COMMON_KEYWORDS_HH
