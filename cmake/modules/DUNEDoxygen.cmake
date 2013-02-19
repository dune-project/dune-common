FIND_PACKAGE(Doxygen)

#
# Set DOT_TRUE for the Doxyfile generation.
#
if (NOT DOXYGEN_DOT_FOUND)
  set(DOT_TRUE '\#')
endif (NOT DOXYGEN_DOT_FOUND)

add_custom_target(doxygen_install)

#
# prepare_doxyfile()
# This functions adds the necessary routines for the generation of the Doxyfile[.in] files needed to doxygen.
MACRO (prepare_doxyfile)
  set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNEWEB_TRUE=\# -D DUNE_MOD_NAME=${DUNE_MOD_NAME} -D DUNE_MOD_VERSION=${DUNE_MOD_VERSION} -D DOXYSTYLE=${CMAKE_CURRENT_SOURCE_DIR}/Doxystyle  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D srcdir=${CMAKE_CURRENT_SOURCE_DIR} -D top_srcdir=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/CreateDoxyFile.cmake)
  add_custom_command (OUTPUT Doxyfile.in Doxyfile
    COMMAND ${make_doxyfile_command} COMMENT "Creating Doxyfile.in")
  add_custom_target(Doxyfile DEPENDS Doxyfile.in Doxyfile)
ENDMACRO (prepare_doxyfile)

MACRO (add_doxygen_target)
  if(DOXYGEN_FOUND)
    prepare_doxyfile()
    add_custom_command(OUTPUT html COMMAND
      ${DOXYGEN_EXECUTABLE} Doxyfile
      COMMENT "Running doxygen documentation" DEPENDS Doxyfile)
    add_custom_target(doxygen_${DUNE_MOD_NAME} DEPENDS html)
    add_dependencies(doc doxygen_${DUNE_MOD_NAME})
    #message(${doxgen_files})
    set(install_doxygen_command ${CMAKE_COMMAND} -D CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -D DUNE_MOD_NAME=${DUNE_MOD_NAME} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/InstallDoxygen.cmake)
    add_custom_target(doxygen_install_${DUNE_MOD_NAME}
      ${install_doxygen_command}
      COMMENT "Installing doxygen documentation"
      DEPENDS doxygen_${DUNE_MOD_NAME})
    add_dependencies(doxygen_install doxygen_install_${DUNE_MOD_NAME})
    # When installing call make install manually
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target doxygen_install )")
  endif(DOXYGEN_FOUND)
ENDMACRO (add_doxygen_target)
