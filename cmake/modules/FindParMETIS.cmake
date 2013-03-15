# Module that checks whether ParMETIS is available.
#
# Accepts the following variables:
#
# PARMETIS_ROOT: Prefix where ParMETIS is installed.
# METIS_LIB_NAME: Name of the METIS library (default: metis).
# PARMETIS_LIB_NAME: Name of the ParMETIS library (default: parmetis).
# METIS_LIBRARY: Full path of the METIS library.
# PARMETIS_LIBRARY: Full path of the ParMETIS library

# Sets the following variables:
#
# METIS_LIBRARY: Full path of the METIS library.
# PARMETIS_LIBRARY: Full path of the ParMETIS library.
# PARMETIS_FOUND: True if ParMETIS was found.
# PARMETIS_LIBRARIES: List of all libraries needed for linking with ParMETIS,
#
# Provides the following macros:
#
# find_package(ParMETIS)
#
# Searches for ParMETIS (See above)
#
#
# add_dune_parmetis_flags(TARGETS)
#
# Adds the necessary flags to comile and link TARGETS with ParMETIS support.
#

# adds ParMETIS flags to the targets
function(add_dune_parmetis_flags _targets)
  if(PARMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${PARMETIS_LIBRARY} ${METIS_LIBRARY})
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${PARMETIS_COMPILE_FLAGS} -DENABLE_PARMETS=1")
    endforeach(_target ${_targets})
    add_dune_mpi_flags(${_targets})
  endif(PARMETIS_FOUND)
endfunction(add_dune_parmetis_flags)


include(DuneMPI)

# search for header parmetis.h
find_path(PARMETIS_INCLUDE_DIR parmetis.h
          PATHS ${PARMETIS_ROOT}
          PATH_SUFFIXES include parmetis
          NO_DEFAULT_PATH
          DOC "Include directory of ParMETIS")
find_path(PARMETIS_INCLUDE_DIR parmetis.h
          PATH_SUFFIXES include parmetis)

set(METIS_LIB_NAME metis
    CACHE STRING "Name of the METIS library (default: metis).")
set(PARMETIS_LIB_NAME parmetis
    CACHE STRING "Name of the ParMETIS library (default: parmetis).")
set(METIS_LIBRARY METIS_LIBRARY-NOTFOUND
    CACHE FILEPATH "Full path of the METIS library")
set(PARMETIS_LIBRARY ParMETIS_LIBRARY-NOTFOUND
    CACHE FILEPATH "Full path of the ParMETIS library")

# check METIS and ParMETIS headers
include(CMakePushCheckState)
cmake_push_check_state() # Save variables
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MPI_DUNE_INCLUDE_PATH} ${PARMETIS_INCLUDE_DIR})
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${MPI_DUNE_COMPILE_FLAGS}")
check_include_file(metis.h METIS_FOUND)
check_include_file(parmetis.h PARMETIS_FOUND)

if(PARMETIS_FOUND)
  set(ParMETIS_INCLUDE_PATH ${CMAKE_REQUIRED_INCLUDES})
  set(ParMETIS_COMPILE_FLAGS "${CMAKE_REQUIRED_FLAGS} -DENABLE_PARMETIS=1")

  # search METIS library
  find_library(METIS_LIBRARY metis
               PATHS ${PARMETIS_ROOT}
               PATH_SUFFIXES lib
               NO_DEFAULT_PATH)
  find_library(METIS_LIBRARY metis)

  # search ParMETIS library
  find_library(PARMETIS_LIBRARY parmetis
               PATHS ${PARMETIS_ROOT}
               PATH_SUFFIXES lib
               NO_DEFAULT_PATH)
  find_library(PARMETIS_LIBRARY parmetis)

  # check ParMETIS library
  if(PARMETIS_LIBRARY)
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${PARMETIS_LIBRARY} ${METIS_LIBRARY} ${MPI_DUNE_LIBRARIES})
    include(CheckSymbolExists)
    check_symbol_exists(parmetis_v3_partkway parmetis.h PARMETIS_FOUND)
  endif(PARMETIS_LIBRARY)
endif(PARMETIS_FOUND)

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "ParMETIS"
  DEFAULT_MSG
  PARMETIS_INCLUDE_DIR
  PARMETIS_LIBRARY
  PARMETIS_FOUND
)

mark_as_advanced(PARMETIS_INCLUDE_DIR METIS_LIBRARY PARMETIS_LIBRARY METIS_LIB_NAME PARMETIS_LIB_NAME)

#restore old values
cmake_pop_check_state()

if(PARMETIS_FOUND)
  set(PARMETIS_INCLUDE_DIRS ${PARMETIS_INCLUDE_DIR})
  set(PARMETIS_LIBRARIES "${PARMETIS_LIBRARY};${METIS_LIBRARY};${MPI_DUNE_LIBRARIES}"
      CACHE FILEPATH "ParMETIS libraries needed for linking")
  set(PARMETIS_LINK_FLAGS "${DUNE_MPI_LINK_FLAGS}"
      CACHE STRING "ParMETIS link flags")
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determing location of ParMETIS succeded:\n"
    "Include directory: ${PARMETIS_INCLUDE_DIRS}\n"
    "Library directory: ${PARMETIS_LIBRARIES}\n\n")
endif(PARMETIS_FOUND)
