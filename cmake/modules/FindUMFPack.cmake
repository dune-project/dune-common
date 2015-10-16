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

include( FindSuiteSparse )
find_package( SuiteSparse COMPONENTS umfpack )

if( SUITESPARSE_FOUND )
  set( UMFPACK_FOUND TRUE )
  set( UMFPACK_INCLUDE_DIR ${SUITESPARSE_INCLUDE_DIR} )
  set( UMFPACK_LIBRARY ${SUITESPARSE_LIBRARY} )
endif()

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
  foreach( dir ${UMFPACK_INCLUDE_DIR} )
    list( APPEND UMFPACK_INCLUDE_FLAGS "-I${dir}/ " )
  endforeach()
  set(UMFPACK_LIBRARIES ${UMFPACK_LIBRARY})
  # log result
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Determining location of UMFPack succeded:\n"
    "Include directory: ${UMFPACK_INCLUDE_DIRS}\n"
    "Library directory: ${UMFPACK_LIBRARIES}\n\n")
  set(UMFPACK_DUNE_COMPILE_FLAGS "${UMFPACK_INCLUDE_FLAGS}"
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
set(HAVE_UMFPACK ${UMFPACK_FOUND})
set(HAVE_SUITESPARSE_UMFPACK ${UMFPACK_FOUND})

# register all umfpack related flags
if(UMFPACK_FOUND)
  dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_UMFPACK=1"
                            LIBRARIES "${UMFPACK_LIBRARIES}"
                            INCLUDE_DIRS "${UMFPACK_INCLUDE_DIRS}")
endif()
