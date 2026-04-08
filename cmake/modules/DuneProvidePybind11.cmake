# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneProvidePybind11
-------------------

Helpers for providing pybind11 to DUNE Python extension targets.

.. cmake:variable:: DUNE_USE_SYSTEM_PYBIND11

  Use an external pybind11 installation instead of the vendored headers shipped
  with ``dune-common``.

  When this option is enabled, :cmake:command:`dune_provide_pybind11()` first
  tries :cmake:command:`find_package(pybind11 CONFIG)`. If that fails, it
  downloads pybind11 and creates an imported compatibility target for it.

.. cmake:command:: dune_provide_pybind11

  Ensure that the given target can use pybind11.

  Depending on :cmake:variable:`DUNE_USE_SYSTEM_PYBIND11`, this either links the
  target against an external or downloaded ``pybind11::module`` target, or adds
  the vendored include directory from ``dune-common`` directly.

  .. code-block:: cmake

    dune_provide_pybind11(<target>)

  ``target``
    Existing target that should be configured for pybind11 usage.

#]=======================================================================]

include_guard(GLOBAL)

macro(dune_provide_pybind11 _target)
  # Option to disable the local copy of pybind11
  option(DUNE_USE_SYSTEM_PYBIND11
    "Use system version of pybind11, not our vendored copy" FALSE)
  add_feature_info(DUNE_USE_SYSTEM_PYBIND11 DUNE_USE_SYSTEM_PYBIND11
    "Use system version of pybind11, not our vendored copy")

  if(DUNE_USE_SYSTEM_PYBIND11)
    # First, try to find the package pybind11, e.g., as system library or
    # if the user manually specifies pybind11_DIR
    find_package(pybind11 CONFIG)

    if(NOT pybind11_FOUND)
      # If it was not found, download pybind11
      message(STATUS "Downloading pybind11 v3.0.1 from github")
      include(FetchContent)
      FetchContent_Declare(pybind11                            # name of the content
        GIT_REPOSITORY https://github.com/pybind/pybind11.git  # the repository
        GIT_TAG        v3.0.1                                  # the tag
        SOURCE_SUBDIR  _ignore_                                # ignore CMakeLists
        )
      FetchContent_MakeAvailable(pybind11)

      # Create a library from the source files
      dune_add_library(pybind11 INTERFACE
        EXPORT_NAME module NAMESPACE pybind11::)
      target_include_directories(pybind11 INTERFACE
        $<BUILD_INTERFACE:${pybind11_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/dune/_deps>)

      install(DIRECTORY "${pybind11_SOURCE_DIR}/include/pybind11"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/dune/_deps"
        FILES_MATCHING PATTERN "*.h")
      install(FILES "${PROJECT_SOURCE_DIR}/dune/python/pybind11/extensions.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/dune/_deps/pybind11")
    else()
      list(APPEND DUNE_COMMON_PACKAGE_DEPENDENCIES "find_dependency(pybind11)")
    endif()

    target_link_libraries(${_target} PUBLIC pybind11::module)
    target_compile_definitions(${_target} PUBLIC DUNE_USE_SYSTEM_PYBIND11=1)
  else()
    target_include_directories(${_target} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dune/python>          # <pybind11/...>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/dune/python>) # <pybind11/...>
  endif()
endmacro()
