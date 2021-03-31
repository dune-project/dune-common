// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_UNUSED_HH
#define DUNE_COMMON_UNUSED_HH

/** \file
 * \brief Definition of the DUNE_UNUSED macro for the case that config.h
 *      is not available
 */

#ifndef HAS_ATTRIBUTE_UNUSED
//! A macro for marking variables that the compiler mistakenly flags as unused, which sometimes happens due to templates.
/**
 * \ingroup CxxUtilities
 * \deprecated Use C++17's \code[[maybe_unused]]\endcode instead. This
 * macro will be removed after Dune 2.8. Be aware that it must be
 * sometimes placed at a different position in the code.
 */
#define DUNE_UNUSED
#else
#ifdef __GNUC__
#  define DUNE_UNUSED _Pragma("GCC warning \"DUNE_UNUSED is deprecated\"") __attribute__((unused))
#else
#  define DUNE_UNUSED _Pragma("message \"DUNE_UNUSED is deprecated\"") __attribute__((unused))
#endif
#endif

/**
 * A macro to mark intentionally unused function parameters with.
 *
 * If possible use C++17's \code[[maybe_unused]]\endcode instead.
 * Due to a bug prior to GCC 9.3 it cannot be used for the first
 * argument of a constructor (bug 81429).
 *
 * \ingroup CxxUtilities
 */
#define DUNE_UNUSED_PARAMETER(parm) static_cast<void>(parm)
#endif
