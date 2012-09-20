add_subdirectory("doxygen")
add_subdirectory("buildsystem")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/example.opts
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  )
