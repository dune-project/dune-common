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
  set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNEWEB_TRUE=\# -D DUNE_MOD_NAME=${DUNE_MOD_NAME} -D DUNE_MOD_VERSION=${DUNE_MOD_VERSION} -D DOXYSTYLE=${DOXYSTYLE_FILE}  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D srcdir=${CMAKE_CURRENT_SOURCE_DIR} -D top_srcdir=${CMAKE_SOURCE_DIR} -P ${SCRIPT_DIR}/CreateDoxyFile.cmake)
  add_custom_command (OUTPUT Doxyfile.in Doxyfile
    COMMAND ${make_doxyfile_command} COMMENT "Creating Doxyfile.in")
  add_custom_target(Doxyfile DEPENDS Doxyfile.in Doxyfile)
ENDMACRO (prepare_doxyfile)

#
# add_doxgen_target
#
# This macro creates a target for building (doxygen_${DUNE_MOD_NAME}) and installing
# (doxygen_install_${DUNE_MOD_NAME}) the generated doxygen documentation.
# The documentation is built during the top-level make doc call. We have added a dependency
# that make sure it is built before running make install.
MACRO (add_doxygen_target)
  if(DOXYGEN_FOUND)
    if("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
      set(SCRIPT_DIR ${CMAKE_SOURCE_DIR}/cmake/scripts)
      set(DOXYSTYLE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxystyle)
    else("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
      set(SCRIPT_DIR ${DUNE_COMMON_SCRIPT_DIR})
    endif("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    prepare_doxyfile()
    # A custom command that exectutes doxygen
    add_custom_command(OUTPUT html COMMAND
      #${DOXYGEN_EXECUTABLE} Doxyfile
      ${CMAKE_COMMAND} -D DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE} -P ${SCRIPT_DIR}/RunDoxygen.cmake
      COMMENT "Running doxygen documentation. This may take a while"
      DEPENDS Doxyfile)
    # Create a target for building the doxygen documentation of a module,
    # that is run during make doc.
    add_custom_target(doxygen_${DUNE_MOD_NAME} DEPENDS html)
    add_dependencies(doc doxygen_${DUNE_MOD_NAME})

    # Use a cmake call to install the doxygen documentation and create a target for it
    file(GLOB doxygenfiles GLOB ${CMAKE_CURRENT_BINARY_DIR}/html/*.html ${CMAKE_CURRENT_BINARY_DIR}/html/*.png ${CMAKE_CURRENT_BINARY_DIR}/html/*.css ${CMAKE_CURRENT_BINARY_DIR}/html/*.gif)

    set(doxygenfiles "${CMAKE_CURRENT_BINARY_DIR}/doxygen.log;${CMAKE_CURRENT_BINARY_DIR}/doxyerr.log;${doxygenfiles}")
    set(install_doxygen_command ${CMAKE_COMMAND} -D FILES="${doxygenfiles}" -D DIR=${CMAKE_INSTALL_PREFIX}/share/doc/${DUNE_MOD_NAME}/doxygen  -P ${SCRIPT_DIR}/InstallFile.cmake)
    add_custom_target(doxygen_install_${DUNE_MOD_NAME}
      ${install_doxygen_command}
      COMMENT "Installing doxygen documentation"
      DEPENDS doxygen_${DUNE_MOD_NAME})

    # When installing call cmake install with the above install target
    install(CODE
      "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target doxygen_install_${DUNE_MOD_NAME} )
       foreach(_file ${doxygenfiles})
         get_filename_component(_basename \${_file} NAME)
         LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES \${CMAKE_INSTALL_PREFIX}/share/doc/${DUNE_MOD_NAME}/doxygen/\${_basename})
       endforeach(_file in \${doxygenfiles})")
  endif(DOXYGEN_FOUND)
ENDMACRO (add_doxygen_target)
