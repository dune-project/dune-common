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
  set(_METIS_LM_LIBRARY "-lm")
endif()

# check metis library
if(METIS_LIBRARY)
  set(_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}") # do a backup
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${METIS_LIBRARY} ${_METIS_LM_LIBRARY})
  include(CheckFunctionExists)
  check_function_exists(METIS_PartGraphKway HAVE_METIS_PARTGRAPHKWAY)

  if(NOT HAVE_METIS_PARTGRAPHKWAY)
    # Maybe we are using static scotch libraries. In this case we need to link
    # the other scotch libraries too. Let's make a best effort.
    # Get the path where METIS_LIBRARY resides
    get_filename_component(_lib_root ${METIS_LIBRARY} DIRECTORY)
    # Search for additional libs only in this directory.
    # Otherwise we might find incompatible ones, e.g. for int instead of long
    find_library(SCOTCH_LIBRARY scotch PATHS ${_lib_root} "The Scotch library." NO_DEFAULT_PATH)
    find_library(SCOTCHERR_LIBRARY scotcherr PATHS ${_lib_root} "The Scotch error library."
      NO_DEFAULT_PATH)
    if(SCOTCH_LIBRARY AND SCOTCHERR_LIBRARY)
      set(_METIS_SCOTCH_LIBRARIES ${SCOTCH_LIBRARY} ${SCOTCHERR_LIBRARY} ${STDTHREAD_LINK_FLAGS} )
      set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES} ${METIS_LIBRARY} ${_METIS_SCOTCH_LIBRARIES} ${_METIS_LM_LIBRARY})
      # unset HAVE_METIS_PARTGRAPHKWAY to force another
      # run of check_function_exists(METIS_PartGraphKway
      unset(HAVE_METIS_PARTGRAPHKWAY CACHE)
      check_function_exists(METIS_PartGraphKway HAVE_METIS_PARTGRAPHKWAY)
    endif()
  endif()
  set(CMAKE_REQUIRED_LIBRARIES "${_CMAKE_REQUIRED_LIBRARIES}")
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
  # We need to cache METIS_LIBRARIES as for subsequent runs
  # The scotch stuff will not be set again!!!
  set(METIS_LIBRARIES ${METIS_LIBRARY} ${_METIS_SCOTCH_LIBRARIES} ${_METIS_LM_LIBRARY}
    CACHE STRING "List of all libraries needed to link to METIS")
  set(HAVE_METIS ${METIS_FOUND})
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determing location of METIS succeeded:\n"
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
