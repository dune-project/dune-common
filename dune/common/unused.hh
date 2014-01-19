// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_UNUSED_HH
#define DUNE_COMMON_UNUSED_HH

/** \file
 * \brief Definition of the DUNE_UNUSED macro for the case that config.h
 *      is not available
 */

#ifndef HAS_ATTRIBUTE_UNUSED
#define DUNE_UNUSED
#else
#define DUNE_UNUSED __attribute__((unused))
#endif

/// A macro to mark intentional unused function parameters with.
#define DUNE_UNUSED_PARAMETER(parm) static_cast<void>(parm)
#endif
