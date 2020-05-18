function(dune_install_library NAME)
  include(DuneModuleName)
  dune_module_name(${NAME} MODULE_NAME CONFIG_NAME)

  # installation of targets and export targts and so on
  install(TARGETS ${NAME} EXPORT ${NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dune
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/dune
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/dune
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/${MODULE_NAME} )

  export(EXPORT ${NAME}-targets
    FILE
      "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-targets.cmake"
    NAMESPACE Dune:: )

  install(EXPORT ${NAME}-targets FILE ${NAME}-targets.cmake
    NAMESPACE Dune::
    DESTINATION
      ${CMAKE_INSTALL_DATADIR}/cmake )

  # find configuration file template
  set(CONFIG_FILES "${CONFIG_NAME}Config.cmake.in" "${NAME}-config.cmake.in" "${NAME}Config.cmake.in")
  list(TRANSFORM CONFIG_FILES PREPEND "${PROJECT_SOURCE_DIR}/cmake/modules/")
  set(CONFIG_FILE "")
  foreach (FILENAME ${CONFIG_FILES})
    if (EXISTS ${FILENAME})
      set(CONFIG_FILE ${FILENAME})
    endif ()
  endforeach ()

  if (NOT CONFIG_FILE)
    message(FATAL_ERROR "No <PakageName>Config.cmake.in file found in list")
  endif ()

  # generate final configuration file
  include(CMakePackageConfigHelpers)
  configure_package_config_file(${CONFIG_FILE}
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-config.cmake
    INSTALL_DESTINATION
      ${CMAKE_INSTALL_DATADIR}/cmake
    )

  # generate version file
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-version.cmake
    VERSION
      ${${NAME}_VERSION}
    COMPATIBILITY
      SameMinorVersion
    )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-version.cmake
    DESTINATION
      ${CMAKE_INSTALL_DATADIR}/cmake
    )

  install(
    FILES
      dune.module
    DESTINATION
      ${CMAKE_INSTALL_LIBDIR}/dunecontrol
  )
  #file(WRITE ${PROJECT_BINARY_DIR}/config.h "")
  #configure_file(dune.module ${PROJECT_BINARY_DIR}/dune.module COPYONLY)

endfunction(dune_install_library)