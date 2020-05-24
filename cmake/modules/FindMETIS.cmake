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

``METIS_LIB_NAME``
  Name of the METIS library (default: metis)

``METIS_LIBRARY``
  Full path to the METIS library

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("METIS" PROPERTIES
  DESCRIPTION "Serial Graph Partitioning"
)

# Try to locate METIS header
find_path(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis include include/metis Lib METISLib
  NO_DEFAULT_PATH)
find_path(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis include include/metis Lib METISLib)

# Set a name of the METIS library. This is typically `metis` or `scotchmetis`
set(METIS_LIB_NAME metis
    CACHE STRING "Name of the METIS library (default: metis).")

find_library(METIS_LIBRARY ${METIS_LIB_NAME}
  PATH_SUFFIXES lib
  NO_DEFAULT_PATH)
find_library(METIS_LIBRARY ${METIS_LIB_NAME}
  PATH_SUFFIXES lib)

# we need to check whether we need to link m, copy the lazy solution from FindBLAS and FindLAPACK here.
if(METIS_LIBRARY AND NOT WIN32)
  set(METIS_NEEDS_LIBM 1)
endif()

# If METIS_LIB_NAME contains "scotch", link against PTScotch library
string(FIND METIS_LIB_NAME "scotch" METIS_NEEDS_SCOTCH)
if(NOT METIS_NEEDS_SCOTCH EQUAL -1)
  include(CMakeFindDependencyMacro)
  find_dependency(PTScotch)
  set(METIS_NEEDS_SCOTCH TRUE)
  set(METIS_HAS_SCOTCH ${PTScotch_FOUND})
else()
  set(METIS_NEEDS_SCOTCH FALSE)
  set(METIS_HAS_SCOTCH TRUE)
endif()

mark_as_advanced(METIS_INCLUDE_DIR METIS_LIBRARY METIS_LIB_NAME
                 METIS_NEEDS_LIBM METIS_NEEDS_SCOTCH METIS_HAS_SCOTCH)

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

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("METIS"
  REQUIRED_VARS
    METIS_LIBRARY METIS_INCLUDE_DIR METIS_HAS_SCOTCH
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
  target_link_libraries(METIS::METIS INTERFACE
    $<$<BOOL:${METIS_NEEDS_LIBM}>:m>
    $<$<BOOL:${METIS_NEEDS_SCOTCH}>:PTScotch::Scotch>
  )
endif()

# register all METIS related flags
set(HAVE_METIS ${METIS_FOUND})
if(METIS_FOUND)
  dune_register_package_flags(LIBRARIES "METIS::METIS")
endif()
