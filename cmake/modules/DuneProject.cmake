#[=======================================================================[.rst:
DuneProject
-----------

Initialize a Dune module and add the dune project library

  dune_project()

This function needs to be run from every project CMakeLists.txt file.
It sets up the module, defines basic variables and manages depedencies.
Don't forget to call :ref:`finalize_dune_project` afterwards.

#]=======================================================================]

include_guard(GLOBAL)

include(DuneCxaDemangle)
include(DuneDoc)
include(DuneModuleDependencies)
include(DunePkgConfig)
include(GNUInstallDirs)


macro(dune_project)
  define_property(GLOBAL PROPERTY DUNE_MODULE_LIBRARIES
        BRIEF_DOCS "List of libraries of the module. DO NOT EDIT!"
        FULL_DOCS "List of libraries of the module. Used to generate CMake's package configuration files. DO NOT EDIT!")

  include(DuneModuleInformation)
  dune_module_information(${CMAKE_SOURCE_DIR})
  if(NOT PROJECT_VERSION OR NOT PROJECT_VERSION_MAJOR)
    # extract information from dune.module
    set(PROJECT_VERSION         "${DUNE_MOD_VERSION}" CACHE STRING "")
    set(PROJECT_VERSION_MAJOR   "${DUNE_VERSION_MAJOR}" CACHE STRING "")
    set(PROJECT_VERSION_MINOR   "${DUNE_VERSION_MINOR}" CACHE STRING "")
    set(PROJECT_VERSION_PATCH   "${DUNE_VERSION_REVISION}" CACHE STRING "")
  endif()

  add_library(${PROJECT_NAME})
  add_library(Dune::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
  set_property(GLOBAL APPEND PROPERTY DUNE_MODULE_LIBRARIES Dune::${PROJECT_NAME})

  # set properties of target ${NAME}
  set_target_properties(${PROJECT_NAME} PROPERTIES
    EXPORT_NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR} )

  # set include directories for target
  target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${CMAKE_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )

  # set <PackageName>_FOUND as if find_package was called for this library
  set(${PROJECT_NAME}_FOUND)

  dune_create_dependency_tree()
  dune_process_dependency_macros()

  # Set variable where the cmake modules will be installed.
  # Thus the user can override it and for example install
  # directly into the CMake installation. We use a cache variable
  # that is overridden by a local variable of the same name if
  # the user does not explicitely set a value for it. Thus the value
  # will automatically change if the user changes CMAKE_INSTALL_DATAROOTDIR
  # or CMAKE_INSTALL_PREFIX
  if(NOT DUNE_INSTALL_MODULEDIR)
    set(DUNE_INSTALL_MODULEDIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/modules
      CACHE PATH
      "Installation directory for CMake modules. Default is \${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/modules when not set explicitely")
  endif()
  if(NOT DUNE_INSTALL_NONOBJECTLIBDIR)
    set(DUNE_INSTALL_NONOBJECTLIBDIR lib
      CACHE PATH
      "Installation directory for libraries that are not architecture dependent. Default is lib when not set explicitely")
  endif()
endmacro(dune_project)