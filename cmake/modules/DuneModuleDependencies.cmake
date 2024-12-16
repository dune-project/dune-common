# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneModuleDependencies
----------------------

Macros to extract dependencies between Dune modules by inspecting the
``dune.module`` files.

.. cmake:command:: dune_check_module_version

  .. code-block:: cmake

    dune_check_module_version(<dune-module> [REQUIRED] [QUIET] VERSION <version-requirement>)

  Check that the version of a dune module `<dune-module>` is compatible with
  `<version-requirement>`. Notice that the `dune_module_information` macro is
  invoked with the dune.module file of `<dune-module>`, thus, the variables for
  `<dune-module>_VERSION` are populated here.

.. cmake:command:: dune_cmake_path_setup

  .. code-block:: cmake

    dune_cmake_path_setup(<dependencies>)

  Set up the cmake module paths. The paths of all projects, including the
  current project, are added so that they follow the reverse order of the
  dependencies. This is an internal function of dune-common.

  ``dependencies``
  The list of dependencies of the project. The dependencies must be order
  as list resulting of a depth-first search of the dependency tree.

.. cmake:command:: dune_create_dependency_tree

  .. code-block:: cmake

    dune_create_dependency_tree([SKIP_CMAKE_PATH_SETUP])

  Create a tree of dune module dependencies by inspecting the ``dune.module``
  files recursively. All of the searched dependencies are stored in a list
  variable named ``ALL_DEPENDENCIES`` which is the result of a depth-first
  search on the dependency tree. If ``SKIP_CMAKE_PATH_SETUP`` is omitted,
  this function will additionally set up the cmake paths on
  ``ALL_DEPENDENCIES``. This is an internal function of dune-common and will be
  removed after version 2.12.

.. cmake:command:: dune_process_dependency_macros

  .. code-block:: cmake

    dune_process_dependency_macros()

  Include the corresponding ``Dune<module>Macros.cmake`` file of all
  dependencies if this file exists.

#]=======================================================================]
include_guard(GLOBAL)

include(DuneEnableAllPackages)
include(DuneModuleInformation)
include(DuneUtilities)

# checks that a module version is compatible with the found version of a module
# notice that this has the side effect of populating the ${module}_VERSION information
macro(dune_check_module_version module)
  cmake_parse_arguments(DUNE_VCHECK "REQUIRED;QUIET" "VERSION" "" ${ARGN})
  if(DUNE_VCHECK_QUIET)
    set(quiet QUIET)
  else()
    unset(quiet)
  endif()
  if(DUNE_VCHECK_REQUIRED)
    set(warning_level "FATAL_ERROR")
  else()
    if(DUNE_VCHECK_QUIET)
      set(warning_level "DEBUG")
    else()
      set(warning_level "WARNING")
    endif()
  endif()

  if(DUNE_VCHECK_VERSION MATCHES "(>=|=|<=).*")
    string(REGEX REPLACE "(>=|=|<=)(.*)" "\\1" DUNE_VCHECK_VERSION_OP ${DUNE_VCHECK_VERSION})
    string(REGEX REPLACE "(>=|=|<=)(.*)" "\\2" DUNE_VCHECK_VERSION_NUMBER ${DUNE_VCHECK_VERSION})
    string(STRIP ${DUNE_VCHECK_VERSION_NUMBER} DUNE_VCHECK_VERSION_NUMBER)
    extract_major_minor_version("${DUNE_VCHECK_VERSION_NUMBER}" DUNE_VCHECK_VERSION)
    set(DUNE_VCHECK_VERSION_STRING "${DUNE_VCHECK_VERSION_MAJOR}.${DUNE_VCHECK_VERSION_MINOR}.${DUNE_VCHECK_VERSION_REVISION}")
  else()
    set(DUNE_VCHECK_VERSION_STRING "0.0.0")
  endif()

  unset(${module}_dune_module)
  foreach(_dune_module_file
      ${${module}_PREFIX}/dune.module
      ${${module}_PREFIX}/lib/dunecontrol/${module}/dune.module
      ${${module}_PREFIX}/lib64/dunecontrol/${module}/dune.module)
    if(EXISTS ${_dune_module_file})
      get_filename_component(_dune_module_file_path ${_dune_module_file} PATH)
      dune_module_information(${_dune_module_file_path} ${quiet})
      set(${module}_dune_module 1)
      set(DUNE_VCHECK_MOD_VERSION_STRING "${DUNE_VERSION_MAJOR}.${DUNE_VERSION_MINOR}.${DUNE_VERSION_REVISION}")
      # check whether dependency matches version requirement
      unset(module_version_wrong)
      if(DUNE_VCHECK_VERSION_OP MATCHES ">=")
        if(NOT (DUNE_VCHECK_MOD_VERSION_STRING VERSION_EQUAL DUNE_VCHECK_VERSION_STRING OR
                DUNE_VCHECK_MOD_VERSION_STRING VERSION_GREATER DUNE_VCHECK_VERSION_STRING))
          set(module_version_wrong 1)
        endif()
      elseif(DUNE_VCHECK_VERSION_OP MATCHES "<=")
        if(NOT (DUNE_VCHECK_MOD_VERSION_STRING VERSION_EQUAL DUNE_VCHECK_VERSION_STRING OR
                DUNE_VCHECK_MOD_VERSION_STRING VERSION_LESS DUNE_VCHECK_VERSION_STRING))
          set(module_version_wrong 1)
        endif()
      elseif(DUNE_VCHECK_VERSION_OP MATCHES "=" AND
          NOT (DUNE_VCHECK_MOD_VERSION_STRING VERSION_EQUAL DUNE_VCHECK_VERSION_STRING))
        set(module_version_wrong 1)
      endif()
    endif()
  endforeach()
  if(NOT ${module}_dune_module)
    message(${warning_level} "Could not find dune.module file for module ${module} "
      "in ${${module}_PREFIX},  ${${module}_PREFIX}/lib/dunecontrol/${module}/, "
      "${${module}_PREFIX}/lib64/dunecontrol/${module}/dune.module")
    set(${module}_FOUND OFF)
  endif()
  if(module_version_wrong)
    message(${warning_level} "Could not find requested version of module ${module}. "
      "Requested version was ${DUNE_FIND_VERSION}, found version is ${DUNE_FIND_MOD_VERSION_STRING}")
    set(${module}_FOUND OFF)
  endif()
endmacro(dune_check_module_version module)

macro(dune_create_dependency_tree)
  cmake_parse_arguments(DUNE_DEPENDENCY_TREE "SKIP_CMAKE_PATH_SETUP" "" "" ${ARGN})
  if(DUNE_COMMON_VERSION VERSION_GREATER_EQUAL 2.12)
    message(DEPRECATION "The cmake function dune_create_dependency_tree() is deprecated and "
                        "will be removed after release 2.12. If you reached this warning, it "
                        "means that at least one of your dependencies was configured with a "
                        "dune-common version 2.9 or lower. To remove this warning, you could"
                        "either reconfigure all your dune dependencies with a newer version of "
                        "dune-common, or by downgrading your current dune-common version to "
                        "something below 2.13.")
  endif()
  set(ALL_DEPENDENCIES "")
  if(${ProjectName}_DEPENDS_MODULE OR ${ProjectName}_SUGGESTS_MODULE)
    set(ALL_DEPENDENCIES ${${ProjectName}_DEPENDS_MODULE} ${${ProjectName}_SUGGESTS_MODULE})
    dune_create_dependency_leafs("${${ProjectName}_DEPENDS_MODULE}" "${${ProjectName}_DEPENDS_VERSION}"
      "${${ProjectName}_SUGGESTS_MODULE}" "${${ProjectName}_SUGGESTS_VERSION}")
  endif()
  # Bring the list of found dependencies into the right order
  list(REVERSE ALL_DEPENDENCIES)
  # Remove possible duplicates
  list(REMOVE_DUPLICATES ALL_DEPENDENCIES)
  if (NOT ${DUNE_DEPENDENCY_TREE_SKIP_CMAKE_PATH_SETUP})
    dune_cmake_path_setup(ALL_DEPENDENCIES)
  endif()
endmacro(dune_create_dependency_tree)


macro(dune_process_dependency_macros)
  foreach(_mod ${DUNE_FOUND_DEPENDENCIES} ${ProjectName})
    if(NOT ${_mod}_PROCESSED)
      # module not processed yet
      set(${_mod}_PROCESSED ${_mod})
      # Search for a cmake files containing tests and directives
      # specific to this module
      dune_module_to_macro(_cmake_mod_name "${_mod}")
      set(_macro "${_cmake_mod_name}Macros")
      set(_mod_cmake _mod_cmake-NOTFOUND) # Prevent false positives due to caching
      find_file(_mod_cmake
        NAMES "${_macro}.cmake"
        PATHS ${CMAKE_MODULE_PATH}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH)
      if(_mod_cmake)
        message(STATUS "Performing tests for ${_mod} (from ${_mod_cmake})")
        include(${_mod_cmake})
      else()
        # only print output for found module dependencies
        # otherwise it's obvious that we cannot find a ModuleMacros.cmake file
        if (${${_mod}_FOUND})
          message(STATUS "No module specific tests for module '${_mod}' ('${_macro}.cmake' not found)")
        endif()
      endif()
      if(${_mod}_INCLUDE_DIRS)
        message(STATUS "Setting ${_mod}_INCLUDE_DIRS=${${_mod}_INCLUDE_DIRS}")
        include_directories("${${_mod}_INCLUDE_DIRS}")
      endif()
      if(DUNE_COMMON_VERSION VERSION_LESS 2.12)
        # Check if module is not added to DUNE_LIBS at find_package time (pre dune 2.10)
        # Remove this check after dune 2.12 when all pre 2.10 upstream config files are unsupported
        if(NOT ${_mod}_CMAKE_CONFIG_VERSION)
          if(${_mod}_LIBRARIES)
            message(STATUS "Setting ${_mod}_LIBRARIES=${${_mod}_LIBRARIES}")
            foreach(_lib ${${_mod}_LIBRARIES})
              list(INSERT DUNE_LIBS 0 "${_lib}")
            endforeach()
          endif()
        endif()
      else()
        message(AUTHOR_WARNING "This needs to be removed!")
      endif()

    endif()
  endforeach()
endmacro(dune_process_dependency_macros)


# ------------------------------------------------------------------------
# Internal macros
# ------------------------------------------------------------------------

macro(dune_cmake_path_setup project_list)
  # clean up module path from previeous modifications (a typical case is dune-common)
  foreach(_proj IN LISTS ${project_list})
    if(${_proj}_MODULE_PATH)
      list(REMOVE_ITEM CMAKE_MODULE_PATH "${${_proj}_MODULE_PATH}")
    endif()
  endforeach()

  # include current project cmake modules
  list(FIND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules" src_proj_index)
  if(src_proj_index EQUAL -1)
    list(LENGTH CMAKE_MODULE_PATH src_proj_index)
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
  endif()

  # start over again including them in reversed order so that newer modules have precedence
  foreach(_proj IN LISTS ${project_list})
    if(${_proj}_MODULE_PATH)
      list(INSERT CMAKE_MODULE_PATH ${src_proj_index} "${${_proj}_MODULE_PATH}")
    endif()
  endforeach()
endmacro(dune_cmake_path_setup)

macro(find_dune_package module)
  cmake_parse_arguments(DUNE_FIND "REQUIRED;QUIET" "VERSION" "" ${ARGN})
  if(DUNE_FIND_QUIET)
    set(quiet QUIET)
  else()
    unset(quiet)
  endif()
  if(DUNE_FIND_REQUIRED)
    set(required REQUIRED)
    set_package_properties(${module} PROPERTIES TYPE REQUIRED)
  else()
    unset(required)
    set_package_properties(${module} PROPERTIES TYPE OPTIONAL)
  endif()
  if(NOT ${module}_FOUND)
    if(NOT (${module}_DIR OR ${module}_ROOT OR
       "${CMAKE_PREFIX_PATH}" MATCHES ".*${module}.*"))
      string(REPLACE  ${ProjectName} ${module} ${module}_DIR
        ${PROJECT_BINARY_DIR})
    endif()
    find_package(${module} ${quiet})
  endif()
  if(${module}_FOUND)
    # parse other module's dune.module file to generate variables for config.h
    dune_check_module_version(${module} ${quiet} ${required} VERSION ${DUNE_FIND_VERSION})
  else(${module}_FOUND)
    if(required)
      message(FATAL_ERROR "Could not find required module ${module}.")
    endif()
  endif()
  set(DUNE_${module}_FOUND ${${module}_FOUND})
  dune_module_to_uppercase(_upper_case ${module})
  set(HAVE_${_upper_case} ${${module}_FOUND}) # need for example for the python meta data...
endmacro(find_dune_package module)


macro(dune_process_dependency_leafs modules versions is_required next_level_deps
    next_level_sugs)
  # TODO: Remove after dune 2.12
  # modules, and versions are not real variables, make them one
  set(mmodules ${modules})
  set(mversions ${versions})
  list(LENGTH mmodules mlength)
  list(LENGTH mversions vlength)
  if(NOT mlength EQUAL vlength)
    message(STATUS "mmodules=${mmodules} modules=${modules}")
    message(STATUS "mversions=${mversions} versions=${mversions}")
    message(FATAL_ERROR "List of modules and versions do not have the same length!")
  endif()
  if(mlength GREATER 0)
    math(EXPR length "${mlength}-1")
    foreach(i RANGE 0 ${length})
      list(GET mmodules ${i} _mod)
      list(GET mversions ${i} _ver)
      if(NOT ${_mod}_FOUND)
        find_dune_package(${_mod} ${is_required} QUIET VERSION "${_ver}")
      endif()
      set(${_mod}_SEARCHED ON)
      if(NOT "${is_required}" STREQUAL "")
        set(${_mod}_REQUIRED ON)
        set(${next_level_deps} ${${_mod}_DEPENDS} ${${next_level_deps}})
      else()
        set(${next_level_sugs} ${${_mod}_DEPENDS} ${${next_level_sugs}})
      endif()
      set(${next_level_sugs} ${${_mod}_SUGGESTS} ${${next_level_sugs}})
    endforeach()
  endif()
  if(${next_level_sugs})
    list(REMOVE_DUPLICATES ${next_level_sugs})
  endif()
  if(${next_level_deps})
    list(REMOVE_DUPLICATES ${next_level_deps})
  endif()
endmacro(dune_process_dependency_leafs)


function(remove_processed_modules modules versions is_required)
  message(DEPRECATION "The cmake function remove_processed_modules() is deprecated and "
                      "will be removed after release 2.9.")
  list(LENGTH ${modules} mlength)
  if(mlength GREATER 0)
    math(EXPR length "${mlength}-1")
    foreach(i RANGE ${length} 0 -1)
      list(GET ${modules} ${i} _mod)
      if(${_mod}_SEARCHED)
        list(REMOVE_AT ${modules} ${i})
        list(REMOVE_AT ${versions} ${i})
        if(is_required AND NOT ${_mod}_REQUIRED AND NOT ${_mod}_FOUND)
          message(FATAL_ERROR "Required module ${_mod} not found!")
        endif()
      endif()
    endforeach()
  endif()
  set(${modules} ${${modules}} PARENT_SCOPE)
  set(${versions} ${${versions}} PARENT_SCOPE)
endfunction(remove_processed_modules modules versions is_required)


macro(dune_create_dependency_leafs depends depends_versions suggests suggests_versions)
  # TODO: Remove after dune 2.12
  set(deps "")
  set(sugs "")
  #Process dependencies
  if(NOT "${depends}" STREQUAL "")
    dune_process_dependency_leafs("${depends}" "${depends_versions}" REQUIRED deps sugs)
  endif()
  # Process suggestions
  if(NOT "${suggests}" STREQUAL "")
    dune_process_dependency_leafs("${suggests}" "${suggests_versions}" "" deps sugs)
  endif()
  split_module_version("${deps}" next_mod_depends next_depends_versions)
  split_module_version("${sugs}" next_mod_suggests next_suggests_versions)
  set(ALL_DEPENDENCIES ${ALL_DEPENDENCIES} ${next_mod_depends} ${next_mod_suggests})
  # Move to next level
  if(next_mod_suggests OR next_mod_depends)
    dune_create_dependency_leafs("${next_mod_depends}" "${next_depends_versions}"
      "${next_mod_suggests}" "${next_suggests_versions}")
  endif()
endmacro(dune_create_dependency_leafs)
