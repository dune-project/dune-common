// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_UNUSED_HH
#define DUNE_COMMON_UNUSED_HH

/** \file
 * \brief Definition of the `DUNE_UNUSED_PARAMETER` macro.
 */

/**
 * A macro to mark intentionally unused function parameters with.
 *
 * If possible use C++17's \code[[maybe_unused]]\endcode instead.
 * Due to a bug prior to GCC 9.3 it cannot be used for the first
 * argument of a constructor (bug 81429).
 *
 * \ingroup CxxUtilities
 */
#define DUNE_UNUSED_PARAMETER(param) static_cast<void>(param)
#endif
