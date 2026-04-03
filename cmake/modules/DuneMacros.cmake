# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneMacros
----------

Legacy compatibility helpers for the DUNE buildsystem.

.. cmake:command:: dune_target_link_libraries

  .. dune:internal::

  Link libraries to the given target.

  Deprecated compatibility wrapper around :cmake:command:`target_link_libraries`.

  .. code-block:: cmake

    dune_target_link_libraries(<target> <libraries>)

  ``target``
    Target to link against the given libraries.

  ``libraries``
    Libraries to link to the target.

.. cmake:command:: add_dune_all_flags

  .. dune:internal::

  Add all globally collected package flags to the given targets.

  Deprecated compatibility helper. Use
  :cmake:command:`dune_target_enable_all_packages()` instead.

  .. code-block:: cmake

    add_dune_all_flags(<target> [<target>...])

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

include_guard(GLOBAL)

enable_language(C) # Enable C to skip CXX bindings for some tests.

include(DuneAddLibrary)
include(DuneDefaultIncludeDirectories)
include(DuneEnableAllPackages)
include(DuneExecuteProcess)
include(DuneModuleDependencies)
include(DuneModuleInformation)
include(DunePathHelper)
include(DunePolicy)
include(DuneProject)
include(DuneReplaceProperties)
include(DuneSymlinkOrCopy)
include(DuneTestMacros)
include(DuneUtilities)

# Suppress a warning about a manually specified variable that is not used if
# python is deactivated.
set(DUNE_MAYBE_UNUSED_VARIABLES "DUNE_OPTS_FILE=${DUNE_OPTS_FILE}")

macro(target_link_dune_default_libraries _target)
  foreach(_lib ${DUNE_LIBS})
    target_link_libraries(${_target} PUBLIC ${_lib})
  endforeach()
endmacro(target_link_dune_default_libraries)

function(dune_target_link_libraries basename libraries)
  target_link_libraries(${basename} PUBLIC ${libraries})
endfunction(dune_target_link_libraries basename libraries)

macro(add_dune_all_flags targets)
  get_property(incs GLOBAL PROPERTY ALL_PKG_INCS)
  get_property(defs GLOBAL PROPERTY ALL_PKG_DEFS)
  get_property(libs GLOBAL PROPERTY ALL_PKG_LIBS)
  get_property(opts GLOBAL PROPERTY ALL_PKG_OPTS)
  foreach(target ${targets})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${incs})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${defs})
    target_link_libraries(${target} PUBLIC ${DUNE_LIBS} ${libs})
    target_compile_options(${target} PUBLIC ${opts})
  endforeach()
endmacro(add_dune_all_flags targets)
