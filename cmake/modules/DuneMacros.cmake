# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Core DUNE module for CMake.
#
# Documentation of the public API defined in this module:
#
# .. cmake_function:: dune_target_link_libraries
#
#    .. cmake_param:: BASENAME
#
#    .. cmake_param:: LIBRARIES
#
#    Link libraries to the static and shared version of
#    library BASENAME
#
#
# .. cmake_function:: add_dune_all_flags
#
#    .. cmake_param:: targets
#       :single:
#       :required:
#       :positional:
#
#       The targets to add the flags of all external libraries to.
#
#    This function is superseded by :ref:`dune_target_enable_all_packages`.
#
include_guard(GLOBAL)

enable_language(C) # Enable C to skip CXX bindings for some tests.

# By default use -pthread flag. This option is set at the beginning to enforce it for
# find_package(Threads) everywhere
set(THREADS_PREFER_PTHREAD_FLAG TRUE CACHE BOOL "Prefer -pthread compiler and linker flag")

include(DuneAddLibrary)
include(DuneEnableAllPackages)
include(DuneExecuteProcess)
include(DuneModuleDependencies)
include(DuneModuleInformation)
include(DunePathHelper)
include(DuneProject)
include(DuneReplaceProperties)
include(DuneSymlinkOrCopy)
include(DuneTestMacros)
include(DuneUtilities)

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
