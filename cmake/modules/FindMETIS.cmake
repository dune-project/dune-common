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
  Use the Scotch library as METIS compatibility library. This library provides a METIS-4
  interface of some METIS library functions.

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("METIS" PROPERTIES
  DESCRIPTION "Serial Graph Partitioning"
)

option(ENABLE_SCOTCH_METIS "Use the Scotch library as METIS compatibility library" FALSE)

# Try to locate METIS header
find_path(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis include include/metis Lib METISLib
  HINTS ${METIS_DIR})

# Set a name of the METIS library. This is typically `metis` or `scotchmetis`
set(METIS_LIB_NAME metis
    CACHE STRING "Name of the METIS library (default: metis).")

find_library(METIS_LIBRARY metis
  PATH_SUFFIXES lib
  HINTS ${METIS_DIR})

# we need to check whether we need to link m, copy the lazy solution
# from FindBLAS and FindLAPACK here.
if(METIS_LIBRARY AND NOT WIN32)
  set(METIS_NEEDS_LIBM 1)
endif()

mark_as_advanced(METIS_INCLUDE_DIR METIS_LIBRARY METIS_LIB_NAME METIS_NEEDS_LIBM)

# determine version of METIS installation
find_file(METIS_HEADER_FILE metis.h
  PATHS ${METIS_INCLUDE_DIR}
  NO_DEFAULT_PATH)
if(METIS_HEADER_FILE)
  file(READ "${METIS_HEADER_FILE}" metisheader)
  string(REGEX REPLACE ".*#define METIS_VER_MAJOR[ ]+([0-9]+).*" "\\1" METIS_MAJOR_VERSION "${metisheader}")
  string(REGEX REPLACE ".*#define METIS_VER_MINOR[ ]+([0-9]+).*" "\\1" METIS_MINOR_VERSION "${metisheader}")
  if(METIS_MAJOR_VERSION GREATER_EQUAL 0 AND METIS_MINOR_VERSION GREATER_EQUAL 0)
    set(METIS_VERSION "${METIS_MAJOR_VERSION}.${METIS_MINOR_VERSION}")
  endif()
endif()
unset(METIS_HEADER_FILE CACHE)

# If scotch is requested, find package PTScotch and check version compatibility:
# scotch provides METIS-4 interface only
set(METIS_DEPENDENCIES_FOUND TRUE)
if(ENABLE_SCOTCH_METIS)
  include(CMakeFindDependencyMacro)
  find_dependency(PTScotch)
  if(NOT PTScotch_FOUND OR (PACKAGE_FIND_VERSION_MAJOR
                            AND NOT PACKAGE_FIND_VERSION_MAJOR EQUAL "4"))
    set(METIS_DEPENDENCIES_FOUND FALSE)
  endif()
endif()
mark_as_advanced(METIS_DEPENDENCIES_FOUND)

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("METIS"
  REQUIRED_VARS
    METIS_LIBRARY METIS_INCLUDE_DIR METIS_DEPENDENCIES_FOUND
  VERSION_VAR
    METIS_VERSION
)

# if both headers and library are found, create imported target
if(METIS_FOUND AND NOT TARGET METIS::METIS)
  add_library(METIS::METIS UNKNOWN IMPORTED)
  set_target_properties(METIS::METIS PROPERTIES
    IMPORTED_LOCATION ${METIS_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${METIS_INCLUDE_DIR}
  )

  # link against libm amd scotch if needed
  if (METIS_NEEDS_LIBM)
    target_link_libraries(METIS::METIS INTERFACE m)
  endif ()

  if (ENABLE_SCOTCH_METIS AND PTScotch_FOUND)
    target_link_libraries(METIS::METIS INTERFACE PTScotch::Scotch)
  endif ()
endif()
