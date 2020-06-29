#[=======================================================================[.rst:
FinalizeDuneProject
-------------------

Finalize a Dune module.

  finalize_dune_project()

This function needs to be run at the end of every project CMakeLists.txt file.
Among other things it creates the cmake package configuration files.

#]=======================================================================]

include_guard(GLOBAL)

include(CMakePackageConfigHelpers)
include(DuneConfigFile)
include(DunePkgConfig)
include(DuneSymlinkOrCopy)
include(GNUInstallDirs)

# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates config.h and the cmake-config files,
# some install directives and exports the module.
macro(finalize_dune_project)
  if(DUNE_SYMLINK_TO_SOURCE_TREE)
    dune_symlink_to_source_tree()
  endif()

  # compute under which libdir the package configuration files are to be installed.
  # If the module installs an object library we use CMAKE_INSTALL_LIBDIR
  # to capture the multiarch triplet of Debian/Ubuntu.
  # Otherwise we fall back to DUNE_INSTALL_NONOBJECTLIB which is lib
  # if not set otherwise.
  get_property(DUNE_MODULE_LIBRARIES GLOBAL PROPERTY DUNE_MODULE_LIBRARIES)
  if(DUNE_MODULE_LIBRARIES)
    set(DUNE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
  else()
    set(DUNE_INSTALL_LIBDIR ${DUNE_INSTALL_NONOBJECTLIBDIR})
  endif()

  # add a dummy source file in case no other sources were added
  get_target_property(PROJECT_SOURCES ${PROJECT_NAME} SOURCES)
  if(NOT PROJECT_SOURCES)
    file(TOUCH ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.cc)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.cc)
  endif()

  # installation of targets and export targts and so on
  install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${DUNE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DUNE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${DUNE_INSTALL_LIBDIR} )

  # generate cmake-targets file
  export(EXPORT ${PROJECT_NAME}-targets
    FILE
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake
    NAMESPACE Dune:: )

  # install cmake-targets file
  install(EXPORT ${PROJECT_NAME}-targets FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE Dune::
    DESTINATION
      ${DUNE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

  if(EXISTS ${PROJECT_SOURCE_DIR}/cmake/pkg/${PROJECT_NAME}-config.cmake.in)
    set(CONFIG_FILE_IN ${PROJECT_SOURCE_DIR}/cmake/pkg/${PROJECT_NAME}-config.cmake.in)
  else()
    set(CONFIG_FILE_IN ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in)
    configure_file(${dune-common_PKG_PATH}/template-config.cmake.in
      ${CONFIG_FILE_IN} COPYONLY)

    # additional lines that are set by the module CMake files
    if(DUNE_CUSTOM_PKG_CONFIG_SECTION)
      file(APPEND ${CONFIG_FILE_IN} "\n${DUNE_CUSTOM_PKG_CONFIG_SECTION}\n")
    endif()
  endif()

  # set additional directories fir the module-config file
  set(DOXYSTYLE_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune-common/doc/doxygen)
  set(SCRIPT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)

  # Set the location where the doc sources are installed.
  set(DUNE_MODULE_SRC_DOCDIR ${PROJECT_SOURCE_DIR}/doc)

  # generate final configuration file
  configure_package_config_file(${CONFIG_FILE_IN}
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION
      ${DUNE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    PATH_VARS
      DOXYSTYLE_DIR SCRIPT_DIR
    )

  # generate version file
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-version.cmake
    VERSION
      ${PROJECT_VERSION}
    COMPATIBILITY
      SameMinorVersion
    )

  # install cmake-config and cmake-version files
  install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-version.cmake
    DESTINATION
      ${DUNE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

  # install dune.module file
  install(FILES dune.module
    DESTINATION ${DUNE_INSTALL_NONOBJECTLIBDIR}/dunecontrol/${PROJECT_NAME})

  # install config.h
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    install(FILES ${PROJECT_SOURCE_DIR}/config.h.cmake
      DESTINATION share/${PROJECT_NAME})
  endif()

  # install pkg-config files
  create_and_install_pkconfig(${DUNE_INSTALL_LIBDIR})

  if("${ARGC}" EQUAL "1")
    message(STATUS "Adding custom target for config.h generation")
    dune_regenerate_config_cmake()
    # add a target to generate config.h.cmake
    add_custom_target(OUTPUT config_collected.h.cmake
      COMMAND dune_regenerate_config_cmake())
    # actually write the config.h file to disk
    # using generated file
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/config_collected.h.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  else()
    message(STATUS "Not adding custom target for config.h generation")
    # actually write the config.h file to disk
    configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  endif()
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)

  if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    feature_summary(WHAT ALL)
  endif()
endmacro(finalize_dune_project)