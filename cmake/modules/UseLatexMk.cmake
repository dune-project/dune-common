# UseLatexMk.cmake is a CMake module to build Latex documents
# from CMake.
#
# add_latex_document(SOURCE texsource
#                    [TARGET target]
#                    [EXCLUDE_FROM_ALL]
#                    [REQUIRED]
#                    [FATHER_TARGET father1 [father2 ...]]
#                    [RCFILE rcfile1 [rcfile2 ...]]
#                    [INSTALL destination]
#                    [BUILD_ON_INSTALL]
#                    )
#
# The arguments:
# SOURCE
#   Required argument with a single tex source that defines the document to be built
# TARGET
#   An optional target name, defaults to a suitable mangling of the given source and its path.
#   An additional target with _clean appended will be added as well, which cleans the output
#   and all auxiliary files.
# EXCLUDE_FROM_ALL
#   Set this to avoid the target from being built by default. If the FATHER_TARGET
#   parameter is set, this option is automatically set.
# REQUIRED
#   Set this option to issue a fatal error if the document could not
#   be built. By default it is only skipped.
# FATHER_TARGET
#   A list of meta-targets that should trigger a rebuild of this target (like "make doc").
#   The targets are expected to exist already. Specifying any such targets will automatically add the
#   above EXCLUDE_FROM_ALL option.
# RCFILE
#   A list configuration file to customize the latexmk build process. These are read by latexmk
#   *after* the automatically generated rc file in the indicated order. Note that latexmk rcfiles
#   override any previous settings.
#   You may also use CMake variables within @'s (like @CMAKE_CURRENT_BINARY_DIR@) and have
#   them replaced with the matching CMake variables (see cmake's configure_file command).
#   Note, that this is a powerful, but advanced feature. For details on what can be achieved
#   see the latexmk manual. Note, that triggering non-PDF builds through latexmkrc files might
#   cause problems with other features of UseLatexMk.
# INSTALL
#   Set this option to an install directory to create an installation rule for this document.
# BUILD_ON_INSTALL
#   Set this option, if you want to trigger a build of this document during installation.
#
# Furthermore, UseLatexMk defines a CMake target clean_latex which cleans the build tree from
# all PDF output and all auxiliary files. Note, that (at least for the Unix Makefiles generator)
# it is not possible to connect this process with the builtin clean target.
#
# Please note the following security restriction:
#
# UseLatexMk relies on latexmk separating input and output directory correctly.
# This includes using an absolute path for the output directory. On some TeX
# systems this requires the disabling of a security measure by setting `openout_any = a`.
# From the latexmk documentation:
#
# Commonly, the directory specified for output files is a subdirectory of the current working direc-
# tory. However, if you specify some other directory, e.g., "/tmp/foo" or "../output", be aware that
# this could cause problems, e.g., with makeindex or bibtex. This is because modern versions of
# these programs, by default, will refuse to work when they find that they are asked to write to a file
# in a directory that appears not to be the current working directory or one of its subdirectories. This
# is part of security measures by the whole TeX system that try to prevent malicious or errant TeX
# documents from incorrectly messing with a user’s files. If for $out_dir or $aux_dir you really do
# need to specify an absolute pathname (e.g., "/tmp/foo") or a path (e.g., "../output") that includes a
# higher-level directory, and you need to use makeindex or bibtex, then you need to disable the secu-
# rity measures (and assume any risks). One way of doing this is to temporarily set an operating
# system environment variable openout_any to "a" (as in "all"), to override the default "paranoid"
# setting.
#
# UseLatexMk.cmake allows to re-enable the TeX security measure by setting LATEXMK_PARANOID to TRUE
# through cmake -D, but it is not guaranteed to work correctly in that case.
#
# For further information, visit https://github.com/dokempf/UseLatexMk
#
#
# Copyright (c) 2017, Dominic Kempf, Steffen Müthing
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# * Neither the name of the Universität Heidelberg nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
include_guard(GLOBAL)

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

  # Determine the output name
  get_filename_component(output ${LMK_SOURCE} NAME_WE)
  set(OUTPUT_PDF ${CMAKE_CURRENT_BINARY_DIR}/${output}.pdf)

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

  # Add the BYPRODUCTS parameter, if the CMake version supports it
  set(BYPRODUCTS_PARAMETER "")
  if (CMAKE_VERSION VERSION_GREATER "3.2")
    set(BYPRODUCTS_PARAMETER BYPRODUCTS ${OUTPUT_PDF})
  endif()

  # Maybe allow latexmk the use of absolute paths
  if(NOT LATEXMK_PARANOID)
    set($ENV{openout_any} "a")
  endif()

  # Call the latexmk executable
  # NB: Using add_custom_target here results in the target always being outofdate.
  #     This offloads the dependency tracking from cmake to latexmk. This is an
  #     intentional decision of UseLatexMk to avoid listing dependencies of the tex source.
  add_custom_target(${LMK_TARGET}
                    ${ALL_OPTION}
                    COMMAND ${LATEXMK_EXECUTABLE} ${LATEXMKRC_OPTIONS} ${LMK_SOURCE}
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
                    COMMAND ${LATEXMK_EXECUTABLE} -C ${LATEXMKRC_OPTIONS} ${LMK_SOURCE}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Cleaning build results from target ${LMK_TARGET}"
                    )
  add_dependencies(clean_latex ${LMK_TARGET}_clean)
endfunction()
