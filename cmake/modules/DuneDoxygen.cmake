# Module for building documentation using doxygen.
#
# provides the following macros:
#
# add_doxgen_target
#
# This macro creates a target for building (doxygen_${ProjectName}) and installing
# (doxygen_install_${ProjectName}) the generated doxygen documentation.
# The documentation is built during the top-level make doc call. We have added a dependency
# that makes sure it is built before running make install.
#
#
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
# This functions adds the necessary routines for the generation of the
# Doxyfile[.in] files needed to doxygen.
MACRO (prepare_doxyfile)
  message(STATUS "using ${DOXYSTYLE_FILE} to create doxystyle file")
  set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNEWEB_TRUE=\# -D ProjectName=${ProjectName} -D DUNE_MOD_VERSION=${DUNE_MOD_VERSION} -D DOXYSTYLE=${DOXYSTYLE_FILE}  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D srcdir=${CMAKE_CURRENT_SOURCE_DIR} -D top_srcdir=${CMAKE_SOURCE_DIR} -P ${SCRIPT_DIR}/CreateDoxyFile.cmake)
  add_custom_command (OUTPUT Doxyfile.in Doxyfile
    COMMAND ${make_doxyfile_command}
    COMMENT "Creating Doxyfile.in"
    DEPENDS ${DOXYSTYLE_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal)
  add_custom_target(Doxyfile DEPENDS Doxyfile.in Doxyfile)
ENDMACRO (prepare_doxyfile)

#
# add_doxgen_target
#
# This macro creates a target for building (doxygen_${ProjectName}) and installing
# (doxygen_install_${ProjectName}) the generated doxygen documentation.
# The documentation is built during the top-level make doc call. We have added a dependency
# that make sure it is built before running make install.
MACRO (add_doxygen_target)
  dune_common_script_dir(SCRIPT_DIR)
  if("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(DOXYSTYLE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxystyle)
  endif("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
  message(STATUS "Using scripts from ${SCRIPT_DIR} for creating doxygen stuff.")

  if(DOXYGEN_FOUND)
    prepare_doxyfile()
    # A custom command that executes doxygen
    add_custom_command(OUTPUT html
      COMMAND ${CMAKE_COMMAND} -D DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE} -P ${SCRIPT_DIR}/RunDoxygen.cmake
      COMMENT "Running doxygen documentation. This may take a while"
      DEPENDS Doxyfile.in)
    # Create a target for building the doxygen documentation of a module,
    # that is run during make doc
    add_custom_target(doxygen_${ProjectName}
      DEPENDS html)
    add_dependencies(doc doxygen_${ProjectName})
  endif(DOXYGEN_FOUND)

  # Use a cmake call to install the doxygen documentation and create a
  # target for it
  include(GNUInstallDirs)
  # When installing call cmake install with the above install target
  install(CODE
    "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target doxygen_${ProjectName}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
      file(GLOB doxygenfiles
        GLOB ${CMAKE_CURRENT_BINARY_DIR}/html/*.html
        ${CMAKE_CURRENT_BINARY_DIR}/html/*.png
        ${CMAKE_CURRENT_BINARY_DIR}/html/*.css
        ${CMAKE_CURRENT_BINARY_DIR}/html/*.gif)
      set(doxygenfiles \"\${doxygenfiles}\")
      foreach(_file \${doxygenfiles})
         get_filename_component(_basename \${_file} NAME)
         LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen/\${_basename})
       endforeach(_file in \${doxygenfiles})
       file(INSTALL \${doxygenfiles} DESTINATION ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen)
       message(STATUS \"Installed doxygen into ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen\")")

ENDMACRO (add_doxygen_target)
