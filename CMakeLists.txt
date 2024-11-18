# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

cmake_minimum_required(VERSION 3.16)
project(dune-common LANGUAGES C CXX)

# CMake 3.29.1 is incompatible as it removed PACKAGE_PREFIX_DIR
if (CMAKE_VERSION VERSION_EQUAL 3.29.1)
  message(FATAL_ERROR "CMake 3.29.1 is not compatible with Dune. Use a different CMake version.")
endif()

# make sure our own modules are found
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

set(THREADS_PREFER_PTHREAD_FLAG TRUE CACHE BOOL "Prefer -pthread compiler and linker flag")

#include the dune macros
include(DuneMacros)

# deactivate global include-directories for dune-common
dune_policy(SET DP_DEFAULT_INCLUDE_DIRS NEW)

# start a dune project with information from dune.module
dune_project()

# Create a target for dune-common with a Dune::Common alias
dune_add_library(dunecommon EXPORT_NAME Common NAMESPACE Dune::)

# set include directories for dunecommon library
dune_default_include_directories(dunecommon PUBLIC)

# minimal c++ standard required
target_compile_features(dunecommon PUBLIC cxx_std_20)

# Set properties to the dunecommon target
add_dune_blas_lapack_flags(dunecommon)
add_dune_tbb_flags(dunecommon)

# collect dependencies to be added into the dune-common-config.cmake files
set(DUNE_COMMON_PACKAGE_DEPENDENCIES
  [[set(THREADS_PREFER_PTHREAD_FLAG TRUE CACHE BOOL "Prefer -pthread compiler and linker flag")]])

# since dunecommon is exported its linked libs must be provided downstream too
if (LAPACK_FOUND)
  list(APPEND DUNE_COMMON_PACKAGE_DEPENDENCIES "find_dependency(LAPACK)")
elseif (BLAS_FOUND)
  list(APPEND DUNE_COMMON_PACKAGE_DEPENDENCIES "find_dependency(BLAS)")
endif()
if (Threads_FOUND)
  list(APPEND DUNE_COMMON_PACKAGE_DEPENDENCIES "find_dependency(Threads)")
endif()
if (TBB_FOUND)
  list(APPEND DUNE_COMMON_PACKAGE_DEPENDENCIES "find_dependency(TBB)")
endif()

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

# write contents into DUNE_CUSTOM_PKG_CONFIG_SECTION, which will be injected into dune-common-config.cmake
string(JOIN "\n" DUNE_CUSTOM_PKG_CONFIG_SECTION
  # make sure that Find<module>.cmake provided by dune-common can be found by cmake
  [[list(APPEND CMAKE_MODULE_PATH "${dune-common_MODULE_PATH}")]]
  ${DUNE_COMMON_PACKAGE_DEPENDENCIES}
)

# finalize the dune project, e.g. generating config.h, dune-common-config.cmake, etc.
finalize_dune_project()
