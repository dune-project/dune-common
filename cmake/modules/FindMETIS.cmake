# .. cmake_module::
#
#    Find the METIS library
#
#    You may set the following variables to modify the
#    behaviour of this module:
#
#    :ref:`METIS_ROOT`
#       Prefix, where METIS is installed
#
#    :ref:`METIS_LIB_NAME`
#       Name of the METIS library (default: metis)
#
#    :ref:`METIS_LIBRARY`
#       Full path to the METIS library
#
#    Sets the following variables:
#
#    :code:`METIS_FOUND`
#       True if the METIS library was found.
#
#    :code:`METIS_LIBRARY`
#       Full path to the METIS library
#
#    :code:`METIS_LIBRARIES`
#       List of libraries needed for linking with METIS
#
# .. cmake_variable:: METIS_ROOT
#
#   You may set this variable to have :ref:`FindMETIS` look
#   for the METIS library and includes in the given path
#   before inspecting default system paths.
#
# .. cmake_variable:: METIS_LIB_NAME
#
#   You may set this variable to specify the name of the METIS
#   library that :ref:`FindMETIS` looks for.
#
# .. cmake_variable:: METIS_LIBRARY
#
#   You may set this variable to specify the full path to the METIS
#   library, that should be used by :ref:`FindMETIS`.
#

# search metis header
find_path(METIS_INCLUDE_DIR metis.h
  PATHS ${METIS_DIR} ${METIS_ROOT}
  PATH_SUFFIXES metis include include/metis Lib METISLib
  NO_DEFAULT_PATH
  DOC "Include directory of metis")
find_path(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis include include/metis Lib METISLib)

set(METIS_LIBRARY METIS_LIBRARY-NOTFOUND CACHE FILEPATH "Full path of the METIS library")

# check metis header
include(CMakePushCheckState)
cmake_push_check_state() # Save variables
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${METIS_INCLUDE_DIR})
check_include_file(metis.h METIS_FOUND)

# search metis library
if(NOT METIS_LIB_NAME)
  set(METIS_LIB_NAME metis)
endif(NOT METIS_LIB_NAME)

find_library(METIS_LIBRARY ${METIS_LIB_NAME}
  PATHS ${METIS_DIR} ${METIS_ROOT}
  PATH_SUFFIXES lib
  NO_DEFAULT_PATH)
find_library(METIS_LIBRARY ${METIS_LIB_NAME}
  PATH_SUFFIXES lib
)

# we need to check whether we need to link m, copy the lazy solution from FindBLAS and FindLAPACK here.
if(METIS_LIBRARY AND NOT WIN32)
  list(APPEND METIS_LIBRARY "-lm")
endif()

# check metis library
if(METIS_LIBRARY)
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${METIS_LIBRARY})
  include(CheckFunctionExists)
  check_function_exists(METIS_PartGraphKway HAVE_METIS_PARTGRAPHKWAY)
endif(METIS_LIBRARY)

# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "METIS"
  DEFAULT_MSG
  METIS_INCLUDE_DIR
  METIS_LIBRARY
  HAVE_METIS_PARTGRAPHKWAY
)

cmake_pop_check_state()

mark_as_advanced(METIS_INCLUDE_DIR METIS_LIBRARIES METIS_LIB_NAME)

# if both headers and library are found, store results
if(METIS_FOUND)
  set(METIS_INCLUDE_DIRS ${METIS_INCLUDE_DIR})
  set(METIS_LIBRARIES ${METIS_LIBRARY})
  set(HAVE_METIS ${METIS_FOUND})
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determing location of METIS succeded:\n"
    "Include directory: ${METIS_INCLUDE_DIRS}\n"
    "Library directory: ${METIS_LIBRARIES}\n\n")
else(METIS_FOUND)
  # log errornous result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determing location of METIS failed:\n"
    "Include directory: ${METIS_INCLUDE_DIRS}\n"
    "Library directory: ${METIS_LIBRARIES}\n\n")
endif(METIS_FOUND)

# register all METIS related flags
if(METIS_FOUND)
  dune_register_package_flags(LIBRARIES "${METIS_LIBRARIES}"
                              INCLUDE_DIRS "${METIS_INCLUDE_DIRS}")
endif()
