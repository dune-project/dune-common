# Module that checks for the GNU MP Bignum (GMP) library, include
# dirs and
#
# Variables used by this module which you may want to set:
# GMP_ROOT Path list to search for GMP
#
# Sets the following variables:
# GMP_FOUND True if the GMP library was found.
#

# search for location of header gmpxx.h", only at positions given by the user
find_path(GMP_INCLUDE_DIR
  NAMES "gmpxx.h"
  PATHS ${GMP_PREFIX} ${GMP_ROOT}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
# try default paths now
find_path(GMP_INCLUDE_DIR
  NAMES "gmpxx.h")

# check if header is accepted
include(CMakePushCheckState)
cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${GMP_INCLUDE_DIR})
include(CheckIncludeFileCXX)
check_include_file_cxx("gmpxx.h" GMP_FOUND)

# look for library gmp, only at positions given by the user
find_library(GMP_LIB gmp
  PATHS ${GMP_PREFIX} ${GMP_ROOT}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH
  DOC "GNU GMP library")
# try default paths now
find_library(GMP_LIB gmp)

# look for library gmpxx, only at positions given by the user
find_library(GMPXX_LIB gmpxx
  PATHS ${GMP_PREFIX} ${GMP_ROOT}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH
  DOC "GNU GMPXX library")
# try default paths now
find_library(GMPXX_LIB gmpxx)

# check if library works
set(GMP_FOUND "GMP_FOUND-NOTFOUND")
if(GMP_LIB AND GMPXX_LIB)
  include(CheckSymbolExists)
  check_symbol_exists(__gmpz_abs ${GMP_LIB} GMP_FOUND)
endif(GMP_LIB AND GMPXX_LIB)
cmake_pop_check_state()

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "GMP"
  DEFAULT_MSG
  GMP_INCLUDE_DIR GMP_LIB GMPXX_LIB GMP_FOUND
)

mark_as_advanced(GMP_LIB GMPXX_LIB GMP_INCLUDE_DIR)

# if both headers and library are found, store results
if(GMP_FOUND)
  set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
  set(GMP_LIBRARIES ${GMP_LIB} ${GMPXX_LIB})
  set(GMP_COMPILE_FLAGS "-DENABLE_GMP=1")
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determing location of GMP succeded:\n"
    "Include directory: ${GMP_INCLUDE_DIRS}\n"
    "Library directory: ${GMP_LIBRARIES}\n\n")
else(GMP_FOUND)
  # log errornous result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determing location of GMP failed:\n"
    "Include directory: ${GMP_INCLUDE_DIR}\n"
    "gmp library directory: ${GMP_LIB}\n"
    "gmpx library directory: ${GMPXX_LIB}\n\n")
endif(GMP_FOUND)

# set HAVE_GMP for config.h
set(HAVE_GMP GMP_FOUND)

#add all GMP related flags to ALL_PKG_FLAGS, this must happen regardless of a target using add_dune_gmp_flags
if(HAVE_GMP)
  set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-DENABLE_GMP=1")
  foreach(dir ${GMP_INCLUDE_DIR})
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-I${dir}")
  endforeach()
endif()