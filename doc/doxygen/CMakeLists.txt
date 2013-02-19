# quickhack for creating the Doxyfile.in and Doxyfile
add_custom_command (OUTPUT Doxyfile.in Doxyfile
  COMMAND ${CMAKE_COMMAND} -D DOXYSTYLE=${CMAKE_CURRENT_SOURCE_DIR}/Doxystyle  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D  abs_top_src_dir=${CMAKE_SOURCE_DIR} -D src_dir=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/CreateDoxyFile.cmake COMMENT "Creating Doxyfile.in")
add_custom_target(Doxyfile DEPENDS Doxyfile.in)
add_dependencies(doc Doxyfile)
install(FILES Doxystyle DESTINATION share/dune-common/doc/doxygen)
