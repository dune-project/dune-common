#[=======================================================================[.rst:
FindPTScotch
------------

Find library PTScotch, i.e. Software package and libraries for sequential
and parallel graph partitioning, static mapping and clustering, sequential
mesh and hypergraph partitioning, and sequential and parallel sparse matrix
block ordering

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``PTScotch::Scotch``
  The sequential Scotch library to link against
``PTScotch::PTScotch``
  The parallel PTScotch library to link against

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``PTScotch_FOUND``
  The Scotch and PTScotch library with all its dependencies is found

Cache Variables
^^^^^^^^^^^^^^^

The following variables may be set to influence this module's behavior:

``PTSCOTCH_SUFFIX``
  Scotch might be compiled using different integer sizes (int32, int64, long).
  When this is is set the headers and libaries are search under the suffix
  :code:`include/scotch-${PTSCOTCH_SUFFIX}`, and :code:`lib/scotch-${PTSCOTCH_SUFFIX}`,
  respectively.

``SCOTCH_INCLUDE_DIR``
  Include directory where the scotch.h is found.

``PTSCOTCH_INCLUDE_DIR``
  Include directory where the ptscotch.h is found.

``SCOTCH_LIBRARY``
  Full path to the scotch library

``PTSCOTCH_LIBRARY``
  Full path to the ptscotch library

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("PTScotch" PROPERTIES
  DESCRIPTION "Sequential and Parallel Graph Partitioning"
)

# find dependency for PTScotch
find_package(MPI)

# search directory might have the PATH_SUFFIX scotch-SUFFIX
if(PTSCOTCH_SUFFIX)
  set(PATH_SUFFIXES "scotch-${PTSCOTCH_SUFFIX}")
else()
  set(PATH_SUFFIXES "scotch")
endif()

# Try to find the include files
find_path(SCOTCH_INCLUDE_DIR scotch.h
  PATH_SUFFIXES ${PATH_SUFFIXES}
  NO_DEFAULT_PATH)
find_path(SCOTCH_INCLUDE_DIR scotch.h
  PATH_SUFFIXES ${PATH_SUFFIXES})

find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h
  HINTS ${SCOTCH_INCLUDE_DIR}
  PATH_SUFFIXES ${PATH_SUFFIXES}
  NO_DEFAULT_PATH)
find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h
  PATH_SUFFIXES ${PATH_SUFFIXES})

# Try to find the (pt)scotch libraries
macro(_find_ptscotch_library libvar libname)
  find_library(${libvar} ${libname}
    PATH_SUFFIXES ${PATH_SUFFIXES}
    NO_DEFAULT_PATH)
  find_library(${libvar} ${libname})
endmacro(_find_ptscotch_library)

_find_ptscotch_library(SCOTCH_LIBRARY scotch)
_find_ptscotch_library(SCOTCHERR_LIBRARY scotcherr)
_find_ptscotch_library(PTSCOTCH_LIBRARY ptscotch)
_find_ptscotch_library(PTSCOTCHERR_LIBRARY ptscotcherr)

mark_as_advanced(SCOTCH_INCLUDE_DIR SCOTCH_LIBRARY SCOTCHERR_LIBRARY
                 PTSCOTCH_INCLUDE_DIR PTSCOTCH_LIBRARY PTSCOTCHERR_LIBRARY)

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("PTScotch"
  REQUIRED_VARS
    SCOTCH_LIBRARY SCOTCHERR_LIBRARY SCOTCH_INCLUDE_DIR
    PTSCOTCH_LIBRARY PTSCOTCHERR_LIBRARY PTSCOTCH_INCLUDE_DIR
    MPI_FOUND
)

if(PTScotch_FOUND)
  # Define an imported target for the sequential Scotch library
  if(NOT TARGET PTScotch::Scotch)
    add_library(PTScotch::Scotch UNKNOWN IMPORTED)
    set_target_properties(PTScotch::Scotch PROPERTIES
      IMPORTED_LOCATION ${SCOTCH_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${SCOTCH_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES ${SCOTCHERR_LIBRARY}
    )
  endif()

  # Define an imported target for the parallel PTScotch library
  if(NOT TARGET PTScotch::PTScotch)
    add_library(PTScotch::PTScotch UNKNOWN IMPORTED)
    set_target_properties(PTScotch::PTScotch PROPERTIES
      IMPORTED_LOCATION ${PTSCOTCH_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${PTSCOTCH_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES ${PTSCOTCHERR_LIBRARY}
    )
    target_link_libraries(PTScotch::PTScotch
      INTERFACE PTScotch::Scotch MPI::MPI_CXX)
  endif()
endif()

set(HAVE_PTSCOTCH ${PTScotch_FOUND})
if(PTScotch_FOUND)
  dune_register_package_flags(LIBRARIES "PTScotch::PTScotch")
endif()
