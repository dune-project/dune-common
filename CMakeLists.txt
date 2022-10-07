# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

cmake_minimum_required(VERSION 3.13)
project(dune-common LANGUAGES C CXX)

# make sure our own modules are found
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

#include the dune macros
include(DuneMacros)

# start a dune project with information from dune.module
dune_project()

# Create the module library for dune-common
dune_add_library(dunecommon)

# Set properties to the dunecommon target
add_dune_blas_lapack_flags(dunecommon)
add_dune_tbb_flags(dunecommon)

# add subdirectories to execute CMakeLists.txt there
add_subdirectory(bin)
add_subdirectory(cmake)
add_subdirectory(doc)
add_subdirectory(dune)
add_subdirectory(lib)
add_subdirectory(share)

# if Python bindings are enabled, include necessary sub directories.
if(DUNE_ENABLE_PYTHONBINDINGS)
  add_subdirectory(python)
endif()

# finalize the dune project, e.g. generating config.h etc.
finalize_dune_project()
