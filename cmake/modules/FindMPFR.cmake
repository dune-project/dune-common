# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindMPFR
--------

Find the GNU MPFR library and the corresponding C++ bindings MPFR-C++

This module searches for both libraries and only considers the package
found if both can be located. It then defines separate targets for the C
and the C++ library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MPFR::mpfr``
  Library target of the C library.
``MPFR::mpreal``
  Library target of the C++ library, which also links to the C library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MPFR_FOUND``
  True if the MPFR library, the MPFR++ headers and
  the MPFR++ library were found.

Cache Variables
^^^^^^^^^^^^^^^

You may set the following variables to modify the behaviour of
this module:

``MPFR_ROOT``
  The directory containing ``include/mpfr.h``.
``MPREAL_ROOT``
  The directory containing ``include/mpreal.h``.

#]=======================================================================]

# Add a feature summary for this package
include(FeatureSummary)
set_package_properties(MPFR PROPERTIES
  DESCRIPTION "GNU MPFR multi-precision library including the C++ bindings MPFR C++ (mpreal)"
  URL "https://www.mpfr.org/"
)

# Try finding the package with pkg-config
find_package(PkgConfig QUIET)
pkg_check_modules(PkgConfigMPFR mpfr IMPORTED_TARGET GLOBAL)

# check whether the static library was found
if(PkgConfigMPFR_STATIC_FOUND)
  set(_mpfr PkgConfigMPFR_STATIC)
else()
  set(_mpfr PkgConfigMPFR)
endif()

# search for location of header mpfr.h, only at positions given by the user
find_path(MPFR_INCLUDE_DIR
  NAMES "mpfr.h"
  HINTS ${${_mpfr}_INCLUDEDIR}
  PATHS ${MPFR_ROOT}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
# try default paths now
find_path(MPFR_INCLUDE_DIR
  NAMES "mpfr.h")

# search for location of header mpreal.h, only at positions given by the user
find_path(MPREAL_INCLUDE_DIR
  NAMES "mpreal.h"
  PATHS ${MPREAL_ROOT}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
# try default paths now
find_path(MPREAL_INCLUDE_DIR
  NAMES "mpreal.h")

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("MPFR"
  REQUIRED_VARS
    ${_mpfr}_LINK_LIBRARIES ${_mpfr}_FOUND MPFR_INCLUDE_DIR MPREAL_INCLUDE_DIR PkgConfig_FOUND
  VERSION_VAR
    ${_mpfr}_VERSION
  FAIL_MESSAGE "Could NOT find MPFR (set MPFR_ROOT or MPREAL_ROOT to path containing mpfr.h or mpreal.h, respectively)"
)

mark_as_advanced(MPFR_INCLUDE_DIR MPREAL_INCLUDE_DIR)

# C library
if(${_mpfr}_FOUND AND NOT TARGET MPFR::mpfr)
  add_library(MPFR::mpfr ALIAS PkgConfig::PkgConfigMPFR)
endif()

# C++ library, which requires a link to the C library
if(MPREAL_INCLUDE_DIR AND NOT TARGET MPFR::mpreal)
  add_library(MPFR::mpreal INTERFACE IMPORTED)
  set_target_properties(MPFR::mpreal PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${MPREAL_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES MPFR::mpfr
  )
endif()
