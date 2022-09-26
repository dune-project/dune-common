# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Some helper functions for people developing the CMake build system
# to get quick and easy access to path variables of Dune modules.
#
# .. cmake_function:: dune_module_path
#
#    .. cmake_param:: MODULE
#       :single:
#       :required:
#
#       The name of the module.
#
#    .. cmake_param:: RESULT
#       :single:
#       :required:
#
#       The name of the variable to export the result.
#
#    .. cmake_param:: CMAKE_MODULES
#       :option:
#
#       Set to return the path to cmake modules
#
#    .. cmake_param:: BUILD_DIR
#       :option:
#
#       Set to return the path to the build directory
#
#    .. cmake_param:: SOURCE_DIR
#       :option:
#
#       Set to return the include path of the module
#
#    .. cmake_param:: SCRIPT_DIR
#       :option:
#
#       Set to return the CMake script dir
#
#
#    Returns the specified path of the given module. This differs
#    whether it is called from the actual module, or from a module
#    requiring or suggesting this module. One and only one type of path
#    may be requested.
#
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
