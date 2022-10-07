# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#
# Module that provides a custom target make doc at the top level
# directory and utility macros for creating install directives
# that make sure that the files to be installed are previously
# generated even if make doc was not called.
#
# All documentation (Latex, Doxygen) will be generated during
# make doc.
# It provides the following macros:
#
# .. cmake_function:: dune_add_latex_document
#
#    .. cmake_brief::
#
#       wrapper around add_latex_document for compatibility reasons
#
# .. cmake_function:: create_doc_install
#
#    .. cmake_brief::
#
#       creates a target for creating and installing a file
#       to a given directory.
#
#    .. cmake_param:: filename
#       :single:
#       :required:
#       :positional:
#
#       The name of the file to be installed.
#
#    .. cmake_param:: targetdir
#       :single:
#       :required:
#       :positional:
#
#       The directory into which the beforementioned file will be installed.
#
#    .. cmake_param:: dependency
#       :single:
#       :required:
#       :positional:
#
#       A target that gets called to create the file that will be installed.
#
#    .. note::
#
#       This macro is needed, as we cannot add dependencies to the install
#       target. See https://gitlab.kitware.com/cmake/cmake/issues/8438
#       and https://gitlab.dune-project.org/core/dune-common/issues/36
#
include_guard(GLOBAL)

include(UseLatexMk)

if (LATEXMK_FOUND AND PDFLATEX_COMPILER)
  set(LATEX_USABLE TRUE)
endif()

add_custom_target(doc)

# add the Sphinx-generated build system documentation
include(DuneSphinxCMakeDoc)
# Support building documentation with doxygen.
include(DuneDoxygen)

macro(create_doc_install filename targetdir dependency)
  if(LATEX_USABLE)
    dune_module_path(MODULE dune-common RESULT scriptdir SCRIPT_DIR)
    get_filename_component(targetfile ${filename} NAME)
    set(install_command ${CMAKE_COMMAND} -D FILES=${filename} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${scriptdir}/InstallFile.cmake)

    # create a custom target for the installation
    add_custom_target(install_${targetfile} ${install_command}
      COMMENT "Installing ${filename} to ${targetdir}"
      DEPENDS ${dependency})
    # When installing, call cmake install with the above install target and add the file to install_manifest.txt
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target install_${targetfile} )
              LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_PREFIX}/${targetdir}/${targetfile})")
  endif()
endmacro(create_doc_install)

macro(dune_add_latex_document)
  add_latex_document(${ARGN})
endmacro(dune_add_latex_document)
