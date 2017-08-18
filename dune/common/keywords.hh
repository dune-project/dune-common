#ifndef DUNE_COMMON_KEYWORDS_HH
#define DUNE_COMMON_KEYWORDS_HH

/** \file
 * \brief Definitions of several macros that conditionally make C++ syntax
 *      available.
 *
 * This header contains several macros that enable C++ features depending on your
 * compiler. Most of these features are optional and provide additional functionality
 * like making code constexpr.
 */


#if DUNE_HAVE_CXX_INLINE_VARIABLES
#define DUNE_INLINE_VARIABLE inline
#else
//! Preprocessor macro used for marking variables inline on supported compilers.
#define DUNE_INLINE_VARIABLE
#endif


#if DUNE_HAVE_CXX_GENERALIZED_CONSTEXPR
#define DUNE_GENERALIZED_CONSTEXPR constexpr
#else
//! Preprocessor macro used for marking code as constexpr under the relaxed rules of C++14 if supported by the compiler.
#define DUNE_GENERALIZED_CONSTEXPR
#endif


#endif // DUNE_COMMON_KEYWORDS_HH
