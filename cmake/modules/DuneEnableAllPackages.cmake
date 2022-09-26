# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Implementation of a simplified CMake build system.
#
# .. cmake_function:: dune_enable_all_packages
#
#    .. cmake_brief::
#
#       Previously, the DUNE build system relied on the user to choose and add the compile and link flags
#       necessary to build an executable. While this offers full control to the user, it
#       is an error-prone procedure.
#
#       Alternatively, users may use this function to simply add the compile flags for all
#       found external modules to all executables in a DUNE module. Likewise, all found libraries are
#       linked to all targets.
#
#    .. cmake_param:: INCLUDE_DIRS
#       :multi:
#
#       A list of include directories, that should be added to all targets.
#       In a standard Dune module, it is not necessary to specify anything.
#
#    .. cmake_param:: COMPILE_DEFINITIONS
#       :multi:
#
#       A list of compile definitions, that should be added to all targets.
#       In a standard Dune module, it is not necessary to specify anything.
#
#    .. cmake_param:: COMPILE_OPTIONS
#       :multi:
#
#       A list of non-definition compile options, that should be added to all targets.
#       In a standard Dune module, it is not necessary to specify anything.
#
#    .. cmake_param:: MODULE_LIBRARIES
#       :multi:
#
#       If your module contains libraries as well as programs and if the programs should automatically
#       link to those libraries, you *MUST* list these libraries in :code:`MODULE_LIBRARIES`. Those libraries will be
#       automatically created by :ref:`dune_enable_all_packages` (which internally calls :ref:`dune_add_library`) and placed
#       in the lib/ directory. The order of the libraries matters: if one library depends on another one, it must
#       be listed after its dependency. This special handling of the libraries is due to the way CMake
#       handle linking (in particular CMP022 and CMP038). You can later add source files to the library
#       anywhere in the source tree by calling :ref:`dune_library_add_sources`.
#
#    .. cmake_param:: VERBOSE
#       :option:
#
#       If this option is set, the set of compile flags, linked libraries and include directories
#       that is in use for all targets in the module is printed upon configuration.
#
#    .. cmake_param:: APPEND
#       :option:
#
#       If this option is set, the definitions, flags and directories specified in this function are
#       appended to the global collection of flags instead of being prepended. Only use it, if you know
#       what you are doing.
#
#    Adds all flags and all libraries to all executables that are subsequently added in the directory
#    from where this function is called and from all its subdirectories (recursively).
#    If used, this function *MUST* be called in the top level CMakeLists.txt BEFORE adding any subdirectories!
#    You can optionally add additional include dirs and compile definitions that will also be applied to
#    all targets in the module.
#
#    .. note::
#       If you want to use :code:`dune_enable_all_packages` with an older version of CMake and your DUNE module
#       creates its own library, you have to manually create the library in the top-level CMakeLists.txt
#       file using :ref:`dune_add_library` (with all sources listed within that call), use
#       :ref:`dune_target_enable_all_packages` to add all packages to the library and finally list that library
#       under :code:`LIBRARIES` in the call to :ref:`dune_register_package_flags`. See dune-pdelab for an example of
#       how to do this correctly.
#
#    While :ref:`dune_enable_all_packages` defines the user interface for this feature, developers might
#    also be interested in the following related functions:
#
#    * :ref:`dune_target_enable_all_packages`
#    * :ref:`dune_register_package_flags`
#    * :ref:`dune_library_add_sources`
#
# .. cmake_function:: dune_target_enable_all_packages
#
#    .. cmake_param:: TARGETS
#       :multi:
#
#       A list of targets to add all flags etc. too.
#
#    Adds all currently registered package flags (see :ref:`dune_register_package_flags`) to the given targets.
#    This function is mainly intended to help write DUNE modules that want to use :ref:`dune_enable_all_packages` and
#    define their own libraries, but need to be compatible with CMake < 3.1
#
# .. cmake_function:: dune_register_package_flags
#
#    .. cmake_param:: INCLUDE_DIRS
#       :multi:
#
#       The list of include directories needed by the external package.
#
#    .. cmake_param:: COMPILE_DEFINITIONS
#       :multi:
#
#       The list of compile definitions needed by the external package.
#
#    .. cmake_param:: COMPILE_OPTIONS
#       :multi:
#
#       The list of compile options needed by the external package.
#
#    .. cmake_param:: LIBRARIES
#       :multi:
#
#       The list of libraries that the external package should link to.
#       The order of the input is preserved in the output.
#
#    .. cmake_param:: APPEND
#       :option:
#
#       If this option is set, the definitions, flags and directories specified in this function are
#       appended to the global collection of flags instead of being prepended. Only use it, if you know
#       what you are doing.
#
#    To correctly implement the automatic handling of external libraries, the compile flags, include paths and link
#    flags of all found packages must be registered with this function. This function is only necessary for people that
#    want to write their own :code:`FindFooBar` CMake modules to link against additional libraries which are not supported by
#    the DUNE core modules. Call this function at the end of every find module. If you are using an external FindFoo
#    module which you cannot alter, call it after the call to :code:`find_package(foo)`.
#
# .. cmake_function:: dune_library_add_sources
#
#    .. cmake_param:: module_library
#       :single:
#       :positional:
#
#       The name of the module library target.
#
#    .. cmake_param: SOURCES
#       :multi:
#       :required:
#
#       The source files to add to the DUNE module library :code:`module_library`.
#       That library must have been created by an earlier call to :ref:`dune_enable_all_packages`
#       in the current DUNE module.
#
#    Register sources for module exported library.
#
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

    get_property(all_incs GLOBAL PROPERTY ALL_PKG_INCS)
    target_include_directories(${_target} PUBLIC ${all_incs})

    get_property(all_defs GLOBAL PROPERTY ALL_PKG_DEFS)
    target_compile_definitions(${_target} PUBLIC ${all_defs})

    get_property(all_opts GLOBAL PROPERTY ALL_PKG_OPTS)
    target_compile_options(${_target} PUBLIC ${all_opts})

    get_property(all_libs GLOBAL PROPERTY ALL_PKG_LIBS)
    target_link_libraries(${_target} PUBLIC ${DUNE_LIBS} ${all_libs})

  endforeach()
endfunction(dune_target_enable_all_packages)


function(dune_library_add_sources lib)
  if (NOT (DEFINED DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES))
    message(FATAL_ERROR "You must call dune_enable_all_packages with the MODULE_LIBRARIES option before calling dune_library_add_sources")
  endif()

  if (NOT lib IN_LIST DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES)
    message(FATAL_ERROR
"Attempt to add sources to library ${lib}, which has not been defined in dune_enable_all_packages.
List of libraries defined in dune_enable_all_packages: ${DUNE_ENABLE_ALL_PACKAGES_MODULE_LIBRARIES}")
  endif()

  cmake_parse_arguments(DUNE_LIBRARY_ADD_SOURCES "" "" "SOURCES" ${ARGN})

  if(DUNE_LIBRARY_ADD_SOURCES_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments for dune_library_add_sources!")
  endif()

  target_sources(${lib} PRIVATE ${DUNE_LIBRARY_ADD_SOURCES_SOURCES})
endfunction(dune_library_add_sources)
