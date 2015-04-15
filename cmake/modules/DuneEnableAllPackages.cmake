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
# dune_enable_all_packages(INCLUDE_DIRS [include_dirs]
#                          COMPILE_DEFINITIONS [compile_definitions]
#                          MODULE_LIBRARIES [libraries]
#                          [VERBOSE] [APPEND]
#                          )
#
# Adds all flags and all libraries to all executables that are subsequently added in the directory
# from where this macro is called and from all its subdirectories (recursively).
# If used, this macro MUST be called in the top level CMakeLists.txt BEFORE adding any subdirectories!
# You can optionally add additional include dirs and compile definitions that will also be applied to
# all targets in the module.
# Finally, if your module contains libraries as well as programs and if the programs should automatically
# link to those libraries, you MUST list these libraries in MODULE_LIBRARIES. Those libraries will be
# automatically created by dune_enable_all_packages (which internally calls dune_add_library()) and placed
# in the lib/ directory. The order of the libraries matters: if one library depends on another one, it must
# be listed after its dependency. This special handling of the libraries is due to the way newer CMake
# versions handle linking (in particular CMP022 and CMP038). You can later add source files to the library
# anywhere in the source tree by calling dune_library_add_sources().
#
# Warning: The library feature requires CMake 3.1+. If you use the feature with older versions, CMake
#          will emit a fatal error. Moreover, it will issue a warning if the cmake_minimum_required()
#          version is older than 3.1.
#
# For a description of the APPEND option, see the documentation of dune_register_package_flags().
# With the VERBOSE option being set, the list of flags is printed during configure.
#
# dune_register_package_flags(COMPILE_DEFINITIONS flags
#                             INCLUDE_DIRS includes
#                             LIBRARIES libs
#                            [APPEND]
#                            )
#
# To implement above feature, the compile flags, include paths and link flags of all
# found packages must be registered with this macro. This macro is only necessary for people
# that do link against additional libraries which are not supported by the dune core modules.
# Call this at the end of every find module. If you are using an external find module which
# you cannot alter, call it after the call find_package().
# The APPEND parameter appends the given flags to the global list instead of prepending.
# Only use it, if you know what you are doing.
#
# dune_library_add_sources(module_library
#                          SOURCES [sources]
#                         )
#
# Adds the source files listed in [sources] to the module library module_library created by an earlier
# call to dune_enable_all_packages.
#

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


macro(dune_enable_all_packages)
  include(CMakeParseArguments)
  set(OPTIONS APPEND VERBOSE)
  set(SINGLEARGS)
  set(MULTIARGS COMPILE_DEFINITIONS INCLUDE_DIRS MODULE_LIBRARIES)
  cmake_parse_arguments(ENABLE_ALL_PACKAGES "${OPTIONS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})

  if(ENABLE_ALL_PACKAGES_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_enable_all_packages!")
  endif()

  # handle additional include dirs specified in dune_enable_all_packages
  if(ENABLE_ALL_PACKAGES_INCLUDE_DIRS)
    if(ENABLE_ALL_PACKAGES_APPEND)
      set_property(GLOBAL APPEND PROPERTY ALL_PKG_INCS "${ENABLE_ALL_PACKAGES_INCLUDE_DIRS}")
    else(ENABLE_ALL_PACKAGES_APPEND)
      get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
      set_property(GLOBAL PROPERTY ALL_PKG_INCS "${ENABLE_ALL_PACKAGES_INCLUDE_DIRS}" "${all_incs}")
    endif(ENABLE_ALL_PACKAGES_APPEND)
  endif(ENABLE_ALL_PACKAGES_INCLUDE_DIRS)

  # add include dirs to all targets in module
  get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
  include_directories(${all_incs})
  # verbose output of include dirs
  if(ENABLE_ALL_PACKAGES_VERBOSE)
    message("Include directories for this project: ${all_incs}")
  endif(ENABLE_ALL_PACKAGES_VERBOSE)

  # handle additional compile definitions specified in dune_enable_all_packages
  if(ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS)
    if(ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS)
      set_property(GLOBAL APPEND PROPERTY ALL_PKG_DEFS "${ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS}")
    else(ENABLE_ALL_PACKAGES_APPEND)
      get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
      set_property(GLOBAL PROPERTY ALL_PKG_DEFS "${ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS}" "${all_defs}")
    endif(ENABLE_ALL_PACKAGES_APPEND)
  endif(ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS)

  # add compile definitions to all targets in module
  get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
  foreach(def ${all_defs})
    add_definitions("-D${def}")
  endforeach(def in ${all_defs})
  # verbose output of compile definitions
  if(ENABLE_ALL_PACKAGES_VERBOSE)
    message("Compile definitions for this project: ${all_defs}")
  endif(ENABLE_ALL_PACKAGES_VERBOSE)

  # handle libraries
  # this is a little tricky because the libraries defined within the current module require special
  # handling to avoid tripping over CMake policies CMP022 and CMP038

  # first add all libraries of upstream Dune modules and of external dependencies
  get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
  link_libraries(${DUNE_LIBS} ${all_libs})

  # now we have to do a little dance: Newer versions of CMake complain if a target links to itself,
  # so we have to create all targets for libraries inside the module before adding them to the set
  # of default libraries to link to. That works because calling link_libraries does not affect targets
  # which already exist.
  # Moroever, CMake generates a warning when creating a library without any source files, and the linker
  # does the same if we add an empty dummy file. We work around that problem by autogenerating a library-specific
  # stub source file with two functions ${lib_name}_version() and ${lib_name}_version_string() and add that
  # as an initial source file.
  # After creating the library with dune_add_library(), we add it to all future targets with a call to
  # link_libraries(). The user can then add the real source files by calling dune_library_add_sources()
  # throughout the module.

  if(ENABLE_ALL_PACKAGES_MODULE_LIBRARIES)

    # This only works for CMAKE 3.1+ because target_sources() - which we use to add sources to the
    # libraries after creating them - was added in that version
    if (CMAKE_VERSION VERSION_LESS 3.1.0)
      message(FATAL_ERROR "dune_enable_all_packages() only supports MODULE_LIBRARIES for CMake 3.1+")
    elseif(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 3.1.0)
      message(WARNING
"You are using dune_enable_all_packages() with the MODULE_LIBRARIES feature.
This requires at least CMake 3.1, but your Dune module only requires ${CMAKE_MINIMUM_REQUIRED_VERSION}.
Update the cmake_minimum_required() call in your main CMakeLists.txt file to get rid of this warning.")
    endif()

    # make sure the /lib directory exists - we need it to create the stub source file in there
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    # figure out the location of the stub source template
    dune_common_script_dir(script_dir)
    foreach(module_lib ${ENABLE_ALL_PACKAGES_MODULE_LIBRARIES})
      # create the stub source file in the output directory...
      configure_file("${script_dir}/module_library.cc.in" "${PROJECT_BINARY_DIR}/lib/lib${module_lib}_stub.cc")
      # ...and create the library...
      dune_add_library(${module_lib} SOURCES "${PROJECT_BINARY_DIR}/lib/lib${module_lib}_stub.cc")
      # ...and add it to all future targets in the module
      link_libraries(${module_lib})
    endforeach(module_lib ${ENABLE_ALL_PACKAGES_MODULE_LIBRARIES})
  endif(ENABLE_ALL_PACKAGES_MODULE_LIBRARIES)

  if(ENABLE_ALL_PACKAGES_VERBOSE)
    get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
    message("Libraries for this project: ${all_libs}")
  endif(ENABLE_ALL_PACKAGES_VERBOSE)

endmacro(dune_enable_all_packages)


macro(dune_library_add_sources lib)

  # This only works for CMAKE 3.1+ because target_sources() - which we use to add sources to the
  # libraries after creating them - was added in that version
  if (CMAKE_VERSION VERSION_LESS 3.1.0)
    message(FATAL_ERROR "dune_library_add_sources() requires CMake 3.1+")
  elseif(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 3.1.0)
    message(WARNING
"You are using dune_enable_all_packages() with the MODULE_LIBRARIES feature.
This requires at least CMake 3.1, but your Dune module only requires ${CMAKE_MINIMUM_REQUIRED_VERSION}.
Update the cmake_minimum_required() call in your main CMakeLists.txt file to get rid of this warning.")
  endif()

  include(CMakeParseArguments)
  cmake_parse_arguments(DUNE_LIB "" "" "SOURCES" ${ARGN})

  if(DUNE_LIB_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_library_add_sources!")
  endif()

  foreach(source ${DUNE_LIB_SOURCES})
    target_sources(${lib} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${source})
  endforeach()
endmacro()
