// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_BOUNDSCHECKING_HH
#define DUNE_BOUNDSCHECKING_HH

#include <dune/common/exceptions.hh>

/**
 * \file
 * \brief Macro for wrapping boundary checks
 */

/**
 * @addtogroup Common
 *
 * @{
 */

#ifndef DUNE_ASSERT_BOUNDS
#if defined(DUNE_CHECK_BOUNDS) || defined(DOXYGEN)

/**
 * \brief If `DUNE_CHECK_BOUNDS` is defined: check if condition
 * \a cond holds; otherwise, do nothing.
 *
 * Meant to be used for conditions that assure writes and reads
 * do not occur outside of memory limits or pre-defined patterns
 * and related conditions.
 */
#define DUNE_ASSERT_BOUNDS(cond)                            \
  do {                                                      \
    if (!(cond))                                            \
      DUNE_THROW(Dune::RangeError, "Index out of bounds."); \
  } while (false)

#else
#define DUNE_ASSERT_BOUNDS(cond)
#endif
#endif

/* @} */

#endif // DUNE_BOUNDSCHECKING_HH
