#
# SPDX-FileCopyrightInfo: Copyright (c) 2017, Dominic Kempf, Steffen Muething
# SPDX-License-Identifier: BSD-3-Clause
#
# Vendored from UseLatexMk. See UseLatexMk.cmake.license for the upstream
# license metadata retained alongside this file.
#

#[=======================================================================[.rst:
UseLatexMk
----------

Helpers for building LaTeX documents with ``latexmk`` from CMake.

.. cmake:command:: add_latex_document

  Add a custom target that builds a PDF document from a LaTeX source file.

  .. code-block:: cmake

    add_latex_document(
      SOURCE <tex-source>
      [TARGET <target-name>]
      [EXCLUDE_FROM_ALL]
      [REQUIRED]
      [FATHER_TARGET <meta-target>...]
      [RCFILE <latexmkrc>...]
      [INSTALL <destination>]
      [BUILD_ON_INSTALL]
    )

  ``SOURCE``
    Required path to the main LaTeX source file.

  ``TARGET``
    Optional target name. If omitted, a name is derived from the source path.

  ``EXCLUDE_FROM_ALL``
    Exclude the generated target from the default ``all`` target. This is
    implied when ``FATHER_TARGET`` is specified.

  ``REQUIRED``
    Fail with a fatal error if the document cannot be built because LaTeX or
    ``latexmk`` is unavailable.

  ``FATHER_TARGET``
    Existing meta-targets that should depend on the generated document target.

  ``RCFILE``
    Additional ``latexmkrc`` files that are configured with
    :cmake:command:`configure_file()` and loaded after the automatically
    generated rc file.

  ``INSTALL``
    Install destination for the generated PDF document.

  ``BUILD_ON_INSTALL``
    Trigger a build of the document during installation.

  The module also creates a global ``clean_latex`` target that removes LaTeX
  output and auxiliary files.

.. cmake:variable:: LATEXMK_PARANOID

  Re-enable TeX's stricter output-directory security behavior. By default,
  this module relaxes that behavior because ``latexmk`` builds may need to
  write outputs outside the current working directory.

  Enabling this option may break some document builds depending on the TeX
  toolchain and auxiliary tools in use.

#]=======================================================================]

include_guard(GLOBAL)

# ensure CMake version is recent enough
if(CMAKE_VERSION VERSION_LESS 3.10)
  message(FATAL_ERROR "UseLatexMk.cmake requires CMake 3.10 or newer")
endif()

# Find LATEX and LatexMk
find_package(LATEX)
find_package(LatexMk)

# Find the latexmkrc template file shipped alongside UseLatexMk.cmake
find_file(LATEXMKRC_TEMPLATE
          latexmkrc.cmake
          HINTS ${CMAKE_MODULE_PATH}
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${PROJECT_SOURCE_DIR}
                ${PROJECT_SOURCE_DIR}/cmake
                ${PROJECT_SOURCE_DIR}/cmake/modules
          NO_CMAKE_FIND_ROOT_PATH
          )

# Add the clean_latex target
if(TARGET clean_latex)
  message(WARNING "clean_latex target already exists. UseLatexMk attaches clean rules to it!")
else()
  add_custom_target(clean_latex)
endif()

set(LATEXMK_SOURCES_BUILD_FROM)

function(add_latex_document)
  # Parse the input parameters to the function
  set(OPTION REQUIRED EXCLUDE_FROM_ALL BUILD_ON_INSTALL)
  set(SINGLE SOURCE TARGET INSTALL)
  set(MULTI FATHER_TARGET RCFILE)
  cmake_parse_arguments(LMK "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})

  if(LMK_UNPARSED_ARGUMENTS)
    message("add_latex_document: Unparsed arguments! This often indicates typos in named arguments.")
  endif()

  # Apply default arguments and check for required arguments
  if(NOT LMK_SOURCE)
    message(FATAL_ERROR "No tex source specified for add_latex_document!")
  endif()
  if(NOT LMK_TARGET)
    # Construct a nice target name from the source file
    get_filename_component(LMK_TARGET ${LMK_SOURCE} ABSOLUTE)
    file(RELATIVE_PATH LMK_TARGET ${PROJECT_SOURCE_DIR} ${LMK_TARGET})
    string(REPLACE "/" "_" LMK_TARGET ${LMK_TARGET})
    string(REPLACE "." "_" LMK_TARGET ${LMK_TARGET})
  endif()
  if(LMK_FATHER_TARGET)
    set(LMK_EXCLUDE_FROM_ALL TRUE)
  endif()
  if(LMK_BUILD_ON_INSTALL AND (NOT LMK_INSTALL))
    message(WARNING "Specified to build on installation, but not installing!")
  endif()

  # Verify that each source is used exactly once
  set(ABS_SOURCE ${LMK_SOURCE})
  if(NOT IS_ABSOLUTE ${ABS_SOURCE})
    get_filename_component(ABS_SOURCE ${ABS_SOURCE} ABSOLUTE)
  endif()
  list(FIND LATEXMK_SOURCES_BUILD_FROM ${ABS_SOURCE} ALREADY_BUILT)
  if(NOT "${ALREADY_BUILT}" STREQUAL "-1")
    message(FATAL_ERROR "UseLatexMk: You are building twice from the same source, which is unsupported!")
  endif()
  set(LATEXMK_SOURCES_BUILD_FROM ${LATEXMK_SOURCES_BUILD_FROM} ${ABS_SOURCE} PARENT_SCOPE)

  # Check the existence of the latexmk executable and skip/fail if not present
  if(NOT (LATEXMK_FOUND AND PDFLATEX_COMPILER))
    if(LMK_REQUIRED)
      message(FATAL_ERROR "Some Latex documents were required by the project, but LATEX or LatexMk were not found!")
    else()
      return()
    endif()
  endif()

  # Inspect the EXCLUDE_FROM_ALL option
  if(LMK_EXCLUDE_FROM_ALL)
    set(ALL_OPTION "")
  else()
    set(ALL_OPTION "ALL")
  endif()

  # Generate a latexmkrc file for this project
  if(NOT LATEXMKRC_TEMPLATE)
    message("Fatal error: The latexmkrc template file could not be found. Consider adding its path to CMAKE_MODULE_PATH")
  endif()
  set(LATEXMKRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${LMK_TARGET}.latexmkrc")
  configure_file(${LATEXMKRC_TEMPLATE} ${LATEXMKRC_FILE} @ONLY)
  set(LATEXMKRC_OPTIONS -r ${LATEXMKRC_FILE})

  # Process additional latexmkrc files
  foreach(rcfile ${LMK_RCFILE})
    get_filename_component(rcfile_base ${rcfile} NAME)
    set(LATEXMKRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${LMK_TARGET}_${rcfile_base}")
    configure_file(${rcfile} ${LATEXMKRC_FILE} @ONLY)
    set(LATEXMKRC_OPTIONS ${LATEXMKRC_OPTIONS} -r ${LATEXMKRC_FILE})
  endforeach()

  # Add the BYPRODUCTS parameter
  set(BYPRODUCTS_PARAMETER BYPRODUCTS ${OUTPUT_PDF})

  # Maybe allow latexmk the use of absolute paths
  set(ENV_COMMAND "")
  if(NOT LATEXMK_PARANOID)
    set(ENV_COMMAND ${CMAKE_COMMAND} -E env openout_any="a")
  endif()

  # For LaTeX package minted, replace value of outputdir from CMAKE_CURRENT_BINARY_DIR to
  # its actual value by coping the LaTeX file to the build dir
  set(LMK_SOURCE_REPLACED ${CMAKE_CURRENT_BINARY_DIR}/${LMK_SOURCE})
  # for in-source-build, adjust
  if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    get_filename_component(source_filename ${LMK_SOURCE} NAME_WE)
    get_filename_component(source_extension ${LMK_SOURCE} LAST_EXT)
    set(LMK_SOURCE_REPLACED "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}_source${source_extension}")
  endif()
  configure_file(${LMK_SOURCE} ${LMK_SOURCE_REPLACED} @ONLY)

  # Determine the output name
  get_filename_component(output ${LMK_SOURCE_REPLACED} NAME_WE)
  set(OUTPUT_PDF ${CMAKE_CURRENT_BINARY_DIR}/${output}.pdf)

  # Call the latexmk executable
  # NB: Using add_custom_target here results in the target always being outofdate.
  #     This offloads the dependency tracking from cmake to latexmk. This is an
  #     intentional decision of UseLatexMk to avoid listing dependencies of the tex source.
  add_custom_target(${LMK_TARGET}
                    ${ALL_OPTION}
                    COMMAND ${ENV_COMMAND} ${LATEXMK_EXECUTABLE} ${LATEXMKRC_OPTIONS} ${LMK_SOURCE_REPLACED}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Building PDF from ${LMK_SOURCE}..."
                    ${BYPRODUCTS_PARAMETER}
                    )

  # Add dependencies to father targets
  foreach(father ${LMK_FATHER_TARGET})
    if(NOT TARGET ${father})
      message(FATAL_ERROR "The target given to add_latex_documents FATHER_TARGET parameter does not exist")
    endif()
    add_dependencies(${father} ${LMK_TARGET})
  endforeach()

  # Add installation rules
  if(LMK_BUILD_ON_INSTALL)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build . --target ${LMK_TARGET} --config $<CONFIGURATION>)")
  endif()
  if(LMK_INSTALL)
    install(FILES ${OUTPUT_PDF}
            DESTINATION ${LMK_INSTALL}
            OPTIONAL)
  endif()

  # Add a clean up rule to the clean_latex target
  add_custom_target(${LMK_TARGET}_clean
                    COMMAND ${ENV_COMMAND} ${LATEXMK_EXECUTABLE} -C ${LATEXMKRC_OPTIONS}  ${LMK_SOURCE_REPLACED}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Cleaning build results from target ${LMK_TARGET}"
                    )
  add_dependencies(clean_latex ${LMK_TARGET}_clean)
endfunction()
