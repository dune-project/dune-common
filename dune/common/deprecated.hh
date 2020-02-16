// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DEPRECATED_HH
#define DUNE_DEPRECATED_HH

//! @addtogroup CxxUtilities
//! @{

/** \file
 * \brief Definition of the DUNE_DEPRECATED macro for the case that config.h
 *      is not available
 * \deprecated Use C++14's [[deprecated]] instead.
 */
#define DUNE_DEPRECATED [[deprecated]]


//! Mark some entity as deprecated
/**
 * \deprecated Use C++14's [[deprecated(msg)]] instead.
 */
#define DUNE_DEPRECATED_MSG(text) [[deprecated(# text)]]


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
