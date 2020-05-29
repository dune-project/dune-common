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

# find package dependencies first
find_package(METIS)
find_package(MPI)

find_path(PARMETIS_INCLUDE_DIR parmetis.h
  PATH_SUFFIXES include parmetis
  HINTS ${PARMETIS_DIR} ${PARMETIS_ROOT})

# Set a name of the METIS library. This is typically `parmetis` or `ptscotchparmetis`
set(PARMETIS_LIB_NAME parmetis
    CACHE STRING "Name of the ParMETIS library (default: parmetis).")

# search ParMETIS library
find_library(PARMETIS_LIBRARY ${PARMETIS_LIB_NAME}
  HINTS ${PARMETIS_DIR} ${PARMETIS_ROOT})

# If PARMETIS_LIB_NAME contains "ptscotch", link against PTScotch library
string(FIND PARMETIS_LIB_NAME "ptscotch" PARMETIS_NEEDS_PTSCOTCH)
if(NOT PARMETIS_NEEDS_PTSCOTCH EQUAL -1)
  include(CMakeFindDependencyMacro)
  find_dependency(PTScotch)
  set(PARMETIS_NEEDS_PTSCOTCH TRUE)
  set(PARMETIS_HAS_PTSCOTCH ${PTScotch_FOUND})
else()
  set(PARMETIS_NEEDS_PTSCOTCH FALSE)
  set(PARMETIS_HAS_PTSCOTCH TRUE)
endif()

mark_as_advanced(PARMETIS_INCLUDE_DIR PARMETIS_LIBRARY PARMETIS_LIB_NAME
                 PARMETIS_NEEDS_PTSCOTCH PARMETIS_HAS_PTSCOTCH)

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

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ParMETIS"
  REQUIRED_VARS
    PARMETIS_LIBRARY PARMETIS_INCLUDE_DIR METIS_FOUND MPI_FOUND PARMETIS_HAS_PTSCOTCH
  VERSION_VAR
    ParMETIS_VERSION
)

# create imported target ParMETIS::ParMETIS
if(PARMETIS_FOUND AND NOT TARGET ParMETIS::ParMETIS)
  add_library(ParMETIS::ParMETIS UNKNOWN IMPORTED)
  set_target_properties(ParMETIS::ParMETIS PROPERTIES
    IMPORTED_LOCATION ${PARMETIS_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${PARMETIS_INCLUDE_DIR}
  )

  # link against required dependencies METIS and MPI
  target_link_libraries(ParMETIS::ParMETIS
    INTERFACE METIS::METIS MPI::MPI_CXX)

  # link against ptscotch if needed
  target_link_libraries(ParMETIS::ParMETIS INTERFACE
    $<$<BOOL:${PARMETIS_NEEDS_PTSCOTCH}>:PTScotch::PTScotch>)
endif()
