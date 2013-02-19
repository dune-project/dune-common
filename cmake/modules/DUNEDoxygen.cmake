FIND_PACKAGE(Doxygen)

if (NOT DOXYGEN_DOT_FOUND)
  set(DOT_TRUE '\#')
endif (NOT DOXYGEN_DOT_FOUND)

message('${DOT_TRUE}')
#
# prepare_doyfile()
# This functions adds the necessary routines for the generation of the Doxyfile[.in] files needed to doxygen.
FUNCTION (prepare_doxyfile)
    set(make_doxyfile_command ${CMAKE_COMMAND} -D DUNE_MOD_NAME=${DUNE_MOD_NAME} -D DUNE_MOD_VERSION=${DUNE_MOD_VERSION} -D DOXYSTYLE=${CMAKE_CURRENT_SOURCE_DIR}/Doxystyle  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D srcdir=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/CreateDoxyFile.cmake)
  add_custom_command (OUTPUT Doxyfile.in Doxyfile
    COMMAND ${make_doxyfile_command} COMMENT "Creating Doxyfile.in")
  add_custom_target(Doxyfile DEPENDS Doxyfile.in Doxyfile)
ENDFUNCTION (prepare_doxyfile)
