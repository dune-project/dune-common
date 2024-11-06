# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneProject
-----------

Initialize and finalize a Dune module.

.. cmake:command:: dune_project

  The command ``dune_project()`` starts a new Dune module by setting several
  variables and loading necessary dependencies.

  .. code-block:: cmake

    dune_project()

  This function needs to be called from every module top-level
  ``CMakeLists.txt`` file. It sets up the module, defines basic variables and
  manages dependencies. Don't forget to call :command:`finalize_dune_project`
  at the end of that ``CMakeLists.txt`` file.


.. cmake:command:: finalize_dune_project

  Finalize the creation of the Dune module by creating package config files.

  .. code-block:: cmake

    finalize_dune_project()

  This function needs to be run at the end of every top-level
  ``CMakeLists.txt`` file. Among other things it creates the cmake package
  configuration file and package version file. Modules can add additional
  entries to these files by setting the variable ``${ProjectName}_INIT``.

  Based on the template ``config.h.cmake`` in the project directory,
  a private header file ``${module}-private-config.hh`` and a public header
  file ``${module}-config.hh`` are created. The public header is installed.
  For legacy reasons, the file ``config.h`` with collected upstream
  configuration files is also created. For more information, see the build
  system documentation.

#]=======================================================================]
include_guard(GLOBAL)

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(DuneModuleDependencies)
include(DuneModuleInformation)
include(DuneSymlinkOrCopy)
include(DunePkgConfig)
include(DuneUtilities)
include(FeatureSummary)
include(GNUInstallDirs)
include(Headercheck)
include(OverloadCompilerFlags)

include(DunePolicy)
dune_define_policy(DP_DEFAULT_INCLUDE_DIRS dune-common 2.12
  "OLD behavior: Use global include_directories. NEW behavior: Include directories must be set on a module library target and are not set globally anymore.")

# Macro that should be called near the beginning of the top level CMakeLists.txt.
# Namely it sets up the module, defines basic variables and manages
# dependencies.
# Don't forget to call finalize_dune_project afterwards.
macro(dune_project)

  if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    message(DEPRECATION "The function `dune_project` shall only be called on CMake project root directory.
    Note that CMake projects may be nested: https://cmake.org/cmake/help/latest/command/project.html.")
  endif()

  # check if CXX flag overloading has been enabled (see OverloadCompilerFlags.cmake)
  initialize_compiler_script()

  # extract information from dune.module
  dune_module_information(${PROJECT_SOURCE_DIR})
  set(ProjectName            "${DUNE_MOD_NAME}")
  set(ProjectVersion         "${DUNE_MOD_VERSION}")
  set(ProjectVersionString   "${DUNE_VERSION_MAJOR}.${DUNE_VERSION_MINOR}.${DUNE_VERSION_REVISION}")
  set(ProjectVersionMajor    "${DUNE_VERSION_MAJOR}")
  set(ProjectVersionMinor    "${DUNE_VERSION_MINOR}")
  set(ProjectVersionRevision "${DUNE_VERSION_REVISION}")
  set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")
  set(ProjectDescription     "${${DUNE_MOD_NAME_UPPERCASE}_DESCRIPTION}")
  set(ProjectAuthor          "${${DUNE_MOD_NAME_UPPERCASE}_AUTHOR}")
  set(ProjectUrl             "${${DUNE_MOD_NAME_UPPERCASE}_URL}")
  set(ProjectPythonRequires  "${${DUNE_MOD_NAME_UPPERCASE}_PYTHON_REQUIRES}")

  # check whether this module has been explicitly disabled through the cmake flags.
  # If so, stop the build. This is necessary because dunecontrol does not parse
  # the given CMake flags for a disabled Dune modules.
  if(CMAKE_DISABLE_FIND_PACKAGE_${ProjectName})
    message("Module ${ProjectName} has been explicitly disabled through the cmake flags. Skipping build.")
    return()
  endif()

  define_property(GLOBAL PROPERTY ${ProjectName}_LIBRARIES
        BRIEF_DOCS "List of libraries of the module. DO NOT EDIT!"
        FULL_DOCS "List of all module-libraries. DO NOT EDIT!")

  define_property(GLOBAL PROPERTY ${ProjectName}_EXPORTED_LIBRARIES
        BRIEF_DOCS "List of libraries exported by the module. DO NOT EDIT!"
        FULL_DOCS "List of libraries exported by the module. DO NOT EDIT!")

  # find weak requirements
  foreach(_mod IN LISTS ${ProjectName}_SUGGESTS)
    split_module_version(${_mod} _mod_name _mod_ver)
    find_dune_package(${_mod_name} VERSION ${_mod_ver} QUIET)
  endforeach()

  # find hard requirements
  foreach(_mod IN LISTS ${ProjectName}_DEPENDS)
    split_module_version(${_mod} _mod_name _mod_ver)
    find_dune_package(${_mod_name} VERSION ${_mod_ver} REQUIRED)
  endforeach()

  if(DUNE_COMMON_VERSION VERSION_LESS 2.12)
    # this is only needed if config files of upstream modules were generated with dune-common < 2.10
    # this behavior will be unsupported when dune-common == 2.12

    set_property(GLOBAL PROPERTY DUNE_DISABLE_ALL_DEPENDENCIES_DEPRECATION_WARNING ON)
    # creates dependency tree, finds all the modules and creates ALL_DEPENDENCIES variable
    dune_create_dependency_tree(SKIP_CMAKE_PATH_SETUP)

    # check if all the dependencies in the tree were indeed added into DUNE_FOUND_DEPENDENCIES
    set(_legacy_order OFF)
    foreach(_mod IN LISTS ALL_DEPENDENCIES)
      list(FIND DUNE_FOUND_DEPENDENCIES ${_mod} _mod_in_dune_deps)
      if (${_mod}_FOUND AND (${_mod_in_dune_deps} EQUAL -1))
        set(_legacy_order ON)
      endif()
    endforeach()

    # at least one found upstream module was not included into DUNE_FOUND_DEPENDENCIES (config file pre 2.10)
    # so we reconstruct it from ALL_DEPENDENCIES
    if (_legacy_order)
      set(DUNE_FOUND_DEPENDENCIES ${ALL_DEPENDENCIES})
      # remove not found modules
      foreach(_mod IN LISTS ALL_DEPENDENCIES)
        if (NOT ${_mod}_FOUND)
          list(REMOVE_ITEM DUNE_FOUND_DEPENDENCIES ${_mod})
        endif()
      endforeach()
    endif()
    set_property(GLOBAL PROPERTY DUNE_DISABLE_ALL_DEPENDENCIES_DEPRECATION_WARNING OFF)
  else()
    message(AUTHOR_WARNING "This needs to be removed!")
  endif()

  # assert the project names matches
  if(NOT (ProjectName STREQUAL PROJECT_NAME))
    message(FATAL_ERROR "Module name from dune.module does not match the name given in CMakeLists.txt.")
  endif()

  # As default request position independent code if shared libraries are built
  # This should allow DUNE modules to use CMake's object libraries.
  # This can be overwritten for targets by setting the target property
  # POSITION_INDEPENDENT_CODE to false/OFF
  include(CMakeDependentOption)
  cmake_dependent_option(CMAKE_POSITION_INDEPENDENT_CODE "Build position independent code" ON "NOT BUILD_SHARED_LIBS" ON)

  # check for C++ features, set compiler flags for C++14 or C++11 mode
  include(CheckCXXFeatures)

  # set include path and link path for the current project.
  dune_policy(GET DP_DEFAULT_INCLUDE_DIRS _include_policy)
  if(_include_policy STREQUAL "OLD")
    include_directories("${PROJECT_SOURCE_DIR}")
    include_directories("${PROJECT_BINARY_DIR}")
    include_directories("${PROJECT_BINARY_DIR}/include")
    include_directories("${PROJECT_BINARY_DIR}/include_private")
  endif()
  unset(_include_policy)
  add_definitions(-DHAVE_CONFIG_H)

  # Create custom target for building the documentation
  # and provide macros for installing the docs and force
  # building them before.
  include(DuneDoc)

  # make sure that cmake paths of all modules are correctly setup
  dune_cmake_path_setup(DUNE_FOUND_DEPENDENCIES)

  # Process the macros provided by the dependencies and ourself
  dune_process_dependency_macros()

  include(GNUInstallDirs)
  # Set variable where the cmake modules will be installed.
  # Thus the user can override it and for example install
  # directly into the CMake installation. We use a cache variable
  # that is overridden by a local variable of the same name if
  # the user does not explicitly set a value for it. Thus the value
  # will automatically change if the user changes CMAKE_INSTALL_DATAROOTDIR
  # or CMAKE_INSTALL_PREFIX
  if(NOT DUNE_INSTALL_MODULEDIR)
    set(DUNE_INSTALL_MODULEDIR ""
      CACHE PATH
      "Installation directory for CMake modules. Default is \${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/modules when not set explicitly")
    set(DUNE_INSTALL_MODULEDIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/modules)
  endif()
  if(NOT DUNE_INSTALL_NONOBJECTLIBDIR)
    set(DUNE_INSTALL_NONOBJECTLIBDIR ""
      CACHE PATH
      "Installation directory for libraries that are not architecture dependent. Default is lib when not set explicitly")
    set(DUNE_INSTALL_NONOBJECTLIBDIR lib)
  endif()
  # set up make headercheck
  include(Headercheck)
  setup_headercheck()

  # define that we found this module
  set(${ProjectName}_FOUND 1)

endmacro(dune_project)


# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates config.h and the cmake-config files,
# some install directives and exports the module.
macro(finalize_dune_project)
  if(DUNE_SYMLINK_TO_SOURCE_TREE)
    dune_symlink_to_source_tree()
  endif()

  #configure all headerchecks
  finalize_headercheck()

  ##########################
  ### CREATE CONFIG FILE ###
  ##########################

  # deprecation warning introduced with Dune 2.10
  if(NOT "${ARGC}" EQUAL 0)
    message(DEPRECATION "Passing arguments to finalize_dune_project is no longer needed.")
  endif()

  #create cmake-config files for installation tree
  include(CMakePackageConfigHelpers)
  include(GNUInstallDirs)
  set(DOXYSTYLE_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune-common/doc/doxygen/)
  set(SCRIPT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
  # Set the location where the doc sources are installed.
  # Needed by custom package configuration
  # file section of dune-grid.
  set(DUNE_MODULE_SRC_DOCDIR "\${${ProjectName}_PREFIX}/${CMAKE_INSTALL_DOCDIR}")
  get_property(${ProjectName}_LIBRARIES GLOBAL PROPERTY ${ProjectName}_LIBRARIES)
  get_property(${ProjectName}_INTERFACE_LIBRARIES GLOBAL PROPERTY ${ProjectName}_INTERFACE_LIBRARIES)

  if(${ProjectName} STREQUAL "dune-common")
  string(JOIN "\n" DUNE_CUSTOM_PKG_CONFIG_SECTION ${DUNE_CUSTOM_PKG_CONFIG_SECTION}
    [[set_and_check(@DUNE_MOD_NAME@_SCRIPT_DIR "@PACKAGE_SCRIPT_DIR@")]]
    [[set_and_check(DOXYSTYLE_FILE "@PACKAGE_DOXYSTYLE_DIR@/Doxystyle")]]
    [[set_and_check(DOXYGENMACROS_FILE "@PACKAGE_DOXYSTYLE_DIR@/doxygen-macros")]]
  )

endif()

  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/cmake/pkg/${ProjectName}-config.cmake.in)
    if(NOT (${ProjectName} STREQUAL "dune-common"))
      # set up dune dependencies
      set(DUNE_DEPENDENCY_HEADER
"macro(find_and_check_dune_dependency module version)
  find_dependency(\${module})
  list(PREPEND CMAKE_MODULE_PATH \"\${dune-common_MODULE_PATH}\")
  include(DuneModuleDependencies)
  list(POP_FRONT CMAKE_MODULE_PATH)
  if(dune-common_VERSION VERSION_GREATER_EQUAL \"2.10\")
    dune_check_module_version(\${module} QUIET REQUIRED VERSION \"\${version}\")
  endif()
endmacro()
")
      foreach(_mod IN LISTS ${ProjectName}_SUGGESTS ${ProjectName}_DEPENDS)
        split_module_version(${_mod} _mod_name _mod_ver)
        if(${_mod_name}_FOUND)
          set(DUNE_DEPENDENCY_HEADER
"${DUNE_DEPENDENCY_HEADER}
find_and_check_dune_dependency(${_mod_name} \"${_mod_ver}\")")
        endif()
      endforeach()
    endif()
    dune_module_to_uppercase(_upcase_module "${ProjectName}")
    # Generate a standard cmake package configuration file
    file(WRITE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config.cmake.in
"if(NOT ${ProjectName}_FOUND)
# Whether this module is installed or not
set(${ProjectName}_INSTALLED @MODULE_INSTALLED@)

# Settings specific to the module
@${ProjectName}_INIT@
# Package initialization
@PACKAGE_INIT@

#report other information
set_and_check(${ProjectName}_PREFIX \"\${PACKAGE_PREFIX_DIR}\")
set_and_check(${ProjectName}_INCLUDE_DIRS \"@PACKAGE_CMAKE_INSTALL_INCLUDEDIR@\")
set(${ProjectName}_CMAKE_CONFIG_VERSION \"${DUNE_COMMON_VERSION}\")
set(${ProjectName}_CXX_FLAGS \"${CMAKE_CXX_FLAGS}\")
set(${ProjectName}_CXX_FLAGS_DEBUG \"${CMAKE_CXX_FLAGS_DEBUG}\")
set(${ProjectName}_CXX_FLAGS_MINSIZEREL \"${CMAKE_CXX_FLAGS_MINSIZEREL}\")
set(${ProjectName}_CXX_FLAGS_RELEASE \"${CMAKE_CXX_FLAGS_RELEASE}\")
set(${ProjectName}_CXX_FLAGS_RELWITHDEBINFO \"${CMAKE_CXX_FLAGS_RELWITHDEBINFO}\")
set(${ProjectName}_DEPENDS \"@${ProjectName}_DEPENDS@\")
set(${ProjectName}_SUGGESTS \"@${ProjectName}_SUGGESTS@\")
set(${ProjectName}_MODULE_PATH \"@PACKAGE_DUNE_INSTALL_MODULEDIR@\")
set(${ProjectName}_PYTHON_WHEELHOUSE \"@PACKAGE_DUNE_PYTHON_WHEELHOUSE@\")
set(${ProjectName}_LIBRARIES \"@${ProjectName}_LIBRARIES@\")
set(${ProjectName}_HASPYTHON @DUNE_MODULE_HASPYTHON@)
set(${ProjectName}_PYTHONREQUIRES \"@DUNE_MODULE_PYTHONREQUIRES@\")

# Resolve dune dependencies
include(CMakeFindDependencyMacro)
${DUNE_DEPENDENCY_HEADER}

# Set up DUNE_LIBS, DUNE_FOUND_DEPENDENCIES, DUNE_*_FOUND, and HAVE_* variables
if(${ProjectName}_LIBRARIES)
  message(STATUS \"Setting ${ProjectName}_LIBRARIES=\${${ProjectName}_LIBRARIES}\")
  list(PREPEND DUNE_LIBS \${${ProjectName}_LIBRARIES})
endif()
list(APPEND DUNE_FOUND_DEPENDENCIES ${ProjectName})
set(DUNE_${ProjectName}_FOUND TRUE)
set(HAVE_${_upcase_module} TRUE)

# Lines that are set by the CMake build system via the variable DUNE_CUSTOM_PKG_CONFIG_SECTION
${DUNE_CUSTOM_PKG_CONFIG_SECTION}

# If this file is found in a super build that includes ${ProjectName}, the
# `${ProjectName}-targets.cmake`-file has not yet been generated. This variable
# determines whether the configuration of ${ProjectName} has been completed.
get_property(${ProjectName}_IN_CONFIG_MODE GLOBAL PROPERTY ${ProjectName}_LIBRARIES DEFINED)

#import the target
if(${ProjectName}_LIBRARIES AND NOT ${ProjectName}_IN_CONFIG_MODE)
  get_filename_component(_dir \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
  include(\"\${_dir}/${ProjectName}-targets.cmake\")
endif()

endif()")
      set(CONFIG_SOURCE_FILE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config.cmake.in)
  else()
    set(CONFIG_SOURCE_FILE ${PROJECT_SOURCE_DIR}/cmake/pkg/${ProjectName}-config.cmake.in)
  endif()
  get_property(${ProjectName}_LIBRARIES GLOBAL PROPERTY ${ProjectName}_LIBRARIES)
  get_property(${ProjectName}_EXPORTED_LIBRARIES GLOBAL PROPERTY ${ProjectName}_EXPORTED_LIBRARIES)

  # DUNE_INSTALL_LIBDIR is deprecated and used here to generate a proper deprecation warnings.
  # After Dune 2.10 CMAKE_INSTALL_LIBDIR has to be used, DUNE_INSTALL_LIBDIR will be removed.
  if(DEFINED DUNE_INSTALL_LIBDIR)
    message(DEPRECATION "DUNE_INSTALL_LIBDIR is deprecated, use CMAKE_INSTALL_LIBDIR instead")
  else()
    set(DUNE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
  endif()

  # Set the location of the doc file source. Needed by custom package configuration
  # file section of dune-grid.
  set(DUNE_MODULE_SRC_DOCDIR "${PROJECT_SOURCE_DIR}/doc")
  set(MODULE_INSTALLED ON)
  if(EXISTS ${PROJECT_SOURCE_DIR}/python)
    set(DUNE_MODULE_HASPYTHON 1)
    set(DUNE_MODULE_PYTHONREQUIRES "${ProjectPythonRequires}")
  else()
    set(DUNE_MODULE_HASPYTHON 0)
    set(DUNE_MODULE_PYTHONREQUIRES "")
  endif()

  if(NOT DEFINED DUNE_PYTHON_WHEELHOUSE)
    set(DUNE_PYTHON_WHEELHOUSE "")
  endif()

  configure_package_config_file(${CONFIG_SOURCE_FILE}
    ${PROJECT_BINARY_DIR}/cmake/pkg/${ProjectName}-config.cmake
    INSTALL_DESTINATION  ${DUNE_INSTALL_LIBDIR}/cmake/${ProjectName}
    PATH_VARS CMAKE_INSTALL_DATAROOTDIR DUNE_INSTALL_MODULEDIR CMAKE_INSTALL_INCLUDEDIR DUNE_PYTHON_WHEELHOUSE
    DOXYSTYLE_DIR SCRIPT_DIR)


  #create cmake-config files for build tree
  set(PACKAGE_CMAKE_INSTALL_INCLUDEDIR ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include)
  set(PACKAGE_CMAKE_INSTALL_DATAROOTDIR ${PROJECT_BINARY_DIR})
  set(PACKAGE_DOXYSTYLE_DIR ${PROJECT_SOURCE_DIR}/doc/doxygen)
  set(PACKAGE_SCRIPT_DIR ${PROJECT_SOURCE_DIR}/cmake/scripts)
  set(PACKAGE_DUNE_INSTALL_MODULEDIR ${PROJECT_SOURCE_DIR}/cmake/modules)
  set(PACKAGE_DUNE_PYTHON_WHEELHOUSE ${PROJECT_BINARY_DIR}/python)
  set(PACKAGE_PREFIX_DIR ${PROJECT_BINARY_DIR})
  set(PACKAGE_INIT "# Set prefix to source dir
set(PACKAGE_PREFIX_DIR ${PROJECT_SOURCE_DIR})
macro(set_and_check _var _file)
  set(\${_var} \"\${_file}\")
  if(\"_\${_file}_\" STREQUAL \"__\")
    message(FATAL_ERROR \"File or directory referenced by variable \${_var} is unset !\")
  endif()
  foreach(_f \${_file})
    if(NOT EXISTS \"\${_f}\")
      message(FATAL_ERROR \"File or directory \${_f} referenced by variable \${_var} does not exist !\")
    endif()
  endforeach(_f)
endmacro()")
  set(MODULE_INSTALLED OFF)

  configure_file(
    ${CONFIG_SOURCE_FILE}
    ${PROJECT_BINARY_DIR}/${ProjectName}-config.cmake @ONLY)

  ###########################
  ### CREATE VERSION FILE ###
  ###########################

  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/${ProjectName}-config-version.cmake.in)
    file(WRITE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config-version.cmake.in
"set(PACKAGE_VERSION \"${ProjectVersionString}\")

if(\"\${PACKAGE_FIND_VERSION_MAJOR}\" EQUAL \"${ProjectVersionMajor}\" AND
     \"\${PACKAGE_FIND_VERSION_MINOR}\" EQUAL \"${ProjectVersionMinor}\")
  set (PACKAGE_VERSION_COMPATIBLE 1) # compatible with newer
  if (\"\${PACKAGE_FIND_VERSION}\" VERSION_EQUAL \"${ProjectVersionString}\")
    set(PACKAGE_VERSION_EXACT 1) #exact match for this version
  endif()
endif()
")
    set(CONFIG_VERSION_FILE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config-version.cmake.in)
  else()
    set(CONFIG_VERSION_FILE ${PROJECT_SOURCE_DIR}/${ProjectName}-config-version.cmake.in)
  endif()
  configure_file(
    ${CONFIG_VERSION_FILE}
    ${PROJECT_BINARY_DIR}/${ProjectName}-config-version.cmake @ONLY)

  ###########################
  ### CREATE TARGETS FILE ###
  ###########################

  # find all namespaces (this was setup in dune_add_library)
  set(_namespaces "")
  foreach(_target ${${ProjectName}_EXPORTED_LIBRARIES})
    # find namespaces: alias without export name
    get_target_property(_export_name ${_target} EXPORT_NAME)
    string(FIND "${_target}" "${_export_name}" _pos_export_name REVERSE)
    string(SUBSTRING "${_target}" 0 ${_pos_export_name} _namespace)
    list(APPEND _namespaces ${_namespace})
  endforeach()
  list(REMOVE_DUPLICATES _namespaces)

  # install files with targets, one per export set
  set(_glob_target_file "
get_filename_component(_dir \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
")
  foreach(_namespace ${_namespaces})
    set(_export_set ${ProjectName}-${_namespace}-export-set)
    string(MD5 _hash "${_export_set}")
    set(_target_file  ${ProjectName}-scoped-targets-${_hash}.cmake)

    # install library export set
    install(EXPORT ${_export_set}
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ProjectName}
      NAMESPACE ${_namespace}
      FILE ${_target_file})

    # export libraries for use in build tree
    export(EXPORT ${_export_set}
      FILE ${PROJECT_BINARY_DIR}/${_target_file}
      NAMESPACE ${_namespace})

    set(_glob_target_file "${_glob_target_file}
include(\"\${_dir}/${_target_file}\")")
  endforeach()

  # add deprecated unaliased targets and warnings (remove after dune 2.12)
  set(${ProjectName}_POLICY_UNSCOPED_EXPORTED_TARGET_VISIBILITY TRUE CACHE BOOL
    "If this policy is set to FALSE, unscoped export names will not be visible in downstream projects.
    This was the default behavior previous to DUNE 2.10.
    Set this variable to FALSE only if you do not want compatibility with Dune 2.9 or earlier.
    The old behavior will be completely removed after Dune 2.12")
  mark_as_advanced({ProjectName}_POLICY_UNSCOPED_EXPORTED_TARGET_VISIBILITY)
  if(${ProjectVersionString} VERSION_LESS_EQUAL 2.12)
    foreach(_interface_name ${${ProjectName}_LIBRARIES})
      # alias with original target name (e.g. dunecommon)
      get_target_property(_unaliased_name ${_interface_name} ALIASED_TARGET)
      if(NOT "${_unaliased_name}" STREQUAL "${_interface_name}")
        set(_glob_target_file "${_glob_target_file}
add_library(${_unaliased_name} INTERFACE IMPORTED)
set_target_properties(${_unaliased_name} PROPERTIES INTERFACE_LINK_LIBRARIES ${_interface_name})")
        if(${ProjectVersionString} VERSION_EQUAL 2.12)
          set(_glob_target_file "${_glob_target_file}
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
  set_property(TARGET ${_unaliased_name} PROPERTY DEPRECATION \"Replace `${_unaliased_name}` to new scoped `${_interface_name}` targets.\")
endif()")
        endif()
      endif()

      # alias with unscoped export name (e.g. Common instead of Dune::Common)
      get_target_property(_export_name ${_interface_name} EXPORT_NAME)
      if(     (NOT "${_export_name}" STREQUAL "${_interface_name}")
          AND (NOT "${_export_name}" STREQUAL "${_unaliased_name}")
          AND ${${ProjectName}_POLICY_UNSCOPED_EXPORTED_TARGET_VISIBILITY})
        set(_glob_target_file "${_glob_target_file}
add_library(${_export_name} INTERFACE IMPORTED)
set_target_properties(${_export_name} PROPERTIES INTERFACE_LINK_LIBRARIES ${_interface_name})")
        if(${ProjectVersionString} VERSION_EQUAL 2.12)
          set(_glob_target_file "${_glob_target_file}
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
  set_property(TARGET ${_export_name} PROPERTY DEPRECATION \"Replace `${_export_name}` to new scoped `${_interface_name}` targets.\")
endif()")
        endif()
      endif()
    endforeach()
  elseif(${ProjectName} STREQUAL "dune-common")
    message(WARNING "TODO: Remove this and code above once DUNE 2.12 is released -Unscoped exported targets is fully unsupported-")
  endif()

  # write targets file
  file(WRITE ${PROJECT_BINARY_DIR}/${ProjectName}-targets.cmake ${_glob_target_file})

  ###########################
  ### INSTALL CMAKE FILEs ###
  ###########################

  # install dune.module file
  install(FILES dune.module DESTINATION ${DUNE_INSTALL_NONOBJECTLIBDIR}/dunecontrol/${ProjectName})

  # install cmake-config files
  install(FILES
    ${PROJECT_BINARY_DIR}/cmake/pkg/${ProjectName}-config.cmake
    ${PROJECT_BINARY_DIR}/${ProjectName}-targets.cmake
    ${PROJECT_BINARY_DIR}/${ProjectName}-config-version.cmake
    DESTINATION ${DUNE_INSTALL_LIBDIR}/cmake/${ProjectName})

  # install config.h
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    install(FILES config.h.cmake DESTINATION share/${ProjectName})
  endif()

  # install pkg-config files
  create_and_install_pkconfig(${DUNE_INSTALL_LIBDIR})

  ###########################
  ### HEADER CONFIG FILEs ###
  ###########################

  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    set(CONFIG_H_CMAKE ${PROJECT_SOURCE_DIR}/config.h.cmake)
  else()
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake)
    set(CONFIG_H_CMAKE ${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake)
  endif()

  # generate and install headers file: ${ProjectName}-config.hh and ${ProjectName}-config-private.hh
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/)
  dune_parse_module_config_file(${ProjectName} FILE ${CONFIG_H_CMAKE})

  # configure private config file
  file(WRITE ${PROJECT_BINARY_DIR}/include_private/${ProjectName}-config-private.hh.cmake "${${ProjectName}_CONFIG_PRIVATE_HH}")
  # parse again dune.module file of current module to set PACKAGE_* variables
  dune_module_information(${PROJECT_SOURCE_DIR} QUIET)
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/include_private/${ProjectName}-config-private.hh.cmake ${CMAKE_CURRENT_BINARY_DIR}/include_private/${ProjectName}-config-private.hh)

  # configure and install public config file
  file(WRITE ${PROJECT_BINARY_DIR}/include/${ProjectName}-config.hh.cmake "${${ProjectName}_CONFIG_HH}\n${${ProjectName}_CONFIG_BOTTOM_HH}")
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/include/${ProjectName}-config.hh.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/${ProjectName}-config.hh)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/${ProjectName}-config.hh DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  message(STATUS "Adding custom target for config.h generation")
  dune_regenerate_config_cmake()
  # add a target to generate config.h.cmake
  if(NOT TARGET OUTPUT)
    add_custom_target(OUTPUT config_collected.h.cmake
      COMMAND dune_regenerate_config_cmake())
  endif()
  # actually write the config.h file to disk
  # using generated file
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/config_collected.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

  if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)
  endif()

endmacro(finalize_dune_project)


# ------------------------------------------------------------------------
# Internal macros
# ------------------------------------------------------------------------

# parse config.h.cmake file and return public, private and bottom parts of the header
# as ${module}_CONFIG_HH, ${module}_CONFIG_PRIVATE_HH, and ${module}_CONFIG_BOTTOM_HH
function(dune_parse_module_config_file module)
  cmake_parse_arguments(ARG "" "FILE" "" ${ARGN})
  dune_module_to_uppercase(upper ${module})

  file(READ "${ARG_FILE}" _config)
  string(REGEX REPLACE
    ".*/\\*[ ]*begin[ ]+${module}[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*${module}[^\\*]*\\*/" "\\1"
    _tfile "${_config}")
  # extract the private section
  string(REGEX MATCH "/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/" _tprivate "${_tfile}")
  string(REGEX REPLACE ".*/\\*[ ]*begin[ ]+private[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*private[^\\*]*\\*/" "\\1" _config_private "${_tprivate}" )
  string(REGEX REPLACE "(.*)/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/(.*)" "\\1\\2" _ttfile "${_tfile}")

  # extract the bottom section
  string(REGEX MATCH "/[\\*][ ]*begin bottom.*/[\\*][ ]*end[ ]*bottom[^\\*]\\*/" _tbottom "${_ttfile}")
  string(REGEX REPLACE ".*/\\*[ ]*begin[ ]+bottom[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*bottom[^\\*]*\\*/" "\\1" _config_bottom "${_tbottom}" )
  string(REGEX REPLACE "(.*)/[\\*][ ]*begin bottom.*/[\\*][ ]*end[ ]*bottom[^\\*]\\*/(.*)" "\\1\\2" _config  "${_ttfile}")

  # resolve headers of dependencies
  unset(_headers)
  foreach(_deps ${${module}_DEPENDS} ${${module}_SUGGESTS})
    split_module_version(${_deps} dep_name dep_version)
    set(_headers "${_headers}
#if __has_include(<${dep_name}-config.hh>)
  #include <${dep_name}-config.hh>
#endif
")
  endforeach()

  # resolve public header part
  set(HAVE_${upper} ${${module}_FOUND} PARENT_SCOPE)
  set(${module}_CONFIG_HH
"
#ifndef ${upper}_CONFIG_HH
#define ${upper}_CONFIG_HH

/* Define to 1 if you have module ${module} available */
#ifndef HAVE_${upper}
#cmakedefine01 HAVE_${upper}
#endif

${_config}

${_headers}

#endif // ${upper}_CONFIG_HH
" PARENT_SCOPE)

  # resolve private header part
  set(${module}_CONFIG_PRIVATE_HH
"
#ifndef ${upper}_CONFIG_PRIVATE_HH
#define ${upper}_CONFIG_PRIVATE_HH

${_config_private}

#include <${module}-config.hh>

#endif // ${upper}_CONFIG_PRIVATE_HH
" PARENT_SCOPE)

  # resolve bottom header part
  # TODO: when legacy config file is removed private and bottom parts can be
  # merged into one, with dependency headers in between
  set(${module}_CONFIG_BOTTOM_HH
"
#ifndef ${upper}_CONFIG_BOTTOM_HH
#define ${upper}_CONFIG_BOTTOM_HH

${_config_bottom}

#endif // ${upper}_CONFIG_BOTTOM_HH
" PARENT_SCOPE)
endfunction()



# Creates a new config_collected.h.cmake file in ${CMAKE_CURRENT_BINARY_DIR} that
# consists of entries from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
# and includes non-private entries from the config.h.cmake files
# of all dependent modules. Finally config.h is created from config_collected.h.cmake.
macro(dune_regenerate_config_cmake)

  # collect header parts from all dependencies
  unset(collected_config_file)
  unset(collected_config_file_bottom)
  foreach(_dep ${DUNE_FOUND_DEPENDENCIES})
    if(EXISTS ${${_dep}_PREFIX}/config.h.cmake)
      set(CONFIG_H_CMAKE ${${_dep}_PREFIX}/config.h.cmake)
    elseif(EXISTS ${${_dep}_PREFIX}/share/${_dep}/config.h.cmake)
      set(CONFIG_H_CMAKE ${${_dep}_PREFIX}/share/${_dep}/config.h.cmake)
    else()
      file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake)
      set(CONFIG_H_CMAKE ${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake)
    endif()

    dune_parse_module_config_file(${_dep} FILE ${CONFIG_H_CMAKE})

    set(collected_config_file        "${collected_config_file}\n${${_dep}_CONFIG_HH}\n")
    set(collected_config_file_bottom "${collected_config_file_bottom}\n${${_dep}_CONFIG_BOTTOM_HH}\n")
  endforeach()

  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    set(CONFIG_H_CMAKE ${PROJECT_SOURCE_DIR}/config.h.cmake)
  else()
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake)
    set(CONFIG_H_CMAKE ${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake)
  endif()

  # collect header parts from this module
  dune_parse_module_config_file(${ProjectName} FILE ${CONFIG_H_CMAKE})

  # write collected header into config.h.cmake
  file(WRITE ${PROJECT_BINARY_DIR}/config_collected.h.cmake "
${collected_config_file}
${${ProjectName}_CONFIG_HH}
${${ProjectName}_CONFIG_PRIVATE_HH}
${collected_config_file_bottom}
${${ProjectName}_CONFIG_BOTTOM_HH}
")

endmacro(dune_regenerate_config_cmake)
