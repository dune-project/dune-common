# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneDoc
-------

Documentation helper commands used by dune-common.

This module provides the top-level ``doc`` target together with helper commands
used to build and install generated documentation files.

.. cmake:command:: dune_add_latex_document

  Wrapper around ``add_latex_document()`` for compatibility reasons.

  .. code-block:: cmake

    dune_add_latex_document(<args...>)


.. cmake:command:: create_doc_install

  Create a target for building and installing a generated documentation file.

  .. code-block:: cmake

    create_doc_install(<filename> <targetdir> <dependency>)

  ``filename``
    The name of the file to be installed.

  ``targetdir``
    The directory into which the file is installed.

  ``dependency``
    The target that creates the file before installation.

  This helper is needed because dependencies cannot be attached directly to the
  install target. See the related CMake issue
  ``https://gitlab.kitware.com/cmake/cmake/issues/8438`` and the DUNE issue
  ``https://gitlab.dune-project.org/core/dune-common/issues/36``.

#]=======================================================================]
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
