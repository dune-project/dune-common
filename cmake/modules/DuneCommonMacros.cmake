# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# enforce C++-14
dune_require_cxx_standard(MODULE "dune-common" VERSION 14)

include(DuneStreams)
dune_set_minimal_debug_level()

# search for lapack
find_package(LAPACK)
include(AddBLASLapackFlags)

find_package(GMP)
include(AddGMPFlags)
find_package(QuadMath)
include(AddQuadMathFlags)

# find program for image manipulation
find_package(Inkscape)
include(UseInkscape)

# find the threading library
find_package(Threads)
include(AddThreadsFlags)

# find the MPI library
find_package(MPI 3.0 COMPONENTS C)
include(AddMPIFlags)

# find library for Threading Building Blocks
find_package(TBB)
include(AddTBBFlags)

# find libraries for graph partitioning
find_package(PTScotch)
include(AddPTScotchFlags)
find_package(METIS)
include(AddMETISFlags)
find_package(ParMETIS 4.0)
include(AddParMETISFlags)

# try to find the Vc library
set(MINIMUM_VC_VERSION)
if((CMAKE_CXX_COMPILER_ID STREQUAL Clang) AND
    (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7))
  message(STATUS "Raising minimum acceptable Vc version to 1.4.1 due to use of Clang 7 (or later), see https://gitlab.dune-project.org/core/dune-common/issues/132")
  set(MINIMUM_VC_VERSION 1.4.1)
endif()
find_package(Vc ${MINIMUM_VC_VERSION} NO_MODULE)
include(AddVcFlags)

# Run the python extension of the Dune cmake build system
include(DunePythonCommonMacros)
