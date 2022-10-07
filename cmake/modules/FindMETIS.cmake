# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindMETIS
---------

Find Serial Graph Partitioning library
`METIS <http://glaros.dtc.umn.edu/gkhome/metis/metis/overview>`_

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following `IMPORTED`_ target:

``METIS::METIS``
  The libraries, flags, and includes to use for METIS, if found.

.. _IMPORTED: https://cmake.org/cmake/help/latest/command/add_library.html#id5

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``METIS_FOUND``
  The METIS library with all its dependencies is found

Cache Variables
^^^^^^^^^^^^^^^

The following variables may be set to influence this module's behavior:

``METIS_INCLUDE_DIR``
  Include directory of METIS

``METIS_LIBRARY``
  Full path to the METIS library

``METIS_API_VERSION``
  This variable specifies the METIS API version provided by the scotch-metis library. This
  is required for Scotch >= 6.0.7 versions if it is not detected automatically. The
  variable may be set to 3 to indicate that scotch implements the METIS API v3 (default
  for older Scotch versions), or it can be set to 5 to indicate that v5 of the METIS API
  is provided. This variable corresponds to the preprocessor flag `SCOTCH_METIS_VERSION`
  that is used when compiling Scotch from source.

#]=======================================================================]

# Text for feature summary
include(FeatureSummary)
set_package_properties("METIS" PROPERTIES
  DESCRIPTION "Serial Graph Partitioning"
  URL "http://glaros.dtc.umn.edu/gkhome/metis/metis/overview"
)

# The METIS API version provided by the METIS or scotch-metis library
set(METIS_API_VERSION 0 CACHE STRING
  "METIS API version provided by METIS or scotch-metis library")

# Try to locate METIS header
find_path(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis)

# Determine version of METIS installation
find_file(METIS_HEADER_FILE metis.h
  PATHS ${METIS_INCLUDE_DIR}
  NO_DEFAULT_PATH)
if(METIS_HEADER_FILE)
  file(READ "${METIS_HEADER_FILE}" metisheader)
  string(REGEX REPLACE ".*#define METIS_VER_MAJOR[ ]+([0-9]+).*" "\\1"
    METIS_MAJOR_VERSION "${metisheader}")
  string(REGEX REPLACE ".*#define METIS_VER_MINOR[ ]+([0-9]+).*" "\\1"
    METIS_MINOR_VERSION "${metisheader}")
  if(METIS_MAJOR_VERSION GREATER_EQUAL 0 AND METIS_MINOR_VERSION GREATER_EQUAL 0)
    set(METIS_VERSION "${METIS_MAJOR_VERSION}.${METIS_MINOR_VERSION}")

    # Specify an api version to be used in config.h files or compile flags
    if(NOT METIS_API_VERSION)
      if(METIS_MAJOR_VERSION GREATER_EQUAL 3 AND METIS_MAJOR_VERSION LESS 5)
        set(METIS_API_VERSION "3")
      else()
        set(METIS_API_VERSION "${METIS_MAJOR_VERSION}")
      endif()
    endif()
  else()
    unset(METIS_MAJOR_VERSION)
    unset(METIS_MINOR_VERSION)
  endif()

  # test whether header file is actually the scotch-metis header
  string(FIND "${metisheader}" "SCOTCH_METIS_PREFIX" IS_SCOTCH_METIS_HEADER)
  if(IS_SCOTCH_METIS_HEADER EQUAL "-1")
    set(IS_SCOTCH_METIS_HEADER FALSE)
  else()
    set(IS_SCOTCH_METIS_HEADER TRUE)
    string(REGEX REPLACE ".*#define SCOTCH_METIS_VERSION[ ]+([0-9]+).*" "\\1"
      SCOTCH_METIS_VERSION "${metisheader}")
  endif()
endif()
unset(METIS_HEADER_FILE CACHE)

# search for the METIS library or for the scotch-metis wrapper library
if(IS_SCOTCH_METIS_HEADER)
  # Debian bookworm has no scotchmetis library only scotchmetisv3
  # and scotchmetisv5.
  if(SCOTCH_METIS_VERSION)
    find_library(METIS_LIBRARY scotchmetisv${SCOTCH_METIS_VERSION})
  endif()
  find_library(METIS_LIBRARY scotchmetis)
else()
  find_library(METIS_LIBRARY metis)
endif()

# We need to check whether we need to link m, copy the lazy solution
# from FindBLAS and FindLAPACK here.
if(METIS_LIBRARY AND NOT WIN32)
  set(METIS_NEEDS_LIBM 1)
endif()

mark_as_advanced(METIS_INCLUDE_DIR METIS_LIBRARY METIS_NEEDS_LIBM METIS_API_VERSION)

# If scotch is requested, find package PTScotch and check version compatibility:
# Scotch provides METIS-3 interface only in version < 6.07, but provides an option to
# select the API-version in later Scotch releases
if(IS_SCOTCH_METIS_HEADER)
  find_package(PTScotch)
  set(HAVE_SCOTCH_METIS ${PTScotch_SCOTCH_FOUND})
  if (PTScotch_SCOTCH_FOUND AND NOT METIS_API_VERSION)
    if(PTScotch_VERSION VERSION_LESS "6.0.7")
      set(METIS_API_VERSION "3")
    else()
      # try to figure out the METIS_API_VERSION by checking for symbols in the library
      include(CheckSymbolExists)
      include(CMakePushCheckState)
      find_package(Threads)
      cmake_push_check_state()
      set(CMAKE_REQUIRED_LIBRARIES ${METIS_LIBRARY} ${SCOTCH_LIBRARY} ${SCOTCHERR_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
      if(METIS_NEEDS_LIBM)
        list(APPEND CMAKE_REQUIRED_LIBRARIES m)
      endif()
      set(CMAKE_REQUIRED_INCLUDES ${METIS_INCLUDE_DIR} ${SCOTCH_INCLUDE_DIR})

      set(CMAKE_REQUIRED_DEFINITIONS "-DSCOTCH_METIS_VERSION=3")
      check_symbol_exists("METIS_PartGraphVKway" "stdio.h;stdint.h;scotch.h;metis.h" IS_SCOTCH_METIS_API_V3)
      if(IS_SCOTCH_METIS_API_V3)
        set(METIS_API_VERSION "3")
      else()
        set(CMAKE_REQUIRED_DEFINITIONS "-DSCOTCH_METIS_VERSION=5")
        check_symbol_exists("METIS_PartGraphKway" "stdio.h;stdint.h;scotch.h;metis.h" IS_SCOTCH_METIS_API_V5)
        if(IS_SCOTCH_METIS_API_V5)
          set(METIS_API_VERSION "5")
        endif()
      endif()
      cmake_pop_check_state()
    endif()
  endif()
endif()

# Behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("METIS"
  REQUIRED_VARS
    METIS_LIBRARY METIS_INCLUDE_DIR METIS_API_VERSION
  VERSION_VAR
    METIS_VERSION
)

# If both headers and library are found, create imported target
if(METIS_FOUND AND NOT TARGET METIS::METIS)
  add_library(METIS::METIS UNKNOWN IMPORTED)
  set_target_properties(METIS::METIS PROPERTIES
    IMPORTED_LOCATION ${METIS_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${METIS_INCLUDE_DIR}
    INTERFACE_COMPILE_DEFINITIONS METIS_API_VERSION=${METIS_API_VERSION}
  )

  # Link against libm if needed
  if(METIS_NEEDS_LIBM)
    set_property(TARGET METIS::METIS APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES m)
  endif()

  # Link against Scotch library if option is enabled
  if(IS_SCOTCH_METIS_HEADER AND PTScotch_FOUND)
    set_property(TARGET METIS::METIS APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES PTScotch::Scotch)
    set_property(TARGET METIS::METIS APPEND PROPERTY
      INTERFACE_COMPILE_DEFINITIONS
        SCOTCH_METIS_VERSION=${METIS_API_VERSION})
  endif()
endif()
