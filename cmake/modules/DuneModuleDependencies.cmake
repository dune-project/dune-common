# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneModuleDependencies
----------------------

Macros to extract dependencies between Dune modules by inspecting the
``dune.module`` files.

.. cmake:command:: dune_create_dependency_tree

  .. code-block:: cmake

    dune_create_dependency_tree()

  Create a tree of dune module dependencies by inspecting the ``dune.module``
  files recursively.


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


macro(dune_create_dependency_tree)
  if(dune-common_MODULE_PATH)
    list(REMOVE_ITEM CMAKE_MODULE_PATH "${dune-common_MODULE_PATH}")
  endif()
  list(FIND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules start)
  set(ALL_DEPENDENCIES "")
  if(${ProjectName}_DEPENDS_MODULE OR ${ProjectName}_SUGGESTS_MODULE)
    set(ALL_DEPENDENCIES ${${ProjectName}_DEPENDS_MODULE} ${${ProjectName}_SUGGESTS_MODULE})
    dune_create_dependency_leafs("${${ProjectName}_DEPENDS_MODULE}" "${${ProjectName}_DEPENDS_VERSION}"
      "${${ProjectName}_SUGGESTS_MODULE}" "${${ProjectName}_SUGGESTS_VERSION}")
  endif()
  set(_my_path "")
  if(ALL_DEPENDENCIES)
    # Reverse the order of the modules and remove duplicates
    # At end of this clause we have have a list modules
    # where for each entry all dependencies are before the
    # module in the list.
    set(NEW_ALL_DEPS "")
    list(LENGTH ALL_DEPENDENCIES length)
    if(length GREATER 0)
      math(EXPR length "${length}-1")
      list(GET ALL_DEPENDENCIES ${length} _mod)
      set(${_mod}_cmake_path_processed 1)
      set(_my_path ${${_mod}_MODULE_PATH})
      list(APPEND NEW_ALL_DEPS ${_mod})
      if(length GREATER 0)
        math(EXPR length "${length}-1")
        foreach(i RANGE ${length} 0 -1)
          list(GET ALL_DEPENDENCIES ${i} _mod)
          if(NOT ${_mod}_cmake_path_processed)
            set(${_mod}_cmake_path_processed 1)
            if(${_mod}_MODULE_PATH)
              list(INSERT _my_path 0 ${${_mod}_MODULE_PATH})
            endif()
            list(APPEND NEW_ALL_DEPS ${_mod})
          endif()
        endforeach()
      endif()
      list(LENGTH CMAKE_MODULE_PATH length)
      math(EXPR length "${length}-1")
      if(start EQUAL -1)
        list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${_my_path})
      else()
        if(start EQUAL ${length})
          list(APPEND CMAKE_MODULE_PATH ${_my_path})
        else()
          if(_my_path)
            list(INSERT CMAKE_MODULE_PATH ${start} ${_my_path})
          endif()
        endif()
      endif()
    endif()
    set(ALL_DEPENDENCIES ${NEW_ALL_DEPS})
  endif()
endmacro(dune_create_dependency_tree)


macro(dune_process_dependency_macros)
  foreach(_mod ${ALL_DEPENDENCIES} ${ProjectName})
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
        if (${${_mod}_FOUND}})
          message(STATUS "No module specific tests for module '${_mod}' ('${_macro}.cmake' not found)")
        endif()
      endif()
      dune_module_to_uppercase(_upper_case "${_mod}")
      if(${_mod}_INCLUDE_DIRS)
        message(STATUS "Setting ${_mod}_INCLUDE_DIRS=${${_mod}_INCLUDE_DIRS}")
        include_directories("${${_mod}_INCLUDE_DIRS}")
      endif()
      if(${_mod}_LIBRARIES)
        message(STATUS "Setting ${_mod}_LIBRARIES=${${_mod}_LIBRARIES}")
        foreach(_lib ${${_mod}_LIBRARIES})
          list(INSERT DUNE_LIBS 0 "${_lib}")
        endforeach()
      endif()

      # register dune module
      dune_register_package_flags(INCLUDE_DIRS "${${_mod}_INCLUDE_DIRS}")
    endif()
  endforeach()
endmacro(dune_process_dependency_macros)


# ------------------------------------------------------------------------
# Internal macros
# ------------------------------------------------------------------------


macro(find_dune_package module)
  cmake_parse_arguments(DUNE_FIND "REQUIRED" "VERSION" "" ${ARGN})
  if(DUNE_FIND_REQUIRED)
    set(required REQUIRED)
    set_package_properties(${module} PROPERTIES TYPE REQUIRED)
    set(_warning_level "FATAL_ERROR")
  else()
    unset(required)
    set(_warning_level "WARNING")
    set_package_properties(${module} PROPERTIES TYPE OPTIONAL)
  endif()
  if(DUNE_FIND_VERSION MATCHES "(>=|=|<=).*")
    string(REGEX REPLACE "(>=|=|<=)(.*)" "\\1" DUNE_FIND_VERSION_OP ${DUNE_FIND_VERSION})
    string(REGEX REPLACE "(>=|=|<=)(.*)" "\\2" DUNE_FIND_VERSION_NUMBER ${DUNE_FIND_VERSION})
    string(STRIP ${DUNE_FIND_VERSION_NUMBER} DUNE_FIND_VERSION_NUMBER)
    extract_major_minor_version("${DUNE_FIND_VERSION_NUMBER}" DUNE_FIND_VERSION)
    set(DUNE_FIND_VERSION_STRING "${DUNE_FIND_VERSION_MAJOR}.${DUNE_FIND_VERSION_MINOR}.${DUNE_FIND_VERSION_REVISION}")
  else()
    set(DUNE_FIND_VERSION_STRING "0.0.0")
  endif()
  if(NOT ${module}_FOUND)
    if(NOT (${module}_DIR OR ${module}_ROOT OR
       "${CMAKE_PREFIX_PATH}" MATCHES ".*${module}.*"))
      string(REPLACE  ${ProjectName} ${module} ${module}_DIR
        ${PROJECT_BINARY_DIR})
    endif()
    find_package(${module} NO_CMAKE_PACKAGE_REGISTRY)
  endif()
  if(NOT ${module}_FOUND AND NOT CMAKE_DISABLE_FIND_PACKAGE_${module})
    message(STATUS "No full CMake package configuration support available."
      " Falling back to pkg-config.")
    # use pkg-config
    find_package(PkgConfig)
    if(NOT PKG_CONFIG_FOUND AND required)
      message(FATAL_ERROR "Could not find module ${module}. We tried to use"
        "pkg-config but could not find it. ")
    endif()
        pkg_check_modules (${module} ${required} ${module}${DUNE_FIND_VERSION})
    set(${module}_FAKE_CMAKE_PKGCONFIG TRUE)
  endif()
  if(${module}_FAKE_CMAKE_PKGCONFIG)
    # compute the path to the libraries
    if(${module}_LIBRARIES)
      unset(_module_lib)
      foreach(lib ${${module}_LIBRARIES})
        foreach(libdir ${${module}_LIBRARY_DIRS})
          if(EXISTS ${libdir}/lib${lib}.a)
            set(_module_lib ${libdir}/lib${lib}.a)
            set(_module_lib_static "STATIC")
          endif()
          if(EXISTS ${libdir}/lib${lib}.so)
            set(_module_lib ${libdir}/lib${lib}.so)
            set(_module_lib_static "")
          endif()
          if(_module_lib)
            #import library
            add_library(${lib} ${_module_lib_static} IMPORTED)
            set_property(TARGET ${lib} APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
            set_target_properties(${lib} PROPERTIES
              IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
              IMPORTED_LOCATION_NOCONFIG "${_module_lib}")
            break()
          endif()
        endforeach()
      endforeach()
    endif()
    if(NOT ${module}_MODULE_PATH)
      if(${module}_INCLUDE_DIRS)
        list(GET ${module}_INCLUDE_DIRS 0 _dir)
        if(EXISTS ${_dir}/../share/dune/cmake/modules)
          set(${module}_MODULE_PATH ${_dir}/../share/dune/cmake/modules)
        endif()
      endif()
    endif()
    unset(${module}_FAKE_CMAKE_PKGCONFIG)
  endif()
  if(${module}_FOUND)
    # parse other module's dune.module file to generate variables for config.h
    unset(${module}_dune_module)
    foreach(_dune_module_file
        ${${module}_PREFIX}/dune.module
        ${${module}_PREFIX}/lib/dunecontrol/${module}/dune.module
        ${${module}_PREFIX}/lib64/dunecontrol/${module}/dune.module)
      if(EXISTS ${_dune_module_file})
        get_filename_component(_dune_module_file_path ${_dune_module_file} PATH)
        dune_module_information(${_dune_module_file_path})# QUIET)
        set(${module}_dune_module 1)
        set(DUNE_FIND_MOD_VERSION_STRING "${DUNE_VERSION_MAJOR}.${DUNE_VERSION_MINOR}.${DUNE_VERSION_REVISION}")
        # check whether dependency matches version requirement
        unset(module_version_wrong)
        if(DUNE_FIND_VERSION_OP MATCHES ">=")
          if(NOT (DUNE_FIND_MOD_VERSION_STRING VERSION_EQUAL DUNE_FIND_VERSION_STRING OR
                  DUNE_FIND_MOD_VERSION_STRING VERSION_GREATER DUNE_FIND_VERSION_STRING))
            set(module_version_wrong 1)
          endif()
        elseif(DUNE_FIND_VERSION_OP MATCHES "<=")
          if(NOT (DUNE_FIND_MOD_VERSION_STRING VERSION_EQUAL DUNE_FIND_VERSION_STRING OR
                  DUNE_FIND_MOD_VERSION_STRING VERSION_LESS DUNE_FIND_VERSION_STRING))
            set(module_version_wrong 1)
          endif()
        elseif(DUNE_FIND_VERSION_OP MATCHES "=" AND
           NOT (DUNE_FIND_MOD_VERSION_STRING VERSION_EQUAL DUNE_FIND_VERSION_STRING))
          set(module_version_wrong 1)
        endif()
      endif()
    endforeach()
    if(NOT ${module}_dune_module)
      message(${_warning_level} "Could not find dune.module file for module ${module} "
        "in ${${module}_PREFIX},  ${${module}_PREFIX}/lib/dunecontrol/${module}/, "
        "${${module}_PREFIX}/lib64/dunecontrol/${module}/dune.module")
      set(${module}_FOUND OFF)
    endif()
    if(module_version_wrong)
      message(${_warning_level} "Could not find requested version of module ${module}. "
        "Requested version was ${DUNE_FIND_VERSION}, found version is ${DUNE_FIND_MOD_VERSION_STRING}")
      set(${module}_FOUND OFF)
    endif()
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
      find_dune_package(${_mod} ${is_required} VERSION "${_ver}")
      set(${_mod}_SEARCHED ON)
      if(NOT "${is_required}" STREQUAL "")
        set(${_mod}_REQUIRED ON)
        set(${next_level_deps} ${${_mod}_DEPENDS} ${${next_level_deps}})
      else(NOT "${is_required}" STREQUAL "")
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
