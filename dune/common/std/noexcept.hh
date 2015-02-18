#ifndef DUNE_COMMON_STD_NOEXCEPT_HH
#define DUNE_COMMON_STD_NOEXCEPT_HH

/**
 * \file
 * \brief Definition of the DUNE_NOEXCEPT macro
 */

#if HAVE_NOEXCEPT_SPECIFIER || defined(DOXYGEN)

/**
 * \brief Set method to noexcept if supported by the compiler.
 * \code
   #include <dune/common/std/noexcept.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to mark methods as
 * noexcept.
 *
 * \note This does not support the noexcept operator.
 */
#define DUNE_NOEXCEPT noexcept

#else
#define DUNE_NOEXCEPT
#endif // HAVE_NOEXCEPT_SPECIFIER || defined(DOXYGEN)

#endif // DUNE_COMMON_STD_NOEXCEPT_HH
