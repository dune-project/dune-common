# Core DUNE module for CMake.
#
# Documentation of the public API defined in this module:
#
# .. cmake_function:: target_link_libraries
#
#    .. cmake_brief::
#
#       Overwrite of CMake's :code:`target_link_libraries`. If no interface key
#       word (like PUBLIC, INTERFACE, PRIVATE etc.) is given, PUBLIC is added.
#       This is to fix problems with CMP0023.
#
#    .. cmake_param:: basename
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

macro(target_link_libraries)
  # do nothing if not at least the two arguments target and scope are passed
  if(${ARGC} GREATER_EQUAL 2)
    target_link_libraries_helper(${ARGN})
  endif()
endmacro(target_link_libraries)

# helper for overwritten target_link_libraries to handle arguments more easily
macro(target_link_libraries_helper TARGET SCOPE)
  if(${SCOPE} MATCHES "^(PRIVATE|INTERFACE|PUBLIC|LINK_PRIVATE|LINK_PUBLIC|LINK_INTERFACE_LIBRARIES)$")
    _target_link_libraries(${TARGET} ${SCOPE} ${ARGN})
  else()
    message(DEPRECATION "Calling target_link_libraries without the <scope> argument is deprecated.")
    _target_link_libraries(${TARGET} PUBLIC ${SCOPE} ${ARGN})
  endif()
endmacro(target_link_libraries_helper)

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
