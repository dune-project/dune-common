#[=======================================================================[.rst:
FindMETIS
---------

Find Serial Graph Partitioning library METIS
(see http://glaros.dtc.umn.edu/gkhome/metis/metis/overview)

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``METIS::METIS``
  The libraries, flags, and includes to use for METIS, if found.

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

``ENABLE_SCOTCH_METIS``
  Use the Scotch library as METIS compatibility library. This library provides an
  interface of some METIS library functions.

``SCOTCH_METIS_VERSION``
  If `ENABLE_SCOTCH_METIS` is set, this variable specifies the METIS API version provided
  by the scotch-metis library. This is required for Scotch >= 6.0.7 versions, since it
  cannot be detected by inspecting provided files. The variable may be set to 3 to indicate
  that scotch implements the METIS API v3 (default for older Scotch versions), or it can
  be set to 5 to indicate that v5 of the METIS API is provided. This variable corresponds
  to the preprocessor flag that is used when compiling Scotch from source.
#]=======================================================================]

# Text for feature summary
include(FeatureSummary)
set_package_properties("METIS" PROPERTIES
  DESCRIPTION "Serial Graph Partitioning"
)

# The Scotch library provides a wrapper around some functions of METIS. Since is does
# not provide the full interface, you have to request it explicitly.
option(ENABLE_SCOTCH_METIS "Use the Scotch library as METIS compatibility library" FALSE)
set(SCOTCH_METIS_VERSION 0 CACHE STRING
  "METIS API version provided by scotch-metis library")

# Try to locate METIS header
find_path(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis)

find_library(METIS_LIBRARY metis)
if(ENABLE_SCOTCH_METIS)
  find_library(METIS_LIBRARY scotchmetis)
endif()

# We need to check whether we need to link m, copy the lazy solution
# from FindBLAS and FindLAPACK here.
if(METIS_LIBRARY AND NOT WIN32)
  set(METIS_NEEDS_LIBM 1)
endif()

mark_as_advanced(METIS_INCLUDE_DIR METIS_LIBRARY METIS_NEEDS_LIBM)

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
  else()
    unset(METIS_MAJOR_VERSION)
    unset(METIS_MINOR_VERSION)
  endif()
endif()
unset(METIS_HEADER_FILE CACHE)

# If scotch is requested, find package PTScotch and check version compatibility:
# Scotch provides METIS-3 interface only in version < 6.07, but provides an option to
# select the API-version in later Scotch releases
if(ENABLE_SCOTCH_METIS)
  include(CMakeFindDependencyMacro)
  find_package(PTScotch QUIET COMPONENTS SCOTCH)
  set(HAVE_SCOTCH_METIS ${PTScotch_FOUND})
  if (PTScotch_FOUND)
    if(PTScotch_VERSION VERSION_LESS "6.0.7")
      set(METIS_MAJOR_VERSION "3")
    elseif(SCOTCH_METIS_VERSION)
      set(METIS_MAJOR_VERSION "${SCOTCH_METIS_VERSION}")
    else()
      message(WARNING "Cannot detect METIS API version provided by the scotch-metis library.
        Set the cmake variable SCOTCH_METIS_VERSION to the corresponding version number.")
    endif()
    if(METIS_MAJOR_VERSION GREATER_EQUAL 0)
      set(METIS_VERSION "${METIS_MAJOR_VERSION}.0")
    endif()
  endif()
endif()

# Specify an api version to be used in config.h files or compile flags
set(METIS_API_VERSION "${METIS_MAJOR_VERSION}")

# Behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("METIS"
  REQUIRED_VARS
    METIS_LIBRARY METIS_INCLUDE_DIR METIS_VERSION
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
    target_link_libraries(METIS::METIS INTERFACE m)
  endif()

  # Link against Scotch library if option is enabled
  if(ENABLE_SCOTCH_METIS AND PTScotch_FOUND)
    target_link_libraries(METIS::METIS INTERFACE PTScotch::Scotch)
    target_compile_definitions(METIS::METIS INTERFACE
      HAVE_SCOTCH_METIS
      SCOTCH_METIS_VERSION=${SCOTCH_METIS_VERSION})
  endif()
endif()
