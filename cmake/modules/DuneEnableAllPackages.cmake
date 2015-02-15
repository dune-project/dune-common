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
# dune_enable_all_packages()
#
# Adds all flags and all libraries to all executables that are added in the directory
# from where this macro is called and from all its subdirectories (recursively).
# Typically, a user would add the call to this macro to his top level CMakeLists.txt

macro(dune_enable_all_packages)
  get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
  include_directories(${all_incs})
  get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
  link_libraries(${DUNE_LIBS} ${all_libs})
  get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
  foreach(def ${all_defs})
    add_definitions("-D${def}")
  endforeach(def in ${all_defs})
endmacro(dune_enable_all_packages)
