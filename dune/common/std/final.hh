// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_FINAL_HH
#define DUNE_COMMON_STD_FINAL_HH

/** \file
 * \brief Definition of the DUNE_FINAL macro
 */

#if HAVE_KEYWORD_FINAL || defined(DOXYGEN)

/**
 * \brief Set method or expression <code>final</code> if
 * supported by the compiler.
 *
 * \code
   #include <dune/common/std/final.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to
 * member functions or classes as <code>final</code>,
 * this keyword was introduced by C++11.
 */
#define DUNE_FINAL final

#else
#define DUNE_FINAL
#endif

#endif   // DUNE_COMMON_STD_FINAL_HH
