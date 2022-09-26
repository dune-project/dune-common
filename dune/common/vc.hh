// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_VC_HH
#define DUNE_COMMON_VC_HH

/**
   \file

   \brief Compatibility header for including <Vc/Vc>

   Certain versions (1.3.2) of Vc (https://github.com/VcDevel/Vc) have a
   problem with certain compiler versions (g++ 7.2.0) in c++17 mode, see #88.
 */

#if HAVE_VC

// include Vc's macro definitions
#include <Vc/global.h>

// undefine the macro that signals C++17 support, if set
#ifdef Vc_CXX17
#undef Vc_CXX17
#endif

// include the rest of Vc
#include <Vc/Vc>

#endif // HAVE_VC

#endif // DUNE_COMMON_VC_HH
