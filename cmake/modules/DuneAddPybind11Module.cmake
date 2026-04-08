# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneAddPybind11Module
=====================

Infrastructure for building Python extension modules with pybind11-style
bindings.

.. cmake:command:: dune_add_pybind11_module

  Add a Python extension module as a shared library target.

  .. code-block:: cmake

    dune_add_pybind11_module(
      NAME <module-name>
      [SOURCES <sources>...]
      [EXCLUDE_FROM_ALL]
      [COMPILE_DEFINITIONS <definitions>...]
      [CMAKE_GUARD <condition>...]
    )

  ``NAME``
    Name of the Python extension module target.

  ``SOURCES``
    Source files used to build the shared library. If omitted,
    ``<NAME>.cc`` is used when that file exists in the current source
    directory.

  ``EXCLUDE_FROM_ALL``
    Exclude the generated target from the default ``all`` target.

  ``COMPILE_DEFINITIONS``
    Additional compile definitions passed to
    :dune:cmake-command:`target_compile_definitions`.

  ``CMAKE_GUARD``
    Conditions evaluated before the module target is added. If one of the
    conditions evaluates to false, the target is skipped.

  The generated target links against ``Dune::Common`` and ``Python3::Module``
  and is configured with :cmake:command:`dune_target_enable_all_packages()`.

.. cmake:command:: dune_add_pybind11_submodule

  Add an object-library submodule and link it into an existing Python module.

  .. code-block:: cmake

    dune_add_pybind11_submodule(
      MODULE <parent-module>
      NAME <submodule-name>
      SOURCES <sources>...
      [EXCLUDE_FROM_ALL]
      [COMPILE_DEFINITIONS <definitions>...]
      [CMAKE_GUARD <condition>...]
    )

  ``MODULE``
    Name of the previously created parent module target that receives the
    submodule object library.

  ``NAME``
    Name of the object-library target providing the submodule sources.

  ``SOURCES``
    Source files compiled into the object library.

  ``EXCLUDE_FROM_ALL``
    Exclude the object-library target from the default ``all`` target.

  ``COMPILE_DEFINITIONS``
    Additional compile definitions passed to
    :dune:cmake-command:`target_compile_definitions`.

  ``CMAKE_GUARD``
    Conditions evaluated before the submodule target is added. If one of the
    conditions evaluates to false, the target is skipped.

  The generated object library links against ``Dune::Common``, is configured
  with :cmake:command:`dune_target_enable_all_packages()`, and is linked into
  the parent module target specified by ``MODULE``.

#]=======================================================================]
include_guard(GLOBAL)

function(dune_add_pybind11_module)
  cmake_parse_arguments(PYBIND11_MODULE "EXCLUDE_FROM_ALL" "NAME" "SOURCES;COMPILE_DEFINITIONS;CMAKE_GUARD" ${ARGN})
  if(PYBIND11_MODULE_UNPARSED_ARGUMENTS)
    message(WARNING "dune_add_pybind11_module: extra arguments provided (typos in named arguments?)")
  endif()

  if(NOT PYBIND11_MODULE_NAME)
    message(FATAL_ERROR "dune_add_pybind11_module: module name not specified")
  endif()

  if(NOT PYBIND11_MODULE_SOURCES)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PYBIND11_MODULE_NAME}.cc)
      set(PYBIND11_MODULE_SOURCES ${PYBIND11_MODULE_NAME}.cc)
    else()
      message(FATAL_ERROR "dune_add_pybind11_module: no source files specified")
    endif()
  endif()

  foreach(condition ${PYBIND11_MODULE_CMAKE_GUARD})
    separate_arguments(condition)
    if(NOT (${condition}))
      message(STATUS "not building ${PYBIND11_MODULE_NAME}, because condition ${condition} failed.")
      return()
    endif()
  endforeach()

  add_library(${PYBIND11_MODULE_NAME} SHARED ${PYBIND11_MODULE_SOURCES})
  set_target_properties(${PYBIND11_MODULE_NAME} PROPERTIES PREFIX "")

  # force '.so' as library suffix on macOS due to a problem in Python
  # https://stackoverflow.com/questions/2488016/how-to-make-python-load-dylib-on-osx
  if (APPLE)
    set_target_properties(${PYBIND11_MODULE_NAME} PROPERTIES SUFFIX ".so")
  endif()

  target_compile_definitions(${PYBIND11_MODULE_NAME} PRIVATE ${PYBIND11_MODULE_COMPILE_DEFINITIONS})
  target_link_libraries(${PYBIND11_MODULE_NAME} PUBLIC Dune::Common Python3::Module)
  dune_target_enable_all_packages(${PYBIND11_MODULE_NAME})

  if(PYBIND11_MODULE_EXCLUDE_FROM_ALL)
    set_property(TARGET ${PYBIND11_MODULE_NAME} PROPERTY EXCLUDE_FROM_ALL 1)
  endif()
endfunction()


## add a submodule for a pybind11 module
function(dune_add_pybind11_submodule)
  cmake_parse_arguments(PYBIND11_SUBMODULE "EXCLUDE_FROM_ALL" "MODULE;NAME" "SOURCES;COMPILE_DEFINITIONS;CMAKE_GUARD" ${ARGN})
  if(PYBIND11_SUBMODULE_UNPARSED_ARGUMENTS)
    message(WARNING "dune_add_pybind11_submodule: extra arguments provided (typos in named arguments?)")
  endif()

  if(NOT PYBIND11_SUBMODULE_NAME)
    message(FATAL_ERROR "dune_add_pybind11_submodule: module name not specified")
  endif()

  if(NOT PYBIND11_SUBMODULE_MODULE)
    message(FATAL_ERROR "dune_add_pybind11_submodule: parent module not specified")
  endif()

  if(NOT PYBIND11_SUBMODULE_SOURCES)
    message(FATAL_ERROR "dune_add_pybind11_submodule: sources not specified")
  endif()

  foreach(condition ${PYBIND11_SUBMODULE_CMAKE_GUARD})
    separate_arguments(condition)
    if(NOT (${condition}))
      message(STATUS "not building ${PYBIND11_SUBMODULE_NAME}, because condition ${condition} failed.")
      return()
    endif()
  endforeach()

  add_library(${PYBIND11_SUBMODULE_NAME} OBJECT ${PYBIND11_SUBMODULE_SOURCES})
  # set compile definitions
  target_compile_definitions(${PYBIND11_SUBMODULE_NAME} PRIVATE ${PYBIND11_SUBMODULE_COMPILE_DEFINITIONS})
  # add all package flags
  dune_target_enable_all_packages(${PYBIND11_SUBMODULE_NAME})
  # Link against Dune::Common
  target_link_libraries(${PYBIND11_SUBMODULE_NAME} PRIVATE Dune::Common)

  # link object file to parent module
  target_link_libraries(${PYBIND11_SUBMODULE_MODULE} PUBLIC ${PYBIND11_SUBMODULE_NAME})

  if(PYBIND11_MODULE_EXCLUDE_FROM_ALL)
    set_property(TARGET ${PYBIND11_SUBMODULE_NAME} PROPERTY EXCLUDE_FROM_ALL 1)
  endif()
endfunction()
