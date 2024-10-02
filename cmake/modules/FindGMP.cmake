# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindGMP
-------

Find the GNU MULTI-Precision Bignum (GMP) library
and the corresponding C++ bindings GMPxx.

This module searches for both libraries and only considers the package
found if both can be located. It then defines separate targets for the C
and the C++ library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GMP::gmp``
  Library target of the C library.
``GMP::gmpxx``
  Library target of the C++ library, which also links to the C library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GMP_FOUND``
  True if the GMP library, the GMPxx headers and
  the GMPxx library were found.

Cache Variables
^^^^^^^^^^^^^^^

You may set the following variables to modify the behaviour of
this module:

``GMP_INCLUDE_DIR``
  The directory containing ``gmp.h``.
``GMP_LIB``
  The path to the gmp library.
``GMPXX_INCLUDE_DIR``
  The directory containing ``gmpxx.h``.
``GMPXX_LIB``
  The path to the gmpxx library.

#]=======================================================================]

# Add a feature summary for this package
include(FeatureSummary)
set_package_properties(GMP PROPERTIES
  DESCRIPTION "GNU multi-precision library"
  URL "https://gmplib.org"
)

# Try finding the package with pkg-config
find_package(PkgConfig QUIET)
pkg_check_modules(PKG QUIET gmp gmpxx)

# Try to locate the libraries and their headers, using pkg-config hints
find_path(GMP_INCLUDE_DIR gmp.h HINTS ${PKG_gmp_INCLUDEDIR})
find_library(GMP_LIB gmp HINTS ${PKG_gmp_LIBDIR})

find_path(GMPXX_INCLUDE_DIR gmpxx.h HINTS ${PKG_gmpxx_INCLUDEDIR})
find_library(GMPXX_LIB gmpxx HINTS ${PKG_gmpxx_LIBDIR})

if (GMP_INCLUDE_DIR AND GMP_LIB AND GMPXX_INCLUDE_DIR AND GMPXX_LIB)
  # Check whether gmp and gmpxx can be used
  file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src_find_gmp.cxx"
  "#include <gmpxx.h>\n#include <iostream>\nint main() { mpz_class x = 42; std::cout << x; }\n")

  try_compile(GMPXX_WORKS ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src_find_gmp.cxx
    CMAKE_FLAGS
      -DINCLUDE_DIRECTORIES='${GMP_INCLUDE_DIR}\;${GMPXX_INCLUDE_DIR}'
      -DLINK_LIBRARIES='${GMP_LIB}\;${GMPXX_LIB}'
    OUTPUT_VARIABLE OUTPUT)
  if(NOT GMPXX_WORKS)
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Performing C++ SOURCE FILE Test GMPXX_WORKS failed with the following output:\n"
      "${OUTPUT}\n")
  endif()
endif()

# Remove these variables from cache inspector
mark_as_advanced(GMP_INCLUDE_DIR GMP_LIB GMPXX_INCLUDE_DIR GMPXX_LIB GMPXX_WORKS)

# Report if package was found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
  DEFAULT_MSG
  GMPXX_LIB GMPXX_INCLUDE_DIR GMP_INCLUDE_DIR GMP_LIB GMPXX_WORKS
)

# Set targets
if(GMP_FOUND)
  # C library
  if(NOT TARGET GMP::gmp)
    add_library(GMP::gmp UNKNOWN IMPORTED)
    set_target_properties(GMP::gmp PROPERTIES
      IMPORTED_LOCATION ${GMP_LIB}
      INTERFACE_INCLUDE_DIRECTORIES ${GMP_INCLUDE_DIR}
    )
  endif()

  # C++ library, which requires a link to the C library
  if(NOT TARGET GMP::gmpxx)
    add_library(GMP::gmpxx UNKNOWN IMPORTED)
    set_target_properties(GMP::gmpxx PROPERTIES
      IMPORTED_LOCATION ${GMPXX_LIB}
      INTERFACE_INCLUDE_DIRECTORIES ${GMPXX_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES GMP::gmp
    )
  endif()
endif()
