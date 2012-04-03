add_subdirectory("doxygen")
add_subdirectory("buildsystem")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/example.opts
  DESTINATION share/doc/dune-common
  )
