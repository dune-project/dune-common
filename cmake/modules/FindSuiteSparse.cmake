# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindSuiteSparse
---------------

Find the SuiteSparse libraries like UMFPACK or SPQR.

Use this module by invoking find_package with the form:

.. code-block:: cmake

    find_package(SuiteSparse
      [<version>] [EXACT]    # Minimum or EXACT version e.g. 5.1
      [REQUIRED]             # Fail with error if SuiteSparse is not found
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
``GraphBLAS``
  Graph algorithms and primitives using semiring algebra. (SuiteSparse >= 5.6)
``KLU`` and ``BTF``
  Sparse LU factorization, well-suited for circuit simulation.
``LDL``
  A sparse LDL' factorization and solve package.
``Mongoose``
  A graph partitioning library. (SuiteSparse >= 5.5)
``SPQR``
  Multifrontal QR factorization.
``UMFPACK``
  Multifrontal LU factorization.

And ordering methods: ``AMD``, ``CAMD``, ``COLAMD``, and ``CCOLAMD``.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SuiteSparse::SuiteSparse``
  A meta library including all the requested optional or required components.
``SuiteSparse::<COMPONENT>``
  Library and include directories for the found ``<COMPONENT>``.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SuiteSparse_FOUND``
  True if all the (required) components are found
``SuiteSparse_<COMPONENT>_FOUND``
  True if a searched ``<COMPONENT>`` is found

Input and Cache Variables
^^^^^^^^^^^^^^^^^^^^^^^^^

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
find_package(LAPACK QUIET)
find_package(BLAS QUIET)

# list of possible component names
set(SUITESPARSE_COMPONENTS
  "AMD" "BTF" "CAMD" "CCOLAMD" "CHOLMOD" "COLAMD"
  "CSparse" "CXSparse" "KLU" "LDL" "SPQR" "UMFPACK")

# Define required and optional component dependencies
set(SUITESPARSE_CHOLDMOD_REQUIRED_DEPENDENCIES "AMD" "COLAMD" "CCOLAMD")
set(SUITESPARSE_CHOLDMOD_REQUIRES_BLAS TRUE)
set(SUITESPARSE_CHOLDMOD_REQUIRES_LAPACK TRUE)
set(SUITESPARSE_KLU_REQUIRED_DEPENDENCIES "AMD" "COLAMD" "BTF")
set(SUITESPARSE_KLU_OPTIONAL_DEPENDENCIES "CHOLMOD" "CAMD" "CCOLAMD")
set(SUITESPARSE_SPQR_REQUIRED_DEPENDENCIES "CHOLMOD" "AMD" "COLAMD")
set(SUITESPARSE_SPQR_REQUIRES_BLAS TRUE)
set(SUITESPARSE_SPQR_REQUIRES_LAPACK TRUE)
set(SUITESPARSE_UMFPACK_REQUIRED_DEPENDENCIES "AMD")
set(SUITESPARSE_UMFPACK_OPTIONAL_DEPENDENCIES "CHOLMOD" "CAMD" "CCOLAMD" "COLAMD")
set(SUITESPARSE_UMFPACK_REQUIRES_BLAS TRUE)

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

# Look for the header files that have different header file names
find_path(SPQR_INCLUDE_DIR "SuiteSparseQR.hpp"
  HINTS ${SUITESPARSE_INCLUDE_DIR}
  PATH_SUFFIXES "suitesparse" "include" "SPQR/Include"
)
find_path(Mongoose_INCLUDE_DIR "Mongoose.hpp"
  HINTS ${SUITESPARSE_INCLUDE_DIR}
  PATH_SUFFIXES "suitesparse" "include" "Mongoose/Include"
)
find_path(GraphBLAS_INCLUDE_DIR "GraphBLAS.h"
  HINTS ${SUITESPARSE_INCLUDE_DIR}
  PATH_SUFFIXES "suitesparse" "include" "GraphBLAS/Include"
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


# check whether everything was found
foreach(_component ${SUITESPARSE_COMPONENTS})
  if(${_component}_LIBRARY AND ${_component}_INCLUDE_DIR)
    set(SuiteSparse_${_component}_FOUND TRUE)
  else()
    set(SuiteSparse_${_component}_FOUND FALSE)
  endif()
endforeach(_component)

# test for required dependencies
foreach(_component ${SUITESPARSE_COMPONENTS})
  foreach(_dependency ${SUITESPARSE_${_component}_REQUIRED_DEPENDENCIES})
    if(NOT SuiteSparse_${_dependency}_FOUND)
      set(SuiteSparse_${_component}_FOUND FALSE)
    endif()
  endforeach(_dependency)
endforeach(_component)

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

# if both headers and library for all required components are found,
# then create imported targets for all components
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

  foreach(_component ${SUITESPARSE_COMPONENTS})
    # Link required dependencies
    foreach(_dependency ${SUITESPARSE_${_component}_REQUIRED_DEPENDENCIES})
      target_link_libraries(SuiteSparse::${_component}
        INTERFACE SuiteSparse::${_dependency})
    endforeach(_dependency)

    # Link found optional dependencies
    foreach(_dependency ${SUITESPARSE_${_component}_OPTIONAL_DEPENDENCIES})
      if(SuiteSparse_${_dependency}_FOUND)
        target_link_libraries(SuiteSparse::${_component}
          INTERFACE SuiteSparse::${_dependency})
      endif()
    endforeach(_dependency)

    # Link BLAS library
    if(SUITESPARSE_${_component}_REQUIRES_BLAS)
      if(TARGET BLAS::BLAS)
        target_link_libraries(SuiteSparse::${_component}
          INTERFACE BLAS::BLAS)
      else()
        target_link_libraries(SuiteSparse::${_component}
          INTERFACE ${BLAS_LINKER_FLAGS} ${BLAS_LIBRARIES})
      endif()
    endif()

    # Link LAPACK library
    if(SUITESPARSE_${_component}_REQUIRES_LAPACK)
      if(TARGET LAPACK::LAPACK)
        target_link_libraries(SuiteSparse::${_component}
          INTERFACE LAPACK::LAPACK)
      else()
        target_link_libraries(SuiteSparse::${_component}
          INTERFACE ${LAPACK_LINKER_FLAGS} ${LAPACK_LIBRARIES})
      endif()
    endif()
  endforeach(_component)

  # Combine all requested components to an imported target
  if(NOT TARGET SuiteSparse::SuiteSparse)
    add_library(SuiteSparse::SuiteSparse INTERFACE IMPORTED)
    target_link_libraries(SuiteSparse::SuiteSparse
      INTERFACE SuiteSparse::SuiteSparse_config)
  endif()
  foreach(_component ${SuiteSparse_FIND_COMPONENTS})
    if(SuiteSparse_${_component}_FOUND)
      set(HAVE_SUITESPARSE_${_component} TRUE)
      target_link_libraries(SuiteSparse::SuiteSparse
        INTERFACE SuiteSparse::${_component})
    endif()
  endforeach(_component)
endif()
