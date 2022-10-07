# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# This cmake module provides infrastructure for building modules using Pybind11
#
# .. cmake_function:: dune_add_pybind11_module
#
#    .. cmake_param:: NAME
#       :required:
#       :single:
#
#       name of the Python module
#
#    .. cmake_param:: SOURCES
#       :multi:
#
#       source files to build shared library
#
#       If this parameter is omitted, <name>.cc will be used if it exists.
#
#    .. cmake_param:: EXCLUDE_FROM_ALL
#       :option:
#
#       exclude this module from the all target
#
#    .. cmake_param:: COMPILE_DEFINITIONS
#       :multi:
#       :argname: def
#
#       A set of compile definitions to add to the target.
#       Only definitions beyond the application of :ref:`add_dune_all_flags`
#       have to be stated.
#
#    .. cmake_param:: CMAKE_GUARD
#       :multi:
#       :argname: condition
#
#       A number of conditions that CMake should evaluate before adding this
#       module. Use this feature instead of guarding the call to
#       :code:`dune_add_pybind11_module` with an :code:`if` clause.
#
#       The passed condition can be a complex expression like
#       `( A OR B ) AND ( C OR D )`. Mind the spaces around the parentheses.
#
#       Example: Write CMAKE_GUARD dune-foo_FOUND if you want your module to only
#       build when the dune-foo module is present.
#
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
  # and add -undefined dynamic_lookup flag to linker
  # https://pybind11.readthedocs.io/en/stable/compiling.html#building-manually
  if (APPLE)
    set_target_properties(${PYBIND11_MODULE_NAME} PROPERTIES SUFFIX ".so")
    target_link_options(${PYBIND11_MODULE_NAME} PRIVATE -undefined dynamic_lookup)
  endif()

  target_compile_definitions(${PYBIND11_MODULE_NAME} PRIVATE ${PYBIND11_MODULE_COMPILE_DEFINITIONS})
  dune_target_enable_all_packages(${PYBIND11_MODULE_NAME})

  if(PYBIND11_MODULE_EXCLUDE_FROM_ALL)
    set_property(TARGET ${PYBIND11_MODULE_NAME} PROPERTY EXCLUDE_FROM_ALL 1)
  endif()
endfunction()
