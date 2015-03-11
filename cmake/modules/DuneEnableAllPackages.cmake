# This module provides the macros necessary for a simplified CMake build system
#
# The dune build system relies on the user to choose and add the compile and link flags
# necessary to build an executable. While this offers full control to the user, it
# is an error-prone procedure.
#
# Alternatively, users may use this modules macros to simply add the compile flags for all
# found external modules to all executables in a module. Likewise, all found libraries are
# linked to all targets.
#
# This module provides the following macros:
#
# dune_enable_all_packages([VERBOSE])
#
# Adds all flags and all libraries to all executables that are added in the directory
# from where this macro is called and from all its subdirectories (recursively).
# Typically, a user would add the call to this macro to his top level CMakeLists.txt
# With the VERBOSE option being set, the list of flags is printed during configure.
#
# dune_register_package_flags(COMPILE_DEFINITIONS flags
#                             INCLUDE_DIRS includes
#                             LIBRARIES libs
#                            [PREPEND]
#                            )
#
# To implement above feature, the compile flags, include paths and link flags of all
# found packages must be registered with this macro. This macro is only necessary for people
# that do link against additional libraries which are not supported by the dune core modules.
# Call this at the end of every find module. If you are using an external find module which
# you cannot alter, call it after the call find_package().
# The PREPEND parameter prepends the given flags to the global list instead of appending.
# Only use it, if you know what you are doing.

macro(dune_enable_all_packages)
  include(CMakeParseArguments)
  cmake_parse_arguments(ENABLE_ALL_PACKAGES VERBOSE "" "" ${ARGN})

  get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
  include_directories(${all_incs})
  if(ENABLE_ALL_PACKAGES_VERBOSE)
    message("Include directories for this project: ${all_incs}")
  endif(ENABLE_ALL_PACKAGES_VERBOSE)

  get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
  link_libraries(${DUNE_LIBS} ${all_libs})
  if(ENABLE_ALL_PACKAGES_VERBOSE)
    message("Libraries for this project: ${all_libs}")
  endif(ENABLE_ALL_PACKAGES_VERBOSE)

  get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
  foreach(def ${all_defs})
    add_definitions("-D${def}")
  endforeach(def in ${all_defs})
  if(ENABLE_ALL_PACKAGES_VERBOSE)
    message("Compile definitions for this project: ${all_defs}")
  endif(ENABLE_ALL_PACKAGES_VERBOSE)
endmacro(dune_enable_all_packages)

function(dune_register_package_flags)
  include(CMakeParseArguments)
  set(OPTIONS APPEND)
  set(SINGLEARGS)
  set(MULTIARGS COMPILE_DEFINITIONS INCLUDE_DIRS LIBRARIES)
  cmake_parse_arguments(REGISTRY "${OPTIONS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})

  if(REGISTRY_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_register_package_flags!")
  endif()

  if(REGISTRY_APPEND)
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_INCS "${REGISTRY_INCLUDE_DIRS}")
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_LIBS "${REGISTRY_LIBRARIES}")
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_DEFS "${REGISTRY_COMPILE_DEFINITIONS}")
  else(REGISTRY_APPEND)
    get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
    get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
    get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
    set_property(GLOBAL PROPERTY ALL_PKG_INCS "${REGISTRY_INCLUDE_DIRS}" "${all_incs}")
    set_property(GLOBAL PROPERTY ALL_PKG_LIBS "${REGISTRY_LIBRARIES}" "${all_libs}")
    set_property(GLOBAL PROPERTY ALL_PKG_DEFS "${REGISTRY_COMPILE_DEFINITIONS}" "${all_defs}")
  endif(REGISTRY_APPEND)
endfunction(dune_register_package_flags)
