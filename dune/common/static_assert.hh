// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STATIC_ASSERT_HH
#define DUNE_STATIC_ASSERT_HH

#warning This header and the macro dune_static_assert are deprecated, use static_assert instead.

/** \file
 * \brief Fallback implementation of the C++0x static_assert feature
 */

/**
 * @addtogroup Common
 *
 * @{
 */

/**
    \brief Helper template so that compilation fails if condition is not true.
    \deprecated Use static_assert from C++11 instead.
 */

#define dune_static_assert(COND,MSG) \
  static_assert(COND,MSG)

#include "typetraits.hh"

/* @} */

#endif
