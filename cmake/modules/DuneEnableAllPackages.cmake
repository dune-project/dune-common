# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneEnableAllPackages
---------------------

Commands for simplified package flag handling in the DUNE build system.

.. cmake:command:: dune_enable_all_packages

  Register all discovered package flags and libraries for subsequent targets in
  the current directory scope and its subdirectories.

  .. code-block:: cmake

    dune_enable_all_packages(
      [INCLUDE_DIRS <dirs...>]
      [COMPILE_DEFINITIONS <defs...>]
      [COMPILE_OPTIONS <opts...>]
      [MODULE_LIBRARIES <libs...>]
      [VERBOSE]
      [APPEND]
    )

  This command provides a simplified mode in which all collected external
  package flags are applied automatically to targets created later in the
  module.

  ``INCLUDE_DIRS``
    Additional include directories added to all subsequent targets.

  ``COMPILE_DEFINITIONS``
    Additional compile definitions added to all subsequent targets.

  ``COMPILE_OPTIONS``
    Additional non-definition compile options added to all subsequent targets.

  ``MODULE_LIBRARIES``
    Libraries from the current module that should participate in the automatic
    linking setup. If programs in the module should link to those libraries
    automatically, they must be listed here.

  ``VERBOSE``
    Print the resulting compile flags, linked libraries, and include
    directories during configuration.

  ``APPEND``
    Append the provided flags to the global collection instead of prepending
    them.

  If used, this command must be called in the top-level ``CMakeLists.txt``
  before adding subdirectories. For modern module libraries, prefer
  :cmake:command:`dune_add_library` and explicit target properties when
  possible.


.. cmake:command:: dune_target_enable_all_packages

  Add all currently registered package flags to specific targets.

  .. code-block:: cmake

    dune_target_enable_all_packages(TARGETS <targets...>)

  ``TARGETS``
    The targets receiving the currently registered package flags.

  The added flags use ``PUBLIC`` scope for compiled libraries and ``INTERFACE``
  scope otherwise.


.. cmake:command:: dune_register_package_flags

  Register package-specific include paths, compile flags, and link libraries.

  .. code-block:: cmake

    dune_register_package_flags(
      [INCLUDE_DIRS <dirs...>]
      [COMPILE_DEFINITIONS <defs...>]
      [COMPILE_OPTIONS <opts...>]
      [LIBRARIES <libs...>]
      [APPEND]
    )

  ``INCLUDE_DIRS``
    Include directories required by the external package.

  ``COMPILE_DEFINITIONS``
    Compile definitions required by the external package.

  ``COMPILE_OPTIONS``
    Compile options required by the external package.

  ``LIBRARIES``
    Libraries that should be linked for the external package. The order of the
    input is preserved.

  ``APPEND``
    Append the specified values to the global collection instead of prepending
    them.

  This command is primarily intended for custom ``FindFoo`` modules or for
  integrating external find modules that cannot be modified directly.


.. cmake:command:: dune_library_add_sources

  Register sources for a module library created through the automatic package
  handling workflow.

  .. code-block:: cmake

    dune_library_add_sources(<module_library> SOURCES <sources...>)

  ``module_library``
    The name of the module library target.

  ``SOURCES``
    Source files added to the DUNE module library ``module_library``.

  The target must have been created earlier by
  :cmake:command:`dune_enable_all_packages` in the current module.

#]=======================================================================]
include_guard(GLOBAL)

function(dune_register_package_flags)
  set(MULTIARGS COMPILE_DEFINITIONS COMPILE_OPTIONS INCLUDE_DIRS LIBRARIES)
  cmake_parse_arguments(REGISTRY "APPEND" "" "${MULTIARGS}" ${ARGN})

  if(REGISTRY_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_register_package_flags!")
  endif()

  if(REGISTRY_APPEND)
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_INCS "${REGISTRY_INCLUDE_DIRS}")
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_LIBS "${REGISTRY_LIBRARIES}")
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_DEFS "${REGISTRY_COMPILE_DEFINITIONS}")
    set_property(GLOBAL APPEND PROPERTY ALL_PKG_OPTS "${REGISTRY_COMPILE_OPTIONS}")
  else(REGISTRY_APPEND)
    get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
    get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
    get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
    get_property(all_opts GLOBAL PROPERTY ALL_PKG_OPTS)
    set_property(GLOBAL PROPERTY ALL_PKG_INCS "${REGISTRY_INCLUDE_DIRS}" "${all_incs}")
    set_property(GLOBAL PROPERTY ALL_PKG_LIBS "${REGISTRY_LIBRARIES}" "${all_libs}")
    set_property(GLOBAL PROPERTY ALL_PKG_DEFS "${REGISTRY_COMPILE_DEFINITIONS}" "${all_defs}")
    set_property(GLOBAL PROPERTY ALL_PKG_OPTS "${REGISTRY_COMPILE_OPTIONS}" "${all_opts}")
  endif(REGISTRY_APPEND)
endfunction(dune_register_package_flags)


function(dune_enable_all_packages)
  set(MULTIARGS COMPILE_DEFINITIONS COMPILE_OPTIONS INCLUDE_DIRS MODULE_LIBRARIES)
  cmake_parse_arguments(DUNE_ENABLE_ALL_PACKAGES "APPEND;VERBOSE" "" "${MULTIARGS}" ${ARGN})

  if(DUNE_ENABLE_ALL_PACKAGES_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_enable_all_packages!")
  endif()

  # handle additional include dirs specified in dune_enable_all_packages
  if(DUNE_ENABLE_ALL_PACKAGES_INCLUDE_DIRS)
    if(DUNE_ENABLE_ALL_PACKAGES_APPEND)
      set_property(GLOBAL APPEND PROPERTY ALL_PKG_INCS "${DUNE_ENABLE_ALL_PACKAGES_INCLUDE_DIRS}")
    else(DUNE_ENABLE_ALL_PACKAGES_APPEND)
      get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
      set_property(GLOBAL PROPERTY ALL_PKG_INCS "${DUNE_ENABLE_ALL_PACKAGES_INCLUDE_DIRS}" "${all_incs}")
    endif(DUNE_ENABLE_ALL_PACKAGES_APPEND)
  endif(DUNE_ENABLE_ALL_PACKAGES_INCLUDE_DIRS)

  # add include dirs to all targets in module
  get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
  include_directories(${all_incs})
  # verbose output of include dirs
  if(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)
    message("Include directories for this project: ${all_incs}")
  endif(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)

  # handle additional compile definitions specified in dune_enable_all_packages
  if(DUNE_ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS)
    if(DUNE_ENABLE_ALL_PACKAGES_APPEND)
      set_property(GLOBAL APPEND PROPERTY ALL_PKG_DEFS "${DUNE_ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS}")
    else(DUNE_ENABLE_ALL_PACKAGES_APPEND)
      get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
      set_property(GLOBAL PROPERTY ALL_PKG_DEFS "${DUNE_ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS}" "${all_defs}")
    endif(DUNE_ENABLE_ALL_PACKAGES_APPEND)
  endif(DUNE_ENABLE_ALL_PACKAGES_COMPILE_DEFINITIONS)

  # add compile definitions to all targets in module
  get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
  # We have to do this in a loop because add_definitions() is kind of broken: even though it is supposed
  # to be *the* function for adding compile definitions, it does not prepend "-D" (as opposed to
  # target_compile_definitions(), which does). Well, whatever...
  foreach(_definition ${all_defs})
    if(_definition)
      add_definitions("-D${_definition}")
    endif()
  endforeach()
  # verbose output of compile definitions
  if(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)
    message("Compile definitions for this project: ${all_defs}")
  endif(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)

  # handle additional compile options specified in dune_enable_all_packages
  if(DUNE_ENABLE_ALL_PACKAGES_COMPILE_OPTIONS)
    if(DUNE_ENABLE_ALL_PACKAGES_APPEND)
      set_property(GLOBAL APPEND PROPERTY ALL_PKG_OPTS "${DUNE_ENABLE_ALL_PACKAGES_COMPILE_OPTIONS}")
    else(DUNE_ENABLE_ALL_PACKAGES_APPEND)
      get_property(all_opts GLOBAL PROPERTY ALL_PKG_OPTS)
      set_property(GLOBAL PROPERTY ALL_PKG_OPTS "${DUNE_ENABLE_ALL_PACKAGES_COMPILE_OPTIONS}" "${all_opts}")
    endif(DUNE_ENABLE_ALL_PACKAGES_APPEND)
  endif(DUNE_ENABLE_ALL_PACKAGES_COMPILE_OPTIONS)

  # add compile options to all targets in module
  get_property(all_opts GLOBAL PROPERTY ALL_PKG_OPTS)
  add_compile_options(${all_opts})
  # verbose output of compile definitions
  if(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)
    message("Compile options for this project: ${all_opts}")
  endif(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)

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

  if(DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES)
    message(DEPRECATION "The `MODULE_LIBRARIES <lib>` argument to `dune_enable_all_packages` is deprecated. To achieve the same thing, first create a library with `dune_add_library(<lib>)`, then add all packages to this library with `dune_target_enable_all_packages(<lib>)`, and finally link this library to all other available targets with `link_libraries(<lib>)`.")

    # make sure the /lib directory exists - we need it to create the stub source file in there
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    # figure out the location of the stub source template
    dune_module_path(MODULE dune-common RESULT script_dir SCRIPT_DIR)
    foreach(module_lib ${DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES})
      # create the stub source file in the output directory (using a c++ compatible name)...
      string(REGEX REPLACE "[^a-zA-Z0-9]" "_" module_lib_mangled ${module_lib})
      configure_file("${script_dir}/module_library.cc.in" "${PROJECT_BINARY_DIR}/lib/lib${module_lib}_stub.cc")

      # ...and create the library...
      dune_add_library(${module_lib} SOURCES "${PROJECT_BINARY_DIR}/lib/lib${module_lib}_stub.cc")
      # ...and add it to all future targets in the module
      link_libraries(${module_lib})
    endforeach(module_lib ${DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES})

    # export the DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES variable to the parent scope
    # this is required to make dune_library_add_sources() work (see further down)
    set(
      DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES
      ${DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES}
      PARENT_SCOPE
      )
  endif(DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES)

  if(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)
    get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
    message("Libraries for this project: ${all_libs}")
  endif(DUNE_ENABLE_ALL_PACKAGES_VERBOSE)

endfunction(dune_enable_all_packages)


function(dune_target_enable_all_packages)
  foreach(_target ${ARGN})
    get_target_property(_target_type ${_target} TYPE)
    if (${_target_type} STREQUAL "INTERFACE_LIBRARY")
      set(scope INTERFACE)
    else()
      set(scope PUBLIC)
    endif()

    get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
    target_include_directories(${_target} ${scope} ${all_incs})

    get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
    target_compile_definitions(${_target} ${scope} ${all_defs})

    get_property(all_opts GLOBAL PROPERTY ALL_PKG_OPTS)
    target_compile_options(${_target} ${scope} ${all_opts})

    get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
    target_link_libraries(${_target} ${scope} ${DUNE_LIBS} ${all_libs})

  endforeach()
endfunction(dune_target_enable_all_packages)


function(dune_library_add_sources lib)
  message(DEPRECATION "The function `dune_library_add_sources(<lib> SOURCES ...)` is
  deprecated. Use the cmake function `target_sources(<lib> PRIVATE ...)` directly.")

  cmake_parse_arguments(DUNE_LIBRARY_ADD_SOURCES "" "" "SOURCES" ${ARGN})
  if(DUNE_LIBRARY_ADD_SOURCES_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_library_add_sources!")
  endif()

  target_sources(${lib} PRIVATE ${DUNE_LIBRARY_ADD_SOURCES_SOURCES})
endfunction(dune_library_add_sources)
