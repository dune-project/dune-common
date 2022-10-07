// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

// DUNE_MODTEST_VERSION 3.2.1
#define DUNE_MODTEST_VERSION_MAJOR 3
#define DUNE_MODTEST_VERSION_MINOR 2
#define DUNE_MODTEST_VERSION_REVISION 1

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <dune/common/version.hh>

// test 3.3 >= 3.2
#if DUNE_VERSION_GTE(DUNE_MODTEST, 3, 3)
#error "3.2 >= 3.3"
#endif

// test 3.2 >= 3.1
#if DUNE_VERSION_LTE(DUNE_MODTEST, 3, 1)
#error "3.2 <= 3.1"
#endif

// test 3.2 == 3.2
#if DUNE_VERSION_GT(DUNE_MODTEST, 3, 2)
  #error "3.2 > 3.2"
#elif DUNE_VERSION_LT(DUNE_MODTEST, 3, 2)
   #error "3.2 < 3.2"
#else
  #if ! DUNE_VERSION_EQUAL(DUNE_MODTEST, 3, 2)
    #error "3.2 != 3.2"
  #endif
#endif

// test 3.2.2 >= 3.2.1
#if DUNE_VERSION_GTE_REV(DUNE_MODTEST, 3, 2, 2)
#error "3.2.1 >= 3.2.2"
#endif

// test 3.2.1 >= 3.2.0
#if DUNE_VERSION_LTE_REV(DUNE_MODTEST, 3, 2, 0)
#error "3.2.1 <= 3.2.0"
#endif

// test 3.2.1 == 3.2.1
#if DUNE_VERSION_GT_REV(DUNE_MODTEST, 3, 2, 1)
  #error "3.2.1 > 3.2.1"
#elif DUNE_VERSION_LT_REV(DUNE_MODTEST, 3, 2, 1)
   #error "3.2.1 < 3.2.1"
#else
  #if ! DUNE_VERSION_EQUAL_REV(DUNE_MODTEST, 3, 2, 1)
    #error "3.2.1 != 3.2.1"
  #endif
#endif

int main()
{
  return 0;
}
