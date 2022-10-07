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

    finalize_dune_project([<generate-config-h>])

  This function needs to be run at the end of every top-level
  ``CMakeLists.txt`` file. Among other things it creates the cmake package
  configuration file and package version file. Modules can add additional
  entries to these files by setting the variable ``${ProjectName}_INIT``.

  If any argument ``<generate-config-h>`` is passed to this macro, a
  ``config.h`` file is created containing preprocessor definitions about found
  packages and module information.

#]=======================================================================]
include_guard(GLOBAL)

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(DuneModuleDependencies)
include(DuneModuleInformation)
include(DuneSymlinkOrCopy)
include(FeatureSummary)
include(GNUInstallDirs)
include(Headercheck)
include(OverloadCompilerFlags)


# Macro that should be called near the beginning of the top level CMakeLists.txt.
# Namely it sets up the module, defines basic variables and manages
# dependencies.
# Don't forget to call finalize_dune_project afterwards.
macro(dune_project)

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
        FULL_DOCS "List of libraries of the module. Used to generate CMake's package configuration files. DO NOT EDIT!")
  dune_create_dependency_tree()

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
  include_directories("${PROJECT_BINARY_DIR}")
  include_directories("${PROJECT_SOURCE_DIR}")
  include_directories("${CMAKE_CURRENT_BINARY_DIR}")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
  add_definitions(-DHAVE_CONFIG_H)

  # Create custom target for building the documentation
  # and provide macros for installing the docs and force
  # building them before.
  include(DuneDoc)

  # activate pkg-config
  include(DunePkgConfig)

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

  #create cmake-config files for installation tree
  include(CMakePackageConfigHelpers)
  include(GNUInstallDirs)
  set(DOXYSTYLE_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune-common/doc/doxygen/)
  set(SCRIPT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
  # Set the location where the doc sources are installed.
  # Needed by custom package configuration
  # file section of dune-grid.
  set(DUNE_MODULE_SRC_DOCDIR "\${${ProjectName}_PREFIX}/${CMAKE_INSTALL_DOCDIR}")

  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/cmake/pkg/${ProjectName}-config.cmake.in)
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
set(${ProjectName}_CXX_FLAGS \"${CMAKE_CXX_FLAGS}\")
set(${ProjectName}_CXX_FLAGS_DEBUG \"${CMAKE_CXX_FLAGS_DEBUG}\")
set(${ProjectName}_CXX_FLAGS_MINSIZEREL \"${CMAKE_CXX_FLAGS_MINSIZEREL}\")
set(${ProjectName}_CXX_FLAGS_RELEASE \"${CMAKE_CXX_FLAGS_RELEASE}\")
set(${ProjectName}_CXX_FLAGS_RELWITHDEBINFO \"${CMAKE_CXX_FLAGS_RELWITHDEBINFO}\")
set(${ProjectName}_DEPENDS \"@${ProjectName}_DEPENDS@\")
set(${ProjectName}_SUGGESTS \"@${ProjectName}_SUGGESTS@\")
set(${ProjectName}_MODULE_PATH \"@PACKAGE_DUNE_INSTALL_MODULEDIR@\")
set(${ProjectName}_LIBRARIES \"@${ProjectName}_LIBRARIES@\")
set(${ProjectName}_HASPYTHON @DUNE_MODULE_HASPYTHON@)
set(${ProjectName}_PYTHONREQUIRES \"@DUNE_MODULE_PYTHONREQUIRES@\")

# Lines that are set by the CMake build system via the variable DUNE_CUSTOM_PKG_CONFIG_SECTION
${DUNE_CUSTOM_PKG_CONFIG_SECTION}

#import the target
if(${ProjectName}_LIBRARIES)
  get_filename_component(_dir \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
  include(\"\${_dir}/${ProjectName}-targets.cmake\")
endif()

endif()")
      set(CONFIG_SOURCE_FILE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config.cmake.in)
  else()
    set(CONFIG_SOURCE_FILE ${PROJECT_SOURCE_DIR}/cmake/pkg/${ProjectName}-config.cmake.in)
  endif()
  get_property(${ProjectName}_LIBRARIES GLOBAL PROPERTY ${ProjectName}_LIBRARIES)

  # compute under which libdir the package configuration files are to be installed.
  # If the module installs an object library we use CMAKE_INSTALL_LIBDIR
  # to capture the multiarch triplet of Debian/Ubuntu.
  # Otherwise we fall back to DUNE_INSTALL_NONOBJECTLIB which is lib
  # if not set otherwise.
  get_property(${ProjectName}_LIBRARIES GLOBAL PROPERTY ${ProjectName}_LIBRARIES)
  if(${ProjectName}_LIBRARIES)
    set(DUNE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
  else()
    set(DUNE_INSTALL_LIBDIR ${DUNE_INSTALL_NONOBJECTLIBDIR})
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

  configure_package_config_file(${CONFIG_SOURCE_FILE}
    ${PROJECT_BINARY_DIR}/cmake/pkg/${ProjectName}-config.cmake
    INSTALL_DESTINATION  ${DUNE_INSTALL_LIBDIR}/cmake/${ProjectName}
    PATH_VARS CMAKE_INSTALL_DATAROOTDIR DUNE_INSTALL_MODULEDIR CMAKE_INSTALL_INCLUDEDIR
    DOXYSTYLE_DIR SCRIPT_DIR)


  #create cmake-config files for build tree
  set(PACKAGE_CMAKE_INSTALL_INCLUDEDIR ${PROJECT_SOURCE_DIR})
  set(PACKAGE_CMAKE_INSTALL_DATAROOTDIR ${PROJECT_BINARY_DIR})
  set(PACKAGE_DOXYSTYLE_DIR ${PROJECT_SOURCE_DIR}/doc/doxygen)
  set(PACKAGE_SCRIPT_DIR ${PROJECT_SOURCE_DIR}/cmake/scripts)
  set(PACKAGE_DUNE_INSTALL_MODULEDIR ${PROJECT_SOURCE_DIR}/cmake/modules)
  set(PACKAGE_PREFIX_DIR ${PROJECT_BINARY_DIR})
  set(PACKAGE_INIT "# Set prefix to source dir
set(PACKAGE_PREFIX_DIR ${PROJECT_SOURCE_DIR})
macro(set_and_check _var _file)
  set(\${_var} \"\${_file}\")
  if(NOT EXISTS \"\${_file}\")
    message(FATAL_ERROR \"File or directory \${_file} referenced by variable \${_var} does not exist !\")
  endif()
endmacro()")
  set(MODULE_INSTALLED OFF)

  configure_file(
    ${CONFIG_SOURCE_FILE}
    ${PROJECT_BINARY_DIR}/${ProjectName}-config.cmake @ONLY)

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

  # install dune.module file
  install(FILES dune.module DESTINATION ${DUNE_INSTALL_NONOBJECTLIBDIR}/dunecontrol/${ProjectName})

  # install cmake-config files
  install(FILES ${PROJECT_BINARY_DIR}/cmake/pkg/${ProjectName}-config.cmake
    ${PROJECT_BINARY_DIR}/${ProjectName}-config-version.cmake
    DESTINATION ${DUNE_INSTALL_LIBDIR}/cmake/${ProjectName})

  # install config.h
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    install(FILES config.h.cmake DESTINATION share/${ProjectName})
  endif()

  # install pkg-config files
  create_and_install_pkconfig(${DUNE_INSTALL_LIBDIR})

  if(${ProjectName}_EXPORT_SET)
    # install library export set
    install(EXPORT ${${ProjectName}_EXPORT_SET}
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ProjectName})

    # export libraries for use in build tree
    export(EXPORT ${${ProjectName}_EXPORT_SET}
      FILE ${PROJECT_BINARY_DIR}/${ProjectName}-targets.cmake)
  endif()

  if("${ARGC}" EQUAL "1")
    message(STATUS "Adding custom target for config.h generation")
    dune_regenerate_config_cmake()
    # add a target to generate config.h.cmake
    if(NOT TARGET OUTPUT)
      add_custom_target(OUTPUT config_collected.h.cmake
        COMMAND dune_regenerate_config_cmake())
    endif()
    # actually write the config.h file to disk
    # using generated file
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/config_collected.h.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  else()
    message(STATUS "Not adding custom target for config.h generation")
    # actually write the config.h file to disk
    configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  endif()

  if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    feature_summary(WHAT ALL)
  endif()

  # check if CXX flag overloading has been enabled
  # and write compiler script (see OverloadCompilerFlags.cmake)
  finalize_compiler_script()
endmacro(finalize_dune_project)


# ------------------------------------------------------------------------
# Internal macros
# ------------------------------------------------------------------------


# Creates a new config_collected.h.cmake file in ${CMAKE_CURRENT_BINARY_DIR} that
# consists of entries from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
# and includes non-private entries from the config.h.cmake files
# of all dependent modules. Finally config.h is created from config_collected.h.cmake.
macro(dune_regenerate_config_cmake)
  set(CONFIG_H_CMAKE_FILE "${PROJECT_BINARY_DIR}/config_collected.h.cmake")
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    file(READ ${PROJECT_SOURCE_DIR}/config.h.cmake _file)
    string(REGEX MATCH
      "/[\\*/][ ]*begin[ ]+${ProjectName}.*\\/[/\\*][ ]*end[ ]*${ProjectName}[^\\*]*\\*/"
      _myfile "${_file}")
  endif()
  # overwrite file with new content
  file(WRITE ${CONFIG_H_CMAKE_FILE} "/* config.h.  Generated from config_collected.h.cmake by CMake.
   It was generated from config_collected.h.cmake which in turn is generated automatically
   from the config.h.cmake files of modules this module depends on. */"
   )

  foreach(_dep ${ProjectName} ${ALL_DEPENDENCIES})
    dune_module_to_uppercase(upper ${_dep})
    set(HAVE_${upper} ${${_dep}_FOUND})
    file(APPEND ${CONFIG_H_CMAKE_FILE}
      "\n\n/* Define to 1 if you have module ${_dep} available */
#cmakedefine01 HAVE_${upper}\n")
  endforeach()

  # add previous module specific section
  foreach(_dep ${ALL_DEPENDENCIES})
    foreach(_mod_conf_file ${${_dep}_PREFIX}/config.h.cmake
        ${${_dep}_PREFIX}/share/${_dep}/config.h.cmake)
      if(EXISTS ${_mod_conf_file})
        file(READ "${_mod_conf_file}" _file)
        string(REGEX REPLACE
          ".*/\\*[ ]*begin[ ]+${_dep}[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*${_dep}[^\\*]*\\*/" "\\1"
          _tfile "${_file}")
        # strip the private section
        string(REGEX REPLACE "(.*)/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/(.*)" "\\1\\2" _ttfile "${_tfile}")

        # extract the bottom section
        string(REGEX MATCH "/[\\*][ ]*begin bottom.*/[\\*][ ]*end[ ]*bottom[^\\*]\\*/" _tbottom "${_ttfile}")
        string(REGEX REPLACE ".*/\\*[ ]*begin[ ]+bottom[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*bottom[^\\*]*\\*/" "\\1" ${_dep}_CONFIG_H_BOTTOM "${_tbottom}" )
        string(REGEX REPLACE "(.*)/[\\*][ ]*begin bottom.*/[\\*][ ]*end[ ]*bottom[^\\*]\\*/(.*)" "\\1\\2" _file  "${_ttfile}")

        # append bottom section
        if(${_dep}_CONFIG_H_BOTTOM)
          set(CONFIG_H_BOTTOM "${CONFIG_H_BOTTOM} ${${_dep}_CONFIG_H_BOTTOM}")
        endif()

        file(APPEND ${CONFIG_H_CMAKE_FILE} "${_file}")
      endif()
    endforeach()
  endforeach()
  # parse again dune.module file of current module to set PACKAGE_* variables
  dune_module_information(${PROJECT_SOURCE_DIR} QUIET)
  file(APPEND ${CONFIG_H_CMAKE_FILE} "\n${_myfile}")
  # append CONFIG_H_BOTTOM section at the end if found
  if(CONFIG_H_BOTTOM)
    file(APPEND ${CONFIG_H_CMAKE_FILE} "${CONFIG_H_BOTTOM}")
  endif()
endmacro(dune_regenerate_config_cmake)
