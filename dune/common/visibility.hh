#ifndef DUNE_COMMON_VISIBILITY_HH
#define DUNE_COMMON_VISIBILITY_HH

/** \file
 * \brief Definition of macros controlling symbol visibility at the ABI level.
 */

#ifdef DOXYGEN

//! Export a symbol as part of the public ABI.
/**
 * Mark a class, function or static variable as visible outside the current DSO.
 * For now, this is mostly important for templated global variables and functions
 * that contain static variables.
 */
#define DUNE_EXPORT implementation_defined

//! Mark a symbol as being for internal use within the current DSO only.
/**
 * Mark a class, function or static variable as inaccessible from outside the current DSO.
 * Doing so will decrease the size of the symbol table, but you have to be sure that the
 * symbol will never have to be accessed from another library or the main executable!
 */
#define DUNE_PRIVATE implementation_defined

#else // DOXYGEN

#if __GNUC__ >= 4
#define DUNE_EXPORT __attribute__((visibility("default")))
#define DUNE_PRIVATE __attribute__((visibility("hidden")))
#else
#define DUNE_EXPORT
#define DUNE_PRIVATE
#endif

#endif // DOXYGEN

#endif // DUNE_COMMON_VISIBILITY
