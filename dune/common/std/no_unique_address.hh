// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_NO_UNIQUE_ADDRESS_HH
#define DUNE_COMMON_STD_NO_UNIQUE_ADDRESS_HH

#if __has_include(<version>)
#include <version>
#endif

// Provide the  macro DUNE_NO_UNIQUE_ADDRESS that expands to [[no_unique_address]]
// or similar depending on the compiler (version)

#if _MSC_VER
  #if _MSC_VER >= 1929 // VS2019 v16.10 and later
    #define DUNE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
  #else // no-op in MSVC v14x ABI
    #define DUNE_NO_UNIQUE_ADDRESS /* [[no_unique_address]] */
  #endif
#elif __has_cpp_attribute(no_unique_address) >= 201803L
  #define DUNE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
  #define DUNE_NO_UNIQUE_ADDRESS /* [[no_unique_address]] */
#endif

#endif // DUNE_COMMON_STD_NO_UNIQUE_ADDRESS_HH
