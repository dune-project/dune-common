# Accepts the following variables:
#
# METIS_DIR: Prefix where ParMETIS is installed.
# METIS_LIB_NAME: Name of the METIS library (default: metis).
# METIS_LIBRARY: Full path of the METIS library.

# Sets the following variables:
#
# METIS_LIBRARY: Full path of the METIS library.
# METIS_FOUND: True if ParMETIS was found.
# METIS_LIBRARIES: List of all libraries needed for linking with METIS,
#
# Provides the following macros:
#
# find_package(METIS)
#
# Searches for METIS (See above)
#
#
# add_dune_metis_flags(TARGETS)
#
# Adds the necessary flags to comile and link TARGETS with ParMETIS support.
#
find_path(METIS_INCLUDE_DIRS metis.h PATHS ${METIS_DIR}
  PATH_SUFFIXES metis NO_DEFAULT_PATH
  DOC "Include directory of metis")
find_path(METIS_INCLUDE_DIRS metis.h PATH_SUFFIXES metis)
set(METIS_LIBRARY METIS_LIBRARY-NOTFOUND CACHE FILEPATH "Full path of the METIS library")

include(CMakePushCheckState)

cmake_push_check_state() # Save variables
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${METIS_INCLUDE_DIRS})
check_include_file(metis.h METIS_FOUND)
if(NOT METIS_LIB_NAME)
  set(METIS_LIB_NAME metis)
endif(NOT METIS_LIB_NAME)

find_library(METIS_LIBRARY ${METIS_LIB_NAME} PATHS ${ParMETIS_DIR}
  PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library(METIS_LIBRARY ${METIS_LIB_NAME})

if(METIS_LIBRARY)
  set(METIS_LIBRARIES ${METIS_LIBRARY})
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${METIS_LIBRARIES})
  include(CheckSymbolExists)
  check_symbol_exists(METIS_PartMeshDual metis.h METIS_FOUND)
endif(METIS_LIBRARY)
cmake_pop_check_state()

if(METIS_FOUND)
  message(STATUS "METIS found. Include directories are ${METIS_INCLUDE_DIRS} and libraries are ${METIS_LIBRARIES}")
else(METIS_FOUND)
  message(WARNING "No functional METIS found.")
endif(METIS_FOUND)

function(add_dune_metis_flags _targets)
  if(METIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${METIS_LIBRARY})
    endforeach(_target ${_targets})
    set_property(TARGET ${_targets} APPEND PROPERTIES
      COMPILE_DIRECTORIES ${METIS_INCLUDE_DIRS})
  endif(METIS_FOUND)
endfunction(add_dune_metis_flags _targets)
