# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

cmake_minimum_required(VERSION 3.16)
project(_project3 VERSION 0.1)

include(DunePolicy)

# non-existent policies should not fail to set a value if set with QUIET
dune_policy(SET _DP000Z NEW QUIET)
