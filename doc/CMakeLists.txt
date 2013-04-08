add_subdirectory("doxygen")
add_subdirectory("buildsystem")
add_subdirectory("comm")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/example.opts
  DESTINATION ${CMAKE_INSTALL_DOCDIR}
  )
