#[=======================================================================[.rst:
FindSuiteSparse
---------------

Find the SuiteSparse libraries like UMFPACK or SPQR.

Use this module by invoking find_package with the form:

  find_package(SuiteSparse
    [<version>] [EXACT]    # Minimum or EXACT version e.g. 5.1
    [REQUIRED]             # Fail with error if Boost is not found
    [COMPONENTS <libs>...] # SuiteSparse libraries by their canonical name
                           # e.g. "UMFPACK" or "SPQR"
    [OPTIONAL_COMPONENTS <libs>...]
                           # Optional SuiteSparse libraries by their canonical name
    )                      # e.g. "UMFPACK" or "SPQR"

Components
^^^^^^^^^^

The SuiteSparse module allows to search for the following components

``UMFPACK``
  Multifrontal LU factorization.
``CHOLMOD``
  Supernodal Cholesky factorization.
``SPQR``
  Multifrontal QR factorization.
``KLU`` or ``BTF``
  Sparse LU factorization, well-suited for circuit simulation.

And ordering methods: ``AMD``, ``CAMD``, ``COLAMD``, and ``CCOLAMD``.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SuiteSparse::SuiteSparse``
  A meta library including all the found components.
``SuiteSparse::<COMPONENT>``
  Library and include directories for the found ``<COMPONENT>``.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SuiteSparse_FOUND``
  True if all the components are found
``SuiteSparse_<COMPONENT>_FOUND``
  True if a searched ``<COMPONENT>`` is found

Cache Variables
^^^^^^^^^^^^^^^

You may set the following variables to modify the behaviour of
this module:

``SUITESPARSE_INCLUDE_DIR``
  The directory containing ``SuiteSparse_config.h``.
``SUITESPARSE_CONFIG_LIB``
  The path to the suitesparseconfig library.

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("SuiteSparse" PROPERTIES
  DESCRIPTION "A suite of sparse matrix software"
  URL "http://faculty.cse.tamu.edu/davis/suitesparse.html"
)

# find package dependencies first
find_package(BLAS)

# look for desired componenents
set(SUITESPARSE_COMPONENTS ${SuiteSparse_FIND_COMPONENTS})

# resolve inter-component dependencies
list(FIND SUITESPARSE_COMPONENTS "UMFPACK" WILL_USE_UMFPACK)
if(NOT WILL_USE_UMFPACK EQUAL -1)
  list(APPEND SUITESPARSE_COMPONENTS AMD CHOLMOD)
endif()
list(FIND SUITESPARSE_COMPONENTS "CHOLMOD" WILL_USE_CHOLMOD)
if(NOT WILL_USE_CHOLMOD EQUAL -1)
  list(APPEND SUITESPARSE_COMPONENTS AMD CAMD COLAMD CCOLAMD)
endif()

if(SUITESPARSE_COMPONENTS)
  list(REMOVE_DUPLICATES SUITESPARSE_COMPONENTS)
endif()

# find SuiteSparse config:
# look for library at positions given by the user
find_library(SUITESPARSE_CONFIG_LIB "suitesparseconfig"
  PATHS ${SuiteSparse_ROOT}
  PATH_SUFFIXES "Lib"
  NO_DEFAULT_PATH
)
# now also include the default paths
find_library(SUITESPARSE_CONFIG_LIB "suitesparseconfig"
  PATH_SUFFIXES "Lib"
)

# look for header files at positions given by the user
find_path(SUITESPARSE_INCLUDE_DIR "SuiteSparse_config.h"
  PATHS ${SuiteSparse_ROOT}
  PATH_SUFFIXES "SuiteSparse_config" "SuiteSparse_config/include" "suitesparse" "include" "src" "SuiteSparse_config/Include"
  NO_DEFAULT_PATH
)
# now also look for default paths
find_path(SUITESPARSE_INCLUDE_DIR "SuiteSparse_config.h"
  PATH_SUFFIXES "SuiteSparse_config" "SuiteSparse_config/include" "suitesparse" "include" "src" "SuiteSparse_config/Include"
)

mark_as_advanced(SUITESPARSE_INCLUDE_DIR SUITESPARSE_CONFIG_LIB)

foreach(_component ${SUITESPARSE_COMPONENTS})
  string(TOLOWER ${_component} _componentLower)

  # look for library at positions given by the user
  find_library(${_component}_LIBRARY "${_componentLower}"
    PATHS ${SuiteSparse_ROOT}
    PATH_SUFFIXES "${_component}" "${_component}/Lib"
    NO_DEFAULT_PATH
  )
  # now also include the default paths
  find_library(${_component}_LIBRARY "${_componentLower}"
    PATH_SUFFIXES "${_component}" "${_component}/Lib"
  )

  # look for header files at positions given by the user
  find_path(${_component}_INCLUDE_DIR "${_componentLower}.h"
    PATHS ${SuiteSparse_ROOT}
    PATH_SUFFIXES "${_componentLower}" "include/${_componentLower}" "suitesparse" "include" "src" "${_component}" "${_component}/Include"
    NO_DEFAULT_PATH
  )
  # now also look for default paths
  find_path(${_component}_INCLUDE_DIR "${_componentLower}.h"
    PATH_SUFFIXES "${_componentLower}" "include/${_componentLower}" "suitesparse" "include" "${_component}" "${_component}/Include"
  )
endforeach()

# SPQR has different header file name SuiteSparseQR.hpp
# look for header files at positions given by the user
find_path(SPQR_INCLUDE_DIR "SuiteSparseQR.hpp"
  PATHS ${SuiteSparse_ROOT}
  PATH_SUFFIXES "spqr" "include/spqr" "suitesparse" "include" "src" "SPQR" "SPQR/Include"
  NO_DEFAULT_PATH
)
# now also look for default paths
find_path(SPQR_INCLUDE_DIR "SuiteSparseQR.hpp"
  PATH_SUFFIXES "spqr" "include/spqr" "suitesparse" "include" "SPQR" "SPQR/Include"
)

# check version, for SuiteSparse
find_file(SUITESPARSE_CONFIG_FILE "SuiteSparse_config.h"
  PATHS ${SUITESPARSE_INCLUDE_DIR}
  NO_DEFAULT_PATH)
if(SUITESPARSE_CONFIG_FILE)
  file(READ "${SUITESPARSE_CONFIG_FILE}" suitesparseconfig)
  string(REGEX REPLACE ".*#define SUITESPARSE_MAIN_VERSION[ ]+([0-9]+).*" "\\1"   SUITESPARSE_MAJOR_VERSION  "${suitesparseconfig}")
  string(REGEX REPLACE ".*#define SUITESPARSE_SUB_VERSION[ ]+([0-9]+).*" "\\1"    SUITESPARSE_MINOR_VERSION  "${suitesparseconfig}")
  string(REGEX REPLACE ".*#define SUITESPARSE_SUBSUB_VERSION[ ]+([0-9]+).*" "\\1" SUITESPARSE_PREFIX_VERSION "${suitesparseconfig}")
  if(SUITESPARSE_MAJOR_VERSION GREATER_EQUAL 0)
    set(SuiteSparse_VERSION "${SUITESPARSE_MAJOR_VERSION}")
  endif()
  if (SUITESPARSE_MINOR_VERSION GREATER_EQUAL 0)
    set(SuiteSparse_VERSION "${SuiteSparse_VERSION}.${SUITESPARSE_MINOR_VERSION}")
  endif()
  if (SUITESPARSE_PREFIX_VERSION GREATER_EQUAL 0)
    set(SuiteSparse_VERSION "${SuiteSparse_VERSION}.${SUITESPARSE_PREFIX_VERSION}")
  endif()
endif()
unset(SUITESPARSE_CONFIG_FILE CACHE)


# check wether everything was found
foreach(_component ${SUITESPARSE_COMPONENTS})
  # variable used for component handling
  if(${_component}_LIBRARY AND ${_component}_INCLUDE_DIR)
    set(SuiteSparse_${_component}_FOUND TRUE)
  else()
    set(SuiteSparse_${_component}_FOUND FALSE)
  endif()

  set(HAVE_SUITESPARSE_${_component} ${SuiteSparse_${_component}_FOUND})

  mark_as_advanced(${_component}_INCLUDE_DIR ${_component}_LIBRARY)
endforeach()

# CHOLMOD requires AMD, COLAMD; CAMD and CCOLAMD are optional
if(CHOLMOD_LIBRARY)
  if(NOT (AMD_LIBRARY AND COLAMD_LIBRARY))
    set(SuiteSparse_CHOLMOD_FOUND FALSE)
  endif()
endif()

# UMFPack requires AMD, can depend on CHOLMOD
if(UMFPACK_LIBRARY)
  if(NOT (AMD_LIBRARY OR CHOLMOD_LIBRARY))
    set(SuiteSparse_UMFPACK_FOUND FALSE)
  endif()
endif()

# SPQR requires SuiteSparse >= 4.3
if(SPQR_LIBRARY)
  if(SuiteSparse_VERSION VERSION_LESS "4.3")
    set(SuiteSparse_SPQR_FOUND FALSE)
  endif()
endif()


# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("SuiteSparse"
  REQUIRED_VARS
    SUITESPARSE_CONFIG_LIB SUITESPARSE_INCLUDE_DIR BLAS_FOUND
  VERSION_VAR
    SuiteSparse_VERSION
  HANDLE_COMPONENTS
)


# if both headers and library are found, store results
if(SuiteSparse_FOUND)
  if(NOT TARGET SuiteSparse::SuiteSparse_config)
    add_library(SuiteSparse::SuiteSparse_config UNKNOWN IMPORTED)
    set_target_properties(SuiteSparse::SuiteSparse_config PROPERTIES
      IMPORTED_LOCATION ${SUITESPARSE_CONFIG_LIB}
      INTERFACE_INCLUDE_DIRECTORIES ${SUITESPARSE_INCLUDE_DIR}
    )
  endif()

  # Define component imported-targets
  foreach(_component ${SUITESPARSE_COMPONENTS})
    if(SuiteSparse_${_component}_FOUND AND NOT TARGET SuiteSparse::${_component})
      add_library(SuiteSparse::${_component} UNKNOWN IMPORTED)
      set_target_properties(SuiteSparse::${_component} PROPERTIES
        IMPORTED_LOCATION ${${_component}_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${${_component}_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES SuiteSparse::SuiteSparse_config
      )
    endif()
  endforeach(_component)

  # dependencies for component CHOLMOD
  if(SuiteSparse_CHOLMOD_FOUND)
    target_link_libraries(SuiteSparse::CHOLMOD INTERFACE
      SuiteSparse::AMD
      SuiteSparse::COLAMD
      $<$<BOOL:${SuiteSparse_COLAMD_FOUND}>:SuiteSparse::CAMD>
      $<$<BOOL:${SuiteSparse_CCOLAMD_FOUND}>:SuiteSparse::CCOLAMD>
    )
  endif()

  # dependencies for component UMFPACK
  if(SuiteSparse_UMFPACK_FOUND)
    target_link_libraries(SuiteSparse::UMFPACK INTERFACE
      $<IF:$<BOOL:${SuiteSparse_CHOLMOD_FOUND}>,SuiteSparse::CHOLMOD,SuiteSparse::AMD>
    )
  endif()

  # Combine all SuiteSparse components to all-target
  if(NOT TARGET SuiteSparse::SuiteSparse)
    add_library(SuiteSparse::SuiteSparse INTERFACE IMPORTED)
  endif()
  foreach(_component ${SUITESPARSE_COMPONENTS})
    target_link_libraries(SuiteSparse::SuiteSparse INTERFACE SuiteSparse::${_component})
  endforeach(_component)
endif()

# set HAVE_SUITESPARSE for config.h
set(HAVE_SUITESPARSE ${SuiteSparse_FOUND})
set(HAVE_UMFPACK ${SuiteSparse_UMFPACK_FOUND})

# register all SuiteSparse related flags
if(SuiteSparse_FOUND)
  dune_register_package_flags(
    COMPILE_DEFINITIONS "ENABLE_SUITESPARSE=1"
    LIBRARIES "SuiteSparse::SuiteSparse")
endif()
