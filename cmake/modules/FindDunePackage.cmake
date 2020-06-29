#[=======================================================================[.rst:
FindDunePackage
---------------

Find a dune module, and load its settings.

  find_dune_package(<module> [REQUIRED] [VERSION <version>])

#]=======================================================================]

include_guard(GLOBAL)

include(DuneModuleInformation)
include(FeatureSummary)

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
      string(REPLACE  ${PROJECT_NAME} ${module} ${module}_DIR
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
        # check whether dependency mathes version requirement
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
endmacro(find_dune_package module)
