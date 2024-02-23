# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneDefaultIncludeDirectories
-----------------------------

Add the default include directories to a target.

.. cmake:command:: dune_default_include_directories

  Add the source and build directory the the targets include directories
  in the build-interface and the corresponding installed include dir in
  the install-interface.

  .. code-block:: cmake

    dune_default_include_directories(<target> [PRIVATE|PUBLIC|INTERFACE])

#]=======================================================================]
include_guard(GLOBAL)

function(dune_default_include_directories _target _scope)
  target_include_directories(${_target} ${_scope}
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include_private>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
endfunction(dune_default_include_directories)