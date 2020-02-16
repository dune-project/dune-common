// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DEPRECATED_HH
#define DUNE_DEPRECATED_HH

/** \file
 * \brief Definition of the DUNE_DEPRECATED macro for the case that config.h
 *      is not available
 */

//! @addtogroup CxxUtilities
//! @{
#if defined(DOXYGEN) || !defined(HAS_ATTRIBUTE_DEPRECATED)
//! Mark some entity as deprecated
/**
 * \deprecated Use C++14's \code[[deprecated]]\endcode instead. It will be
 * removed after Dune 2.8. Be aware that it must be sometimes placed at
 * different position in the code.
 */
#define DUNE_DEPRECATED
#else // defined(HAS_ATTRIBUTE_DEPRECATED)
#define DUNE_DEPRECATED __attribute__((deprecated))
#endif

#if defined(DOXYGEN) || !defined(HAS_ATTRIBUTE_DEPRECATED_MSG)
//! Mark some entity as deprecated
/**
 * \deprecated Use C++14's \code[[deprecated(msg)]]\endcode instead. It
 * will be removed after Dune 2.8. Be aware that it must be sometimes
 * placed at different position in the code.
 */
#define DUNE_DEPRECATED_MSG(text) DUNE_DEPRECATED
#else // defined(HAS_ATTRIBUTE_DEPRECATED_MSG)
#define DUNE_DEPRECATED_MSG(text) __attribute__((deprecated(# text)))
#endif

#ifdef DOXYGEN
/**
 * \brief Ignore deprecation warnings (start)
 *
 * This macro can be used together with `DUNE_NO_DEPRECATED_END` to mark a
 * block in which deprecation warnings are ignored.  This can be useful for
 * implementations of deprecated methods that call other deprecated methods
 * or for testing deprecated methods in the testsuite.
 *
 * \code
     DUNE_NO_DEPRECATED_BEGIN
     some_deprecated_function();
     another_deprecated_function();
     DUNE_NO_DEPRECATED_END
 * \endcode
 *
 * \warning This macro must always be used together with `DUNE_NO_DEPRECATED_END`
 */
#define DUNE_NO_DEPRECATED_BEGIN ...
/**
 * \brief Ignore deprecation warnings (end)
 *
 * \warning This macro must always be used together with `DUNE_NO_DEPRECATED_BEGIN`
 */
#define DUNE_NO_DEPRECATED_END   ...
#else
#  if defined __clang__
#    define DUNE_NO_DEPRECATED_BEGIN \
         _Pragma("clang diagnostic push") \
         _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#    define DUNE_NO_DEPRECATED_END _Pragma("clang diagnostic pop")
#  elif defined __INTEL_COMPILER
#    define DUNE_NO_DEPRECATED_BEGIN \
         _Pragma("warning push") \
         _Pragma("warning(disable:1478)") \
         _Pragma("warning(disable:1786)")
#    define DUNE_NO_DEPRECATED_END _Pragma("warning pop")
#  elif defined __GNUC__
#    define DUNE_NO_DEPRECATED_BEGIN \
         _Pragma("GCC diagnostic push") \
         _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#    define DUNE_NO_DEPRECATED_END _Pragma("GCC diagnostic pop")
#  else
#    define DUNE_NO_DEPRECATED_BEGIN /* Noop. */
#    define DUNE_NO_DEPRECATED_END   /* Noop. */
#  endif
#endif

//! @}

#endif
