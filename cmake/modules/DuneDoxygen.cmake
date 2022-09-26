# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Module for building documentation using doxygen.
#
# .. cmake_function:: add_doxygen_target
#
#    .. cmake_param:: TARGET
#       :single:
#
#       The suffix to add to the target name, default to the module name.
#
#    .. cmake_param:: DEPENDS
#       :multi:
#
#       A list of further dependencies of the doxygen documentation.
#       Might include :code:`mainpage.txt`.
#
#    .. cmake_param:: OUTPUT
#       :single:
#
#       Name of the output target, necessary if you don't generate html.
#
#    This macro creates a target for building (:code:`doxygen_${ProjectName}`) and installing
#    (:code:`doxygen_install_${ProjectName}`) the generated doxygen documentation.
#    The documentation is built during the top-level :code:`make doc` call. We have added a dependency
#    that makes sure it is built before running :code:`make install`.
#
include_guard(GLOBAL)

find_package(Doxygen)
set_package_properties("Doxygen" PROPERTIES
  DESCRIPTION "Class documentation generator"
  URL "www.doxygen.org"
  PURPOSE "To generate the class documentation from C++ sources")

# Set DOT_TRUE for the Doxyfile generation.
if (NOT DOXYGEN_DOT_FOUND)
  set(DOT_TRUE '\#')
endif()

add_custom_target(doxygen_install)

#
# prepare_doxyfile()
# This functions adds the necessary routines for the generation of the
# Doxyfile[.in] files needed to doxygen.
macro(prepare_doxyfile)
  message(STATUS "using ${DOXYSTYLE_FILE} to create doxystyle file")
  message(STATUS "using C macro definitions from ${DOXYGENMACROS_FILE} for Doxygen")

  # check whether module has a Doxylocal file
  find_file(_DOXYLOCAL Doxylocal PATHS ${CMAKE_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)

  if(_DOXYLOCAL)
    set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNE_MOD_NAME=${ProjectName} -D DUNE_MOD_VERSION=${ProjectVersion} -D DOXYSTYLE=${DOXYSTYLE_FILE} -D DOXYGENMACROS=${DOXYGENMACROS_FILE}  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D srcdir=${CMAKE_CURRENT_SOURCE_DIR} -D top_srcdir=${CMAKE_SOURCE_DIR} -P ${scriptdir}/CreateDoxyFile.cmake)
    add_custom_command(OUTPUT Doxyfile.in Doxyfile
      COMMAND ${make_doxyfile_command}
      COMMENT "Creating Doxyfile.in"
      DEPENDS ${DOXYSTYLE_FILE} ${DOXYGENMACROS_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal)
  else()
    set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNE_MOD_NAME=${ProjectName} -D DUNE_MOD_VERSION=${DUNE_MOD_VERSION} -D DOXYSTYLE=${DOXYSTYLE_FILE} -D DOXYGENMACROS=${DOXYGENMACROS_FILE} -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D top_srcdir=${CMAKE_SOURCE_DIR} -P ${scriptdir}/CreateDoxyFile.cmake)
    add_custom_command(OUTPUT Doxyfile.in Doxyfile
      COMMAND ${make_doxyfile_command}
      COMMENT "Creating Doxyfile.in"
      DEPENDS ${DOXYSTYLE_FILE} ${DOXYGENMACROS_FILE})
  endif()
  add_custom_target(doxyfile DEPENDS Doxyfile.in Doxyfile)
endmacro(prepare_doxyfile)

macro(add_doxygen_target)
  set(options )
  set(oneValueArgs TARGET OUTPUT)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(DOXYGEN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  # default target name is the module name
  if(NOT DOXYGEN_TARGET)
    set(DOXYGEN_TARGET ${ProjectName})
  endif()

  # default output is html
  if(NOT DOXYGEN_OUTPUT)
    set(DOXYGEN_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html")
  endif()

  dune_module_path(MODULE dune-common RESULT scriptdir SCRIPT_DIR)
  if(PROJECT_NAME STREQUAL "dune-common")
    set(DOXYSTYLE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxystyle)
    set(DOXYGENMACROS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-macros)
  endif()
  message(STATUS "Using scripts from ${scriptdir} for creating doxygen stuff.")

  if(DOXYGEN_FOUND)
    prepare_doxyfile()
    # custom command that executes doxygen
    add_custom_command(OUTPUT ${DOXYGEN_OUTPUT}
      COMMAND ${CMAKE_COMMAND} -D DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE} -P ${scriptdir}/RunDoxygen.cmake
      COMMENT "Building doxygen documentation. This may take a while"
      DEPENDS Doxyfile.in ${DOXYGEN_DEPENDS})
    # Create a target for building the doxygen documentation of a module,
    # that is run during make doc
    add_custom_target(doxygen_${DOXYGEN_TARGET}
      DEPENDS ${DOXYGEN_OUTPUT})
    add_dependencies(doc doxygen_${DOXYGEN_TARGET})

    # Use a cmake call to install the doxygen documentation and create a
    # target for it
    include(GNUInstallDirs)
    # When installing call cmake install with the above install target
    install(CODE
      "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target doxygen_${ProjectName}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        file(GLOB doxygenfiles
          GLOB ${CMAKE_CURRENT_BINARY_DIR}/html/*.html
          ${CMAKE_CURRENT_BINARY_DIR}/html/*.js
          ${CMAKE_CURRENT_BINARY_DIR}/html/*.png
          ${CMAKE_CURRENT_BINARY_DIR}/html/*.css
          ${CMAKE_CURRENT_BINARY_DIR}/html/*.gif
          ${CMAKE_CURRENT_BINARY_DIR}/*.tag
          )
        set(doxygenfiles \"\${doxygenfiles}\")
        foreach(_file \${doxygenfiles})
           get_filename_component(_basename \${_file} NAME)
           LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen/\${_basename})
         endforeach()
         file(INSTALL \${doxygenfiles} DESTINATION ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen)
         message(STATUS \"Installed doxygen into ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen\")")
  endif()
endmacro(add_doxygen_target)
