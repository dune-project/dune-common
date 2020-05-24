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

``CHOLMOD``
  Supernodal Cholesky factorization.
``CSparse`` and ``CXSparse``
  A Concise Sparse Matrix package.
``KLU`` and ``BTF``
  Sparse LU factorization, well-suited for circuit simulation.
``LDL``
  A sparse LDL' factorization and solve package.
``SPQR``
  Multifrontal QR factorization.
``UMFPACK``
  Multifrontal LU factorization.

And ordering methods: ``AMD``, ``CAMD``, ``COLAMD``, and ``CCOLAMD``.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SuiteSparse::SuiteSparse``
  A meta library including all the found components.
``SuiteSparse::<component>``
  Library and include directories for the found ``<component>``.
  Spelling of the component is always lowercase.

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

``SuiteSparse_ROOT``
  The root directory of the SuiteSparse installation, containing
  subdirectories :code:`include/` and :code:`lib/` including the
  header files and libraries of SuiteSparse and its components,
  respectively.
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
include(CMakeFindDependencyMacro)
find_dependency(BLAS)

# list of possible component names
set(SUITESPARSE_COMPONENTS
  "AMD" "BTF" "CAMD" "CCOLAMD" "CHOLMOD" "COLAMD"
  "CSparse" "CXSparse" "KLU" "LDL" "SPQR" "UMFPACK")

# look for library suitesparseconfig
find_library(SUITESPARSE_CONFIG_LIB "suitesparseconfig"
  PATH_SUFFIXES "SuiteSparse_config"
)
# look for header file SuiteSparse_config.h
find_path(SUITESPARSE_INCLUDE_DIR "SuiteSparse_config.h"
  PATH_SUFFIXES "suitesparse" "include" "SuiteSparse_config"
)

get_filename_component(SUITESPARSE_LIB_DIR ${SUITESPARSE_CONFIG_LIB} DIRECTORY)
mark_as_advanced(SUITESPARSE_INCLUDE_DIR SUITESPARSE_CONFIG_LIB)

foreach(_component ${SUITESPARSE_COMPONENTS})
  string(TOLOWER ${_component} _componentLower)

  # look for library of the component
  find_library(${_component}_LIBRARY "${_componentLower}"
    HINTS ${SUITESPARSE_LIB_DIR}
    PATH_SUFFIXES "${_component}/Lib"
  )
  # look for header file of the component
  find_path(${_component}_INCLUDE_DIR "${_componentLower}.h"
    HINTS ${SUITESPARSE_INCLUDE_DIR}
    PATH_SUFFIXES "suitesparse" "include" "${_component}/Include"
  )

  mark_as_advanced(${_component}_INCLUDE_DIR ${_component}_LIBRARY)
endforeach()

# Look for the header file of SPQR the has different header file name SuiteSparseQR.hpp
find_path(SPQR_INCLUDE_DIR "SuiteSparseQR.hpp"
  HINTS ${SUITESPARSE_INCLUDE_DIR}
  PATH_SUFFIXES "suitesparse" "include" "SPQR/Include"
)

# check version of SuiteSparse
find_file(SUITESPARSE_CONFIG_FILE "SuiteSparse_config.h"
  HINTS ${SUITESPARSE_INCLUDE_DIR}
  NO_DEFAULT_PATH)
if(SUITESPARSE_CONFIG_FILE)
  file(READ "${SUITESPARSE_CONFIG_FILE}" suitesparseconfig)
  string(REGEX REPLACE ".*#define SUITESPARSE_MAIN_VERSION[ ]+([0-9]+).*" "\\1"
    SUITESPARSE_MAJOR_VERSION  "${suitesparseconfig}")
  string(REGEX REPLACE ".*#define SUITESPARSE_SUB_VERSION[ ]+([0-9]+).*" "\\1"
    SUITESPARSE_MINOR_VERSION  "${suitesparseconfig}")
  string(REGEX REPLACE ".*#define SUITESPARSE_SUBSUB_VERSION[ ]+([0-9]+).*" "\\1"
    SUITESPARSE_PREFIX_VERSION "${suitesparseconfig}")
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
  if(${_component}_LIBRARY AND ${_component}_INCLUDE_DIR)
    set(SuiteSparse_${_component}_FOUND TRUE)
  else()
    set(SuiteSparse_${_component}_FOUND FALSE)
  endif()
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

    # Link against BLAS
    if(TARGET BLAS::BLAS)
      target_link_libraries(SuiteSparse::SuiteSparse_config
        INTERFACE BLAS::BLAS)
    else()
      target_link_libraries(SuiteSparse::SuiteSparse_config
        INTERFACE ${BLAS_LINKER_FLAGS} ${BLAS_LIBRARIES})
    endif()
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

  # Dependencies for component CHOLMOD
  if(SuiteSparse_CHOLMOD_FOUND)
    target_link_libraries(SuiteSparse::CHOLMOD INTERFACE
      SuiteSparse::AMD
      SuiteSparse::COLAMD
      $<$<BOOL:${SuiteSparse_CAMD_FOUND}>:SuiteSparse::CAMD>
      $<$<BOOL:${SuiteSparse_CCOLAMD_FOUND}>:SuiteSparse::CCOLAMD>
    )
  endif()

  # Dependencies for component UMFPACK
  if(SuiteSparse_UMFPACK_FOUND)
    target_link_libraries(SuiteSparse::UMFPACK INTERFACE
      $<IF:$<BOOL:${SuiteSparse_CHOLMOD_FOUND}>,SuiteSparse::CHOLMOD,SuiteSparse::AMD>
    )
  endif()

  # Combine all SuiteSparse components to all-target
  if(NOT TARGET SuiteSparse::SuiteSparse)
    add_library(SuiteSparse::SuiteSparse INTERFACE IMPORTED)
    set_target_properties(SuiteSparse::SuiteSparse PROPERTIES
      INTERFACE_LINK_LIBRARIES SuiteSparse::SuiteSparse_config
    )
  endif()
  foreach(_component ${SUITESPARSE_COMPONENTS})
    if(SuiteSparse_${_component}_FOUND)
      target_link_libraries(SuiteSparse::SuiteSparse
        INTERFACE SuiteSparse::${_component})
    endif()
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
