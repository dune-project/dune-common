# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(dune-common-standalone CXX)

# find a build or installed directory with dune-common
find_package(dune-common REQUIRED)

# compile a simple executable with some dune-common utilities
add_executable(dune-common-standalone dune-common-standalone.cc)

# linking only against Dune::Common should be enough
target_link_libraries(dune-common-standalone PRIVATE Dune::Common)
