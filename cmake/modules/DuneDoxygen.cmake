# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneDoxygen
===========

Support for building module documentation with Doxygen.

.. cmake:command:: add_doxygen_target

  Create a Doxygen build target for the current module.

  .. code-block:: cmake

    add_doxygen_target(
      [TARGET <target-suffix>]
      [DEPENDS <files>...]
      [OUTPUT <path>]
    )

  ``TARGET``
    Suffix of the generated build target. The default is the current module
    name.

  ``DEPENDS``
    Additional dependencies of the generated Doxygen build step, for example a
    manually maintained ``mainpage.txt`` file.

  ``OUTPUT``
    Output path produced by the Doxygen run. The default is the generated
    ``html`` directory in the current binary directory.

  This command creates a module-specific ``doxygen_<target>`` target and adds
  it as a dependency of the top-level ``doxygen`` and ``doc`` targets. During
  installation, the generated Doxygen output is copied into
  ``${CMAKE_INSTALL_DOCDIR}/doxygen``.

.. cmake:variable:: DUNE_MATHJAX_DISABLE_LOCAL

  If set to ``TRUE``, local MathJax discovery is disabled and Doxygen will not
  use an installed MathJax2 copy from the system.

.. cmake:variable:: DUNE_MATHJAX_DISABLE_CDN

  If set to ``TRUE``, MathJax will not be loaded from the content delivery
  network when no local MathJax2 installation is available.

#]=======================================================================]
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

add_custom_target(doxygen)
add_dependencies(doc doxygen)
add_custom_target(doxygen_install)

##############################
# Begin MathJax support

# Variables to configure MathJax support
option(DUNE_MATHJAX_DISABLE_LOCAL "Flag to disable usage of local MathJax")
option(DUNE_MATHJAX_DISABLE_CDN "Flag to disable usage of MathJax from content delivery network")

# Use local MathJax2 unless disabled
set(mathjax_relpath "")
set(use_mathjax OFF)
if(NOT DUNE_MATHJAX_DISABLE_LOCAL)
  # This currently searches for MathJax2 only which is the default in Doxygen.
  # Newer versions do not provide MathJax.js and have to be enabled manually
  # in Doxygen.
  find_package(MathJax2)
  if(MATHJAX2_FOUND)
    message(STATUS "Using local MathJax found in ${MATHJAX2_PATH}")
    set(use_mathjax ON)
    set(mathjax_relpath "${MATHJAX2_PATH}")
  endif()
endif()

# Use MathJax2 from cdn unless disabled
if((NOT DUNE_MATHJAX_DISABLE_CDN) AND (mathjax_relpath STREQUAL ""))
  message(STATUS "Using MathJax from content delivery network")
  set(use_mathjax ON)
endif()

# Don't use MathJax
if(NOT use_mathjax)
  message(STATUS "MathJax is disabled")
endif()

# Variables forwarded to Doxygen
set_property(GLOBAL PROPERTY DUNE_USE_MATHJAX "${use_mathjax}")
set_property(GLOBAL PROPERTY DUNE_MATHJAX_RELPATH "${mathjax_relpath}")

# End MathJax support
##############################

#
# prepare_doxyfile()
# This functions adds the necessary routines for the generation of the
# Doxyfile[.in] files needed to doxygen.
macro(prepare_doxyfile)
  cmake_parse_arguments(DOXYFILE "" "TARGET" "" ${ARGN})

  # default target name is the module name
  if(NOT DOXYFILE_TARGET)
    set(DOXYFILE_TARGET ${PROJECT_NAME})
  endif()

  # Get global properties for MathJax configuration
  get_property(use_mathjax GLOBAL PROPERTY DUNE_USE_MATHJAX)
  get_property(mathjax_relpath GLOBAL PROPERTY DUNE_MATHJAX_RELPATH)

  message(STATUS "using ${DOXYSTYLE_FILE} to create doxystyle file")
  message(STATUS "using C macro definitions from ${DOXYGENMACROS_FILE} for Doxygen")

  # check whether module has a Doxylocal file
  find_file(_DOXYLOCAL Doxylocal PATHS ${CMAKE_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)
  # convert use_mathjax to YES/NO for Doxygen
  set(use_mathjax $<IF:$<BOOL:${use_mathjax}>,YES,NO>)
  if(_DOXYLOCAL)
    set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNE_MOD_NAME=${ProjectName} -D DUNE_MOD_VERSION=${ProjectVersion} -D DOXYSTYLE=${DOXYSTYLE_FILE} -D DOXYGENMACROS=${DOXYGENMACROS_FILE}  -D DOXYLOCAL=${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D srcdir=${CMAKE_CURRENT_SOURCE_DIR} -D top_srcdir=${${PROJECT_NAME}_SOURCE_DIR} -D DUNE_USE_MATHJAX="${use_mathjax}" -D DUNE_MATHJAX_RELPATH="${mathjax_relpath}" -P ${scriptdir}/CreateDoxyFile.cmake)
    add_custom_command(OUTPUT Doxyfile.in Doxyfile
      COMMAND ${make_doxyfile_command}
      COMMENT "Creating Doxyfile.in"
      DEPENDS ${DOXYSTYLE_FILE} ${DOXYGENMACROS_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxylocal)
  else()
    set(make_doxyfile_command ${CMAKE_COMMAND} -D DOT_TRUE=${DOT_TRUE} -D DUNE_MOD_NAME=${ProjectName} -D DUNE_MOD_VERSION=${DUNE_MOD_VERSION} -D DOXYSTYLE=${DOXYSTYLE_FILE} -D DOXYGENMACROS=${DOXYGENMACROS_FILE} -D abs_top_srcdir=${CMAKE_SOURCE_DIR} -D top_srcdir=${${PROJECT_NAME}_SOURCE_DIR} -D DUNE_USE_MATHJAX="${use_mathjax}" -D DUNE_MATHJAX_RELPATH="${mathjax_relpath}" -P ${scriptdir}/CreateDoxyFile.cmake)
    add_custom_command(OUTPUT Doxyfile.in Doxyfile
      COMMAND ${make_doxyfile_command}
      COMMENT "Creating Doxyfile.in"
      DEPENDS ${DOXYSTYLE_FILE} ${DOXYGENMACROS_FILE})
  endif()
  add_custom_target(doxyfile_${DOXYFILE_TARGET} DEPENDS Doxyfile.in Doxyfile)
endmacro(prepare_doxyfile)

macro(add_doxygen_target)
  set(options )
  set(oneValueArgs TARGET OUTPUT)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(DOXYGEN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  # default target name is the module name
  if(NOT DOXYGEN_TARGET)
    set(DOXYGEN_TARGET ${PROJECT_NAME})
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

  if(TARGET Doxygen::doxygen)
    prepare_doxyfile(TARGET ${DOXYGEN_TARGET})
    # custom command that executes doxygen
    add_custom_command(OUTPUT ${DOXYGEN_OUTPUT}
      COMMAND ${CMAKE_COMMAND} -D DOXYGEN_EXECUTABLE=$<TARGET_FILE:Doxygen::doxygen> -P ${scriptdir}/RunDoxygen.cmake
      COMMENT "Building doxygen documentation. This may take a while"
      DEPENDS Doxyfile.in ${DOXYGEN_DEPENDS})
    # Create a target for building the doxygen documentation of a module,
    # that is run during make doc
    add_custom_target(doxygen_${DOXYGEN_TARGET}
      DEPENDS ${DOXYGEN_OUTPUT})
    add_dependencies(doxygen doxygen_${DOXYGEN_TARGET})

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
           # Manifest is generated when prefix was set at configuration time, otherwise is skipped
           LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_FULL_DOCDIR}/doxygen/\${_basename})
         endforeach()
         file(INSTALL \${doxygenfiles} DESTINATION ${CMAKE_INSTALL_DOCDIR}/doxygen)
         message(STATUS \"Installed doxygen into ${CMAKE_INSTALL_DOCDIR}/doxygen\")")
  endif()
endmacro(add_doxygen_target)
