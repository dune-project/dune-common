// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_METIS_HH
#define DUNE_METIS_HH

/**
 * \file This include file is intended to be used instead of the direct include of `metis.h`. It
 * provides the right order of necessary pre-includes, e.g. when scotch-metis is found, and introduces
 * version-/distribution-independent type definitions for floating-point arguments (`real_t`) and
 * integer arguments (`idx_t`) in the namespace `Dune::Metis`.
 **/

#if HAVE_METIS

#if HAVE_SCOTCH_METIS
extern "C" {
  #include <scotch.h>
}
#endif

extern "C" {
  #include <metis.h>
}

#if HAVE_SCOTCH_METIS && !defined(SCOTCH_METIS_RETURN) && !defined(METIS_OK)
  // NOTE: scotchmetis does not define a return type for METIS functions
  #define METIS_OK 1
#endif

namespace Dune::Metis {

#if defined(REALTYPEWIDTH) || defined(SCOTCH_METIS_DATATYPES)
  using real_t = ::real_t;
#else
  using real_t = double;
#endif

#if defined(IDXTYPEWIDTH) || defined(SCOTCH_METIS_DATATYPES)
  using idx_t = ::idx_t;
#elif HAVE_SCOTCH_METIS
  using idx_t = SCOTCH_Num;
#else
  using idx_t = int;
#endif

} // end namespace Dune::Metis

#endif // HAVE_METIS
#endif // DUNE_METIS_HH
