# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DunePathHelper
--------------

Helper commands to query path variables of Dune modules.

.. cmake:command:: dune_module_path

  Return a selected path associated with a Dune module.

  .. code-block:: cmake

    dune_module_path(
      MODULE <module>
      RESULT <result>
      [CMAKE_MODULES]
      [BUILD_DIR]
      [SOURCE_DIR]
      [SCRIPT_DIR]
    )

  ``MODULE``
    The name of the module whose path is requested.

  ``RESULT``
    The name of the output variable receiving the result.

  ``CMAKE_MODULES``
    Return the path to the module's CMake modules directory.

  ``BUILD_DIR``
    Return the path to the module's build directory.

  ``SOURCE_DIR``
    Return the path to the module's source directory.

  ``SCRIPT_DIR``
    Return the path to the module's CMake scripts directory.

  Exactly one path selector must be requested. The returned path depends on
  whether the command is called from the module itself or from a dependent
  module that already knows the exported path variables.

#]=======================================================================]
include_guard(GLOBAL)

function(dune_module_path)
  # Parse Arguments
  set(OPTION CMAKE_MODULES BUILD_DIR SOURCE_DIR SCRIPT_DIR)
  set(SINGLE MODULE RESULT)
  cmake_parse_arguments(PATH "${OPTION}" "${SINGLE}" "" ${ARGN})
  if(PATH_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_module_path: This often indicates typos!")
  endif()

  # Check whether one and only one path type was set.
  set(OPTION_FOUND 0)
  foreach(opt ${OPTION})
    if(${PATH_${opt}})
      if(OPTION_FOUND)
        message(FATAL_ERROR "Cannot request two different paths from dune_module_path")
      else()
        set(OPTION_FOUND 1)
      endif()
    endif()
  endforeach()
  if(NOT OPTION_FOUND)
    message(FATAL_ERROR "Cannot determine type of requested path!")
  endif()

  # Set the requested paths for the cmake module path
  if(PATH_CMAKE_MODULES)
    set(IF_CURRENT_MOD ${PROJECT_SOURCE_DIR}/cmake/modules)
    set(IF_NOT_CURRENT_MOD ${${PATH_MODULE}_MODULE_PATH})
  endif()

  # Set the requested paths for the cmake script path
  if(PATH_SCRIPT_DIR)
    set(IF_CURRENT_MOD ${PROJECT_SOURCE_DIR}/cmake/scripts)
    set(IF_NOT_CURRENT_MOD ${${PATH_MODULE}_SCRIPT_DIR})
  endif()

  # Set the requested paths for the build directory
  if(PATH_BUILD_DIR)
    set(IF_CURRENT_MOD ${PROJECT_BINARY_DIR})
    set(IF_NOT_CURRENT_MOD ${${PATH_MODULE}_DIR})
  endif()

  # Set the requested paths for the include directory
  if(PATH_SOURCE_DIR)
    set(IF_CURRENT_MOD ${PROJECT_SOURCE_DIR})
    set(IF_NOT_CURRENT_MOD ${${PATH_MODULE}_PREFIX})
  endif()

  # Now set the path in the outer scope!
  if(PROJECT_NAME STREQUAL ${PATH_MODULE})
    set(${PATH_RESULT} ${IF_CURRENT_MOD} PARENT_SCOPE)
  else()
    set(${PATH_RESULT} ${IF_NOT_CURRENT_MOD} PARENT_SCOPE)
  endif()
endfunction()
