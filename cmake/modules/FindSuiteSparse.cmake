# Module that checks whether SuiteSparse is available.
#
# Synopsis:
#   find_package( SuiteSparse COMPONENTS < list-of-components > )
#
#  Components are:
#   amd, btf, camd, ccolamd, cholmod, colamd, cxsparse,
#   klu, ldl, RBio, spqr, umfpack
#
# Variables used by this module which you may want to set:
# SUITESPARSE_ROOT   Path list to search for SuiteSparse
#
# Sets the following variables
#
# SUITESPARSE_FOUND          True if SuiteSparse was found and usable in the desired setting
# SUITESPARSE_INCLUDE_DIRS   Path to the SuiteSparse include dirs
# SUITESPARSE_LIBRARIES      Name of the SuiteSparse libraries
# SUITESPARSE_<COMP>_FOUND   True if <COMP> was found as part of SuiteSparse
#

find_package(BLAS QUIET REQUIRED)
if(NOT BLAS_FOUND)
  message(WARNING "SuiteSparse requires BLAS which was not found, skipping the test.")
  return()
endif()

# look for desired componenents
set( SUITESPARSE_COMPONENTS ${SuiteSparse_FIND_COMPONENTS} )

# resolve inter comp. dependencies
list( FIND SUITESPARSE_COMPONENTS "umfpack" WILL_USE_UMFPACK )
if( NOT WILL_USE_UMFPACK EQUAL -1 )
  list( APPEND SUITESPARSE_COMPONENTS amd cholmod )
endif()
list( FIND SUITESPARSE_COMPONENTS "cholmod" WILL_USE_CHOLMOD )
if( NOT WILL_USE_CHOLMOD EQUAL -1 )
  list( APPEND SUITESPARSE_COMPONENTS amd camd colamd ccolamd )
endif()

if( SUITESPARSE_COMPONENTS )
  list( REMOVE_DUPLICATES SUITESPARSE_COMPONENTS )
endif()

set( ready TRUE )
foreach( modname ${SUITESPARSE_COMPONENTS} )
  dune_module_to_uppercase( MODNAME ${modname} )
  if( NOT SUITESPARSE_${MODNAME}_FOUND )
    set( ready FALSE )
  endif()
endforeach()
if( ready )
  return ()
endif()

# find SuiteSparse config:
# look for library at positions given by the user
find_library( SUITESPARSE_CONFIG_LIB
  NAMES "suitesparseconfig"
  PATHS ${SUITESPARSE_ROOT}
  PATH_SUFFIXES "lib" "lib32" "lib64" "Lib"
  NO_DEFAULT_PATH
)

#now  also include the deafult paths
find_library( SUITESPARSE_CONFIG_LIB
  NAMES "suitesparseconfig"
  PATH_SUFFIXES "lib" "lib32" "lib64" "Lib"
)

#look for header files at positions given by the user
find_path( SUITESPARSE_INCLUDE_DIR
  NAMES "SuiteSparse_config.h"
  PATHS ${SUITESPARSE_ROOT}
  PATH_SUFFIXES "SuiteSparse_config" "SuiteSparse_config/include" "suitesparse" "include" "src" "SuiteSparse_config/Include"
  NO_DEFAULT_PATH
)
#now also look for default paths
find_path( SUITESPARSE_INCLUDE_DIR
  NAMES "SuiteSparse_config.h"
  PATHS ${SUITESPARSE_ROOT}
  PATH_SUFFIXES "SuiteSparse_config" "SuiteSparse_config/include" "suitesparse" "include" "src" "SuiteSparse_config/Include"
)

foreach( modname ${SUITESPARSE_COMPONENTS} )
  dune_module_to_uppercase( MODNAME ${modname} )

  #look for library at positions given by the user
  find_library( ${MODNAME}_LIBRARY
    NAMES "${modname}"
    PATHS ${SUITESPARSE_ROOT}
    PATH_SUFFIXES "lib" "lib32" "lib64" "${MODNAME}" "${MODNAME}/Lib"
    NO_DEFAULT_PATH
  )

  #now  also include the deafult paths
  find_library( ${MODNAME}_LIBRARY
    NAMES "${modname}"
    PATH_SUFFIXES "lib" "lib32" "lib64" "${MODNAME}" "${MODNAME}/Lib"
  )

  #look for header files at positions given by the user
  find_path(${MODNAME}_INCLUDE_DIR
    NAMES "${modname}.h"
    PATHS ${SUITESPARSE_ROOT}
    PATH_SUFFIXES "${modname}" "include/${modname}" "suitesparse" "include" "src" "${MODNAME}" "${MODNAME}/Include"
    NO_DEFAULT_PATH
  )
  #now also look for default paths
  find_path(${MODNAME}_INCLUDE_DIR
    NAMES "${modname}.h"
    PATH_SUFFIXES "${modname}" "include/${modname}" "suitesparse" "include" "${MODNAME}" "${MODNAME}/Include"
  )
endforeach()


# resolve inter-modular dependencies

# CHOLMOD requires AMD, COLAMD; CAMD and CCOLAMD is optional
if( CHOLMOD_LIBRARY )
  if( NOT AMD_LIBRARY )
    if( NOT COLAMD_LIBRARY )
      message(WARNING "CHOLMOD requires AMD and COLAMD which was not found, skipping the test.")
      return()
    endif()
  endif()

  list( APPEND CHOLMOD_LIBRARY ${AMD_LIBRARY} ${COLAMD_LIBRARY} )
  if( CAMD_LIBRARY )
    list( APPEND CHOLMOD_LIBRARY ${CAMD_LIBRARY} )
  endif()
  if( CCOLAMD_LIBRARY )
    list( APPEND CHOLMOD_LIBRARY ${CCOLAMD_LIBRARY} )
  endif()
  list( REVERSE CHOLMOD_LIBRARY )
  # remove duplicates
  list( REMOVE_DUPLICATES CHOLMOD_LIBRARY )
  list( REVERSE CHOLMOD_LIBRARY )
endif()

# UMFPack requires AMD, can depend on CHOLMOD
if( UMFPACK_LIBRARY )
  # check wether cholmod was found
  if( CHOLMOD_LIBRARY )
    list( APPEND UMFPACK_LIBRARY ${CHOLMOD_LIBRARY} )
  else()
    list( APPEND UMFPACK_LIBRARY ${AMD_LIBRARY} )
  endif()
  list( REVERSE UMFPACK_LIBRARY )
  # remove duplicates
  list( REMOVE_DUPLICATES UMFPACK_LIBRARY )
  list( REVERSE UMFPACK_LIBRARY )
endif()

set( SUITESPARSE_FOUND TRUE )
# check wether everything was found
foreach( modname ${SUITESPARSE_COMPONENTS} )
  dune_module_to_uppercase( MODNAME ${modname} )
  set( SUITESPARSE_${MODNAME}_FOUND TRUE )
  if( ${MODNAME}_LIBRARY AND ${MODNAME}_INCLUDE_DIR )
    list( APPEND SUITESPARSE_INCLUDE_DIR "${${MODNAME}_INCLUDE_DIR}" )
    list( APPEND SUITESPARSE_LIBRARY "${${MODNAME}_LIBRARY}" )
  else()
      set( SUITESPARSE_FOUND FALSE )
  endif()

  set( HAVE_SUITESPARSE_${MODNAME} 1 )
  mark_as_advanced( HAVE_SUITESPARSE_${MODNAME} )
  mark_as_advanced( SUITESPARSE_${MODNAME}_FOUND )
endforeach()

list( APPEND SUITESPARSE_LIBRARY ${SUITESPARSE_CONFIG_LIB} )

# make them unique
if( SUITESPARSE_INCLUDE_DIR )
  list( REMOVE_DUPLICATES SUITESPARSE_INCLUDE_DIR )
endif()

if( SUITESPARSE_LIBRARY )
  list( REVERSE SUITESPARSE_LIBRARY )
  list( REMOVE_DUPLICATES SUITESPARSE_LIBRARY )
  list( REVERSE SUITESPARSE_LIBRARY )
endif()


# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "SuiteSparse"
  DEFAULT_MSG
  SUITESPARSE_INCLUDE_DIR
  SUITESPARSE_LIBRARY
)

mark_as_advanced(SUITESPARSE_INCLUDE_DIR SUITESPARSE_LIBRARY)

# if both headers and library are found, store results
if(SUITESPARSE_FOUND)
  set(SUITESPARSE_LIBRARIES ${SUITESPARSE_LIBRARY} )
  set(SUITESPARSE_INCLUDE_DIRS ${SUITESPARSE_INCLUDE_DIR} )
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determining location of SuiteSparse succeded:\n"
    "Include directory: ${SUITESPARSE_INCLUDE_DIRS}\n"
    "Library directory: ${SUITESPARSE_LIBRARIES}\n\n")
  set( SUITESPARSE_COMPILER_FLAGS )
  foreach(dir ${SUITESPARSE_INCLUDE_DIRS})
    set( SUITESPARSE_COMPILER_FLAGS "${SUITESPARSE_COMPILER_FLAGS} -I${dir}/" )
  endforeach()
  set(SUITESPARSE_DUNE_COMPILE_FLAGS ${SUITESPARSE_COMPILER_FLAGS}
    CACHE STRING "Compile Flags used by DUNE when compiling with SuiteSparse programs")
  set(SUITESPARSE_DUNE_LIBRARIES ${BLAS_LIBRARIES} ${SUITESPARSE_LIBRARIES}
    CACHE STRING "Libraries used by DUNE when linking SuiteSparse programs")
else(SUITESPARSE_FOUND)
  # log errornous result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKES_FILES_DIRECTORY}/CMakeError.log
    "Determing location of SuiteSparse failed:\n"
    "Include directory: ${SUITESPARSE_INCLUDE_DIRS}\n"
    "Library directory: ${SUITESPARSE_LIBRARIES}\n\n")
endif(SUITESPARSE_FOUND)

#set HAVE_SUITESPARSE for config.h
set(HAVE_SUITESPARSE ${SUITESPARSE_FOUND})

# register all SuiteSparse related flags
if(SUITESPARSE_FOUND)
  dune_register_package_flags(
    LIBRARIES "${SUITESPARSE_LIBRARIES}"
    INCLUDE_DIRS "${SUITESPARSE_INCLUDE_DIRS}")
endif()
