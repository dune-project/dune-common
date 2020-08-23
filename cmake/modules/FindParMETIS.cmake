#[=======================================================================[.rst:
FindParMETIS
------------

Find Parallel Graph Partitioning library ParMETIS
(see http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview)

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``ParMETIS::ParMETIS``
  The libraries, flags, and includes to use for ParMETIS, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``ParMETIS_FOUND``
  The ParMETIS library with all its dependencies is found

Cache Variables
^^^^^^^^^^^^^^^

The following variables may be set to influence this module's behavior:

``PARMETIS_INCLUDE_DIR``
  Include directory where the parmetis.h is found.

``PARMETIS_LIB_NAME``
  Name of the ParMETIS library (default: parmetis)

``PARMETIS_LIBRARY``
  Full path to the ParMETIS library

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("ParMETIS" PROPERTIES
  DESCRIPTION "Parallel Graph Partitioning"
)

# The PTScotch library provides a wrapper around some functions of ParMETIS, since not
# the full interface, you have to request it explicitly.
option(ENABLE_PTSCOTCH_PARMETIS "Use the (PT)Scotch library as ParMETIS compatibility library" OFF)

# find package dependencies first
include(CMakeFindDependencyMacro)
find_dependency(METIS)
find_dependency(MPI)

find_path(PARMETIS_INCLUDE_DIR parmetis.h
  PATH_SUFFIXES include parmetis
  HINTS ${PARMETIS_DIR} ${PARMETIS_ROOT})

# search ParMETIS library
find_library(PARMETIS_LIBRARY parmetis
  HINTS ${PARMETIS_DIR} ${PARMETIS_ROOT})
if(ENABLE_PTSCOTCH_PARMETIS)
  find_library(PARMETIS_LIBRARY ptscotchparmetis
    HINTS ${PARMETIS_DIR} ${PARMETIS_ROOT})
endif()

mark_as_advanced(PARMETIS_INCLUDE_DIR PARMETIS_LIBRARY)

# determine version of ParMETIS installation
find_file(PARMETIS_HEADER_FILE parmetis.h
  PATHS ${PARMETIS_INCLUDE_DIR}
  NO_DEFAULT_PATH)
if(PARMETIS_HEADER_FILE)
  file(READ "${PARMETIS_HEADER_FILE}" parmetisheader)
  string(REGEX REPLACE ".*#define PARMETIS_MAJOR_VERSION[ ]+([0-9]+).*" "\\1" ParMETIS_MAJOR_VERSION "${parmetisheader}")
  string(REGEX REPLACE ".*#define PARMETIS_MINOR_VERSION[ ]+([0-9]+).*" "\\1" ParMETIS_MINOR_VERSION "${parmetisheader}")
  if(ParMETIS_MAJOR_VERSION GREATER_EQUAL 0 AND ParMETIS_MINOR_VERSION GREATER_EQUAL 0)
    set(ParMETIS_VERSION "${ParMETIS_MAJOR_VERSION}.${ParMETIS_MINOR_VERSION}")
  endif()
endif()
unset(PARMETIS_HEADER_FILE CACHE)

# set a flag whether all ParMETIS dependencies are found correctly
if(METIS_FOUND AND MPI_FOUND)
  set(PARMETIS_DEPENDENCIES_FOUND TRUE)

  # minimal requires METIS version 5.0 for ParMETIS >= 4.0
  if (ParMETIS_VERSION VERSION_GREATER_EQUAL "4.0"
      AND METIS_VERSION VERSION_LESS "5.0")
    set(PARMETIS_DEPENDENCIES_FOUND FALSE)
  endif()
endif()

# If ptscotch-parmetis is requested, find package PTScotch
if(ENABLE_PTSCOTCH_PARMETIS)
  include(CMakeFindDependencyMacro)
  find_dependency(PTScotch COMPONENTS PTSCOTCH)
  set(HAVE_PTSCOTCH_PARMETIS ${PTScotch_FOUND})
  if(PTScotch_FOUND AND MPI_FOUND)
    set(PARMETIS_DEPENDENCIES_FOUND TRUE)
  endif()
endif()

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ParMETIS"
  REQUIRED_VARS
    PARMETIS_LIBRARY PARMETIS_INCLUDE_DIR PARMETIS_DEPENDENCIES_FOUND
  VERSION_VAR
    ParMETIS_VERSION
)

# create imported target ParMETIS::ParMETIS
if(ParMETIS_FOUND AND NOT TARGET ParMETIS::ParMETIS)
  add_library(ParMETIS::ParMETIS UNKNOWN IMPORTED)
  set_target_properties(ParMETIS::ParMETIS PROPERTIES
    IMPORTED_LOCATION ${PARMETIS_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${PARMETIS_INCLUDE_DIR}
  )

  # link against required dependencies
  target_link_libraries(ParMETIS::ParMETIS INTERFACE MPI::MPI_CXX)

  # link against PTScotch or METIS if needed
  if(ENABLE_PTSCOTCH_PARMETIS AND PTScotch_FOUND)
    target_link_libraries(ParMETIS::ParMETIS INTERFACE PTScotch::PTScotch)
  else()
    target_link_libraries(ParMETIS::ParMETIS INTERFACE METIS::METIS)
  endif()
endif()
