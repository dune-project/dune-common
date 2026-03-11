# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

cmake_minimum_required(VERSION 3.16)
project(_project2 VERSION 0.1)

include(DunePolicy)

set(DUNE_POLICY_DISABLE_WARNING ON)
dune_get_policy(_DP000X _policy)
if(_policy STREQUAL NEW)
  message(FATAL_ERROR "Dune policy _DP000X should not be set to NEW")
endif()