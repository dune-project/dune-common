# .. cmake_module::
#
#    Find the GNU MPFR library
#    and the corresponding C++ bindings MPFR-C++
#
#    You may set the following variables to modify the
#    behaviour of this module:
#
#    :ref:`MPFR_ROOT`
#       Path list to search for MPFR and MPFR-C++
#
#    Sets the following variables:
#
#    :code:`MPFR_FOUND`
#       True if the GMP library, the GMPxx headers and
#       the GMPxx library were found.
#
# .. cmake_variable:: MPFR_ROOT
#
#   You may set this variable to have :ref:`FindGMP` look
#   for the gmp and gmpxx packages in the given path before
#   inspecting system paths.
#


# search for location of header mpfr.h", only at positions given by the user
find_path(MPFR_INCLUDE_DIR
  NAMES "mpfr.h"
  PATHS ${MPFR_PREFIX} ${MPFR_ROOT}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
# try default paths now
find_path(MPFR_INCLUDE_DIR
  NAMES "mpfr.h")

# search for location of header mpfr.h", only at positions given by the user
find_path(MPREAL_INCLUDE_DIR
  NAMES "mpreal.h"
  PATHS ${MPFR_PREFIX} ${MPFR_ROOT}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)
# try default paths now
find_path(MPREAL_INCLUDE_DIR
  NAMES "mpreal.h")

# check if header is accepted
include(CMakePushCheckState)
cmake_push_check_state()
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MPFR_INCLUDE_DIR} ${MPREAL_INCLUDE_DIR})
include(CheckIncludeFileCXX)
check_include_file_cxx("mpreal.h" MPREAL_HEADER_WORKS)

# look for library gmp, only at positions given by the user
find_library(GMP_LIB gmp
  PATHS ${GMP_PREFIX} ${GMP_ROOT}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH
  DOC "GNU GMP library")
# try default paths now
find_library(GMP_LIB gmp)

# look for library gmpxx, only at positions given by the user
find_library(MPFR_LIB mpfr
  PATHS ${MPFR_PREFIX} ${MPFR_ROOT}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH
  DOC "GNU MPFR library")
# try default paths now
find_library(MPFR_LIB mpfr)

# check if library works
if(GMP_LIB AND MPFR_LIB)
  include(CheckSymbolExists)
  check_library_exists(${GMP_LIB} __gmpz_abs "" GMP_LIB_WORKS)
endif(GMP_LIB AND MPFR_LIB)
cmake_pop_check_state()

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "MPFR"
  DEFAULT_MSG
  MPFR_INCLUDE_DIR MPREAL_INCLUDE_DIR GMP_LIB MPFR_LIB MPREAL_HEADER_WORKS GMP_LIB_WORKS
)

mark_as_advanced(GMP_LIB MPFR_LIB MPFR_INCLUDE_DIR MPREAL_INCLUDE_DIR)

# text for feature summary
set_package_properties("MPFR" PROPERTIES
  DESCRIPTION "GNU MPFR multi-precision library including the C++ bindings MPFR-C++"
  PURPOSE "Multi-precision quadrature rules, basis function evaluation etc.")

# if MPFR headers, GMP library, and MPFR library are found, store results
if(MPFR_FOUND)
  set(MPFR_INCLUDE_DIRS ${MPFR_INCLUDE_DIR} ${MPREAL_INCLUDE_DIR})
  set(MPFR_LIBRARIES ${GMP_LIB} ${MPFR_LIB})
  set(MPFR_COMPILE_FLAGS "-DENABLE_MPFR=1")
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determining location of GMP, MPREAL succeeded:\n"
    "Include directory: ${MPFR_INCLUDE_DIRS}\n"
    "Library directory: ${MPFR_LIBRARIES}\n\n")
else()
  # log errornous result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining location of GMP, MPREAL failed:\n"
    "Include directory: ${MPFR_INCLUDE_DIRS}\n"
    "gmp library directory: ${GMP_LIB}\n"
    "mpfr library directory: ${MPFR_LIB}\n\n")
endif()

# set HAVE_MPFR for config.h
set(HAVE_MPFR ${MPFR_FOUND})

# register all MPREAL related flags
if(HAVE_MPFR)
  dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_MPFR=1"
                              LIBRARIES "${GMP_LIB};${MPFR_LIB}"
                              INCLUDE_DIRS "${MPFR_INCLUDE_DIR};${MPREAL_INCLUDE_DIR}")
endif()
