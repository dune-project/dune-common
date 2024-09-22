# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

cmake_minimum_required(VERSION 3.16)
project(dune_policy_test VERSION 0.1)

include(DunePolicy)
dune_define_policy(_DP000X dune_policy_test 1.0 "The is a test policy")

add_subdirectory(project1)
add_subdirectory(project2)