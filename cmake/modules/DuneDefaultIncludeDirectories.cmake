# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneDefaultIncludeDirectories
-----------------------------

Add the default include directories to a target.

.. cmake:command:: dune_default_include_directories

  Add the standard DUNE include directories to a target.

  .. code-block:: cmake

    dune_default_include_directories(<target> [PRIVATE|PUBLIC|INTERFACE])

  ``<target>``
    Target whose include directories should be extended.

  ``[PRIVATE|PUBLIC|INTERFACE]``
    Visibility used for the default include directories added through
    ``target_include_directories()``.

  The command adds the project source directory, build directory, generated
  public include directory, and installed include directory. For non-interface
  targets it also adds the generated private include directory.

#]=======================================================================]
include_guard(GLOBAL)

function(dune_default_include_directories _target _scope)
  target_include_directories(${_target} ${_scope}
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>          # config.h
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>  # <module>-config.hh
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  if(NOT _scope STREQUAL "INTERFACE")
    target_include_directories(${_target} PRIVATE
      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include_private>) # <module>-config-private.hh
  endif()
endfunction(dune_default_include_directories)
