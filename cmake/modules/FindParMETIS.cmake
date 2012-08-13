# Module that checks whether ParMETIS is available.
#
# Accepts the following variables:
#
# ParMETIS_DIR: Prefix where ParMETIS is installed.
# METIS_LIB_NAME: Name of the METIS library (default: metis).
# PARMETIS_LIB_NAME: Name of the ParMETIS library (default: parmetis).
# METIS_LIBRARY: Full path of the METIS library.
# ParMETIS_LIBRARY: Full path of the ParMETIS library

# Sets the following variables:
#
# METIS_LIBRARY: Full path of the METIS library.
# ParMETIS_LIBRARY: Full path of the ParMETIS library.
# ParMETIS_FOUND: True if ParMETIS was found.
# ParMETIS_LIBRARIES: List of all libraries needed for linking with ParMETIS,
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
include(DuneMPI)

foreach(_dir ${ParMETIS_DIR})
  list(APPEND _ParMETIS_INCLUDE_DIRS ${ParMETIS_DIR} ${ParMETIS_DIR}/include)
endforeach(_dir ${ParMETIS_DIR})

find_path(ParMETIS_INCLUDE_DIRS parmetis.h PATHS ${_ParMETIS_INCLUDE_DIRS} NO_DEFAULT_PATH)

if(NOT ParMETIS_INCLUDE_DIRS)
  find_path(ParMETIS_INCLUDE_DIRS parmetis.h)
endif(NOT ParMETIS_INCLUDE_DIRS)


set(METIS_LIB_NAME metis CACHE STRING "Name of the METIS library (default: metis).")
set(PARMETIS_LIB_NAME parmetis CACHE STRING "Name of the ParMETIS library (default: parmetis).")
set(METIS_LIBRARY METIS_LIBRARY-NOTFOUND CACHE FILEPATH "Full path of the METIS library")
set(ParMETIS_LIBRARY ParMETIS_LIBRARY-NOTFOUND CACHE FILEPATH "Full path of the ParMETIS library")

# save old variables
set(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
set(CMAKE_REQUIRED_FLAGS_SAVE    ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_LIBRARIES_SAVE ${CMAKE_REQUIRED_LIBRARIES})

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MPI_DUNE_INCLUDE_PATH})
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${MPI_DUNE_COMPILE_FLAGS}")

if(ParMETIS_INCLUDE_DIRS)
  check_include_file(parmetis.h ParMETIS_FOUND)
endif(ParMETIS_INCLUDE_DIRS)

if(ParMETIS_FOUND)
  set(ParMETIS_INCLUDE_PATH ${CMAKE_REQUIRED_INCLUDES})
  set(ParMETIS_COMPILE_FLAGS "${CMAKE_REQUIRED_FLAGS} -DENABLE_PARMETIS=1")

  foreach(_dir ${ParMETIS_DIR})
    list(APPEND _ParMETIS_LIB_DIRS ${ParMETIS_DIR} ${ParMETIS_DIR}/lib)
  endforeach(_dir ${ParMETIS_DIR})

  find_library(METIS_LIBRARY metis PATHS ${_ParMETIS_LIB_DIRS} NO_DEFAULT_PATH)

  if(NOT METIS_LIBRARY)
    find_library(METIS_LIBRARY metis)
  endif(NOT METIS_LIBRARY)

  find_library(ParMETIS_LIBRARY parmetis PATHS ${_ParMETIS_LIB_DIRS} NO_DEFAULT_PATH)

  if(NOT ParMETIS_LIBRARY)
    find_library(ParMETIS_LIBRARY parmetis)
  endif(NOT ParMETIS_LIBRARY)

  if(ParMETIS_LIBRARY)
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${ParMETIS_LIBRARY} ${METIS_LIBRARY} ${MPI_DUNE_LIBRARIES})
    include(CheckSymbolExists)
    check_symbol_exists(parmetis_v3_partkway parmetis.h ParMETIS_FOUND)
  endif(ParMETIS_LIBRARY)

  if(ParMETIS_FOUND)
    set(ParMETIS_LIBRARIES "${ParMETIS_LIBRARY};${METIS_LIBRARY};${MPI_DUNE_LIBRARIES}"
      CACHE FILEPATH "ParMETIS libraries needed for linking")
    set(ParMETIS_LINK_FLAGS "${DUNE_MPI_LINK_FLAGS}" CACHE STRING "ParMETIS link flags")
  else(ParMETIS_FOUND)
    set(ParMETIS_LIBRARIES "" CACHE FILEPATH "ParMETIS libraries needed for linking")
    set(ParMETIS_LINK_FLAGS "" CACHE STRING "ParMETIS link flags")
  endif(ParMETIS_FOUND)

endif(ParMETIS_FOUND)

if(ParMETIS_FOUND)
  message(STATUS "ParMETIS library was found")
else(ParMETIS_FOUND)
  message(STATUS "No ParMETIS available")
endif(ParMETIS_FOUND)
#restore old values

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
set(CMAKE_REQUIRED_FLAGS    ${CMAKE_REQUIRED_FLAGS_SAVE})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_SAVE})

# adds ParMETIS flags to the targets
function(add_dune_parmetis_flags _targets)
  if(ParMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${ParMETIS_LIBRARY} ${METIS_LIBRARY})
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${PARMETIS_COMPILE_FLAGS} -DENABLE_PARMETS=1")
    endforeach(_target ${_targets})
    add_dune_mpi_flags(${_targets})
  endif(ParMETIS_FOUND)
endfunction(add_dune_parmetis_flags)
