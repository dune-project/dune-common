#ifndef DUNE_COMMON_STD_CONSTEXPR_HH
#define DUNE_COMMON_STD_CONSTEXPR_HH

/**
 * \file
 * \brief Definition of the DUNE_CONSTEXPR macro
 */

#if HAVE_CONSTEXPR || defined(DOXYGEN)

/**
 * \brief Set method or expression constexpr if supported by the compiler.
 * \code
   #include <dune/common/std/constexpr.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to mark expressions or
 * functions as constexpr. C++11 introduced constexpr, C++1y will extend
 * constexpr's use cases.
 */
#define DUNE_CONSTEXPR constexpr

#else // #if HAVE_CONSTEXPR
#define DUNE_CONSTEXPR
#endif // #else // #if HAVE_CONSTEXPR

#warning The header dune/common/std/constexpr.hh is deprecated. Remove the include and use "constexpr".

#endif // #ifndef DUNE_COMMON_STD_CONSTEXPR_HH
