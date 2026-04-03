# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindPTScotch
------------

Find `PTScotch <https://gitlab.inria.fr/scotch/scotch>`_, the Scotch software
suite for sequential and parallel graph partitioning and related ordering
algorithms.

Components
^^^^^^^^^^

This module supports the following components:

``SCOTCH``
  Sequential Scotch library.
``PTSCOTCH``
  Parallel version of Scotch. Requires MPI.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PTScotch::Scotch``
  `Imported target <https://cmake.org/cmake/help/latest/command/add_library.html#imported-libraries>`_
  for the sequential Scotch library.
``PTScotch::PTScotch``
  `Imported target <https://cmake.org/cmake/help/latest/command/add_library.html#imported-libraries>`_
  for the parallel PTScotch library.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``PTScotch_FOUND``
  Boolean result indicating whether the requested PTScotch components were
  found.
``PTScotch_SCOTCH_FOUND``
  Boolean result indicating whether the sequential Scotch library was found.
``PTScotch_PTSCOTCH_FOUND``
  Boolean result indicating whether the parallel PTScotch library was found.
``PTScotch_VERSION``
  Version of the detected Scotch installation.

Hints
^^^^^

The following variables may be set to influence this module's behavior:

``PTSCOTCH_SUFFIX``
  Scotch might be compiled using different integer sizes (int32, int64, long).
  When this is set, the headers and libraries are searched under the suffix
  ``include/scotch-${PTSCOTCH_SUFFIX}`` and ``lib/scotch-${PTSCOTCH_SUFFIX}``,
  respectively.

``SCOTCH_INCLUDE_DIR``
  Include directory where the scotch.h is found.

``PTSCOTCH_INCLUDE_DIR``
  Include directory where the ptscotch.h is found.

``SCOTCH_LIBRARY`` and ``SCOTCHERR_LIBRARY``
  Full paths to the sequential Scotch libraries.

``PTSCOTCH_LIBRARY`` and ``PTSCOTCHERR_LIBRARY``
  Full paths to the parallel PTScotch libraries.

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("PTScotch" PROPERTIES
  DESCRIPTION "Sequential and Parallel Graph Partitioning"
  URL "https://gitlab.inria.fr/scotch/scotch"
)

# find dependency for PTScotch
include(CMakeFindDependencyMacro)
find_package(MPI QUIET)

# search directory might have the PATH_SUFFIX scotch-SUFFIX
if(PTSCOTCH_SUFFIX)
  set(PATH_SUFFIXES "scotch-${PTSCOTCH_SUFFIX}")
else()
  set(PATH_SUFFIXES "scotch")
endif()

# Try to find the include files
find_path(SCOTCH_INCLUDE_DIR scotch.h
  PATH_SUFFIXES ${PATH_SUFFIXES})

find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h
  HINTS ${SCOTCH_INCLUDE_DIR}
  PATH_SUFFIXES ${PATH_SUFFIXES})

# Try to find the (pt)scotch libraries
find_library(SCOTCH_LIBRARY scotch)
find_library(SCOTCHERR_LIBRARY scotcherr)
find_library(PTSCOTCH_LIBRARY ptscotch)
find_library(PTSCOTCHERR_LIBRARY ptscotcherr)

mark_as_advanced(SCOTCH_INCLUDE_DIR SCOTCH_LIBRARY SCOTCHERR_LIBRARY
                 PTSCOTCH_INCLUDE_DIR PTSCOTCH_LIBRARY PTSCOTCHERR_LIBRARY)

# check version of (PT)Scotch
find_file(SCOTCH_HEADER "scotch.h"
  HINTS ${SCOTCH_INCLUDE_DIR}
  NO_DEFAULT_PATH)
if(SCOTCH_HEADER)
  file(READ "${SCOTCH_HEADER}" scotchheader)
  string(REGEX REPLACE ".*#define SCOTCH_VERSION[ ]+([0-9]+).*" "\\1"
    SCOTCH_MAJOR_VERSION  "${scotchheader}")
  string(REGEX REPLACE ".*#define SCOTCH_RELEASE[ ]+([0-9]+).*" "\\1"
    SCOTCH_MINOR_VERSION  "${scotchheader}")
  string(REGEX REPLACE ".*#define SCOTCH_PATCHLEVEL[ ]+([0-9]+).*" "\\1"
    SCOTCH_PREFIX_VERSION "${scotchheader}")
  if(SCOTCH_MAJOR_VERSION GREATER_EQUAL 0)
    set(PTScotch_VERSION "${SCOTCH_MAJOR_VERSION}")
  endif()
  if (SCOTCH_MINOR_VERSION GREATER_EQUAL 0)
    set(PTScotch_VERSION "${PTScotch_VERSION}.${SCOTCH_MINOR_VERSION}")
  endif()
  if (SCOTCH_PREFIX_VERSION GREATER_EQUAL 0)
    set(PTScotch_VERSION "${PTScotch_VERSION}.${SCOTCH_PREFIX_VERSION}")
  endif()
endif()
unset(SCOTCH_HEADER CACHE)

# set if (PT)Scotch components found
if (SCOTCH_INCLUDE_DIR AND SCOTCH_LIBRARY AND SCOTCHERR_LIBRARY)
  set(PTScotch_SCOTCH_FOUND TRUE)
endif ()

if (PTSCOTCH_INCLUDE_DIR AND PTSCOTCH_LIBRARY AND PTSCOTCHERR_LIBRARY AND MPI_FOUND)
  set(PTScotch_PTSCOTCH_FOUND TRUE)
endif ()

# dependencies between components
if (NOT PTScotch_SCOTCH_FOUND)
  set(PTScotch_PTSCOTCH_FOUND FALSE)
endif ()

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("PTScotch"
  REQUIRED_VARS
    SCOTCH_LIBRARY SCOTCHERR_LIBRARY SCOTCH_INCLUDE_DIR
  VERSION_VAR
    PTScotch_VERSION
  HANDLE_COMPONENTS
)

if(PTScotch_FOUND)
  # Define an imported target for the sequential Scotch library
  if(PTScotch_SCOTCH_FOUND AND NOT TARGET PTScotch::Scotch)
    add_library(PTScotch::Scotch UNKNOWN IMPORTED)
    set_target_properties(PTScotch::Scotch PROPERTIES
      IMPORTED_LOCATION ${SCOTCH_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${SCOTCH_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES ${SCOTCHERR_LIBRARY}
    )
  endif()

  # Define an imported target for the parallel PTScotch library
  if(PTScotch_SCOTCH_FOUND AND PTScotch_PTSCOTCH_FOUND AND NOT TARGET PTScotch::PTScotch)
    add_library(PTScotch::PTScotch UNKNOWN IMPORTED)
    set_target_properties(PTScotch::PTScotch PROPERTIES
      IMPORTED_LOCATION ${PTSCOTCH_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${PTSCOTCH_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES ${PTSCOTCHERR_LIBRARY}
    )
    target_link_libraries(PTScotch::PTScotch
      INTERFACE PTScotch::Scotch MPI::MPI_C)
  endif()
endif()
