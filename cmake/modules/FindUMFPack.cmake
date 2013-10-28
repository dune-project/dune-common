# Module that checks whether UMFPack is available.
#
# Variables used by this module which you may want to set:
# UMFPACK_ROOT   Path list to search for UMFPack
#
# Sets the following variables
#
# UMFPACK_FOUND          True if UMFPack was found and usable
# UMFPACK_INCLUDE_DIRS   Path to the UMFPack include dirs
# UMFPACK_LIBRARIES      Name of the UMFPack libraries
#

find_package(BLAS QUIET REQUIRED)
if(NOT BLAS_FOUND)
  message(WARNING "UMFPack requires BLAS which was not found, skipping the test.")
  return()
endif()

find_library(AMD_LIBRARY
  NAMES "amd"
  PATHS ${UMFPACK_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64" "AMD" "AMD/Lib"
  NO_DEFAULT_PATH
)

find_library(AMD_LIBRARY
  NAMES "amd"
  PATH_SUFFIXES "lib" "lib32" "lib64" "AMD" "AMD/Lib"
)

if(NOT AMD_LIBRARY)
  message(WARNING "UMFPack requires AMD (approximate minimum degree ordering) which was not found, skipping the test.")
  return()
endif()

#look for header files at positions given by the user
find_path(UMFPACK_INCLUDE_DIR
  NAMES "umfpack.h"
  PATHS ${UMFPACK_ROOT}
  PATH_SUFFIXES "umfpack" "include/umfpack" "suitesparse" "include" "src" "UMFPACK" "UMFPACK/Include"
  NO_DEFAULT_PATH
)
#now also look for default paths
find_path(UMFPACK_INCLUDE_DIR
  NAMES "umfpack.h"
  PATH_SUFFIXES "umfpack" "include/umfpack" "suitesparse" "include" "UMFPACK" "UMFPACK/Include"
)

#look for library at positions given by the user
find_library(UMFPACK_LIBRARY
  NAMES "umfpack"
  PATHS ${UMFPACK_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64" "UMFPACK" "UMFPACK/Lib"
  NO_DEFAULT_PATH
)
#now  also include the deafult paths
find_library(UMFPACK_LIBRARY
  NAMES "umfpack"
  PATH_SUFFIXES "lib" "lib32" "lib64" "UMFPACK" "UMFPACK/Lib"
)

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "UMFPack"
  DEFAULT_MSG
  UMFPACK_INCLUDE_DIR
  UMFPACK_LIBRARY
)

mark_as_advanced(UMFPACK_INCLUDE_DIR UMFPACK_LIBRARY)

# if both headers and library are found, store results
if(UMFPACK_FOUND)
  set(UMFPACK_INCLUDE_DIRS ${UMFPACK_INCLUDE_DIR})
  set(UMFPACK_LIBRARIES ${UMFPACK_LIBRARY})
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determining location of UMFPack succeded:\n"
    "Include directory: ${UMFPACK_INCLUDE_DIRS}\n"
    "Library directory: ${UMFPACK_LIBRARIES}\n\n")
  set(UMFPACK_DUNE_COMPILE_FLAGS "-I${UMFPACK_INCLUDE_DIRS}"
    CACHE STRING "Compile Flags used by DUNE when compiling with UMFPack programs")
  set(UMFPACK_DUNE_LIBRARIES ${UMFPACK_LIBRARIES} ${BLAS_LIBRARIES} ${AMD_LIBRARY}
    CACHE STRING "Libraries used by DUNE when linking UMFPack programs")
else(UMFPACK_FOUND)
  # log errornous result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKES_FILES_DIRECTORY}/CMakeError.log
    "Determing location of UMFPack failed:\n"
    "Include directory: ${UMFPACK_INCLUDE_DIRS}\n"
    "Library directory: ${UMFPACK_LIBRARIES}\n\n")
endif(UMFPACK_FOUND)

#set HAVE_UMFPACK for config.h
set(HAVE_UMFPACK UMFPACK_FOUND)

#add all umfpack related flags to ALL_PKG_FLAGS, this must happen regardless of a target using add_dune_umfpack_flags
if(UMFPACK_FOUND)
  set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "${UMFPACK_DUNE_COMPILE_FLAGS}")
  foreach(dir "${UMFPACK_INCLUDE_DIRS}")
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-I${dir}")
  endforeach()
endif()
