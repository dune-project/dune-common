# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindSphinx
==========

Find Sphinx, the Python documentation generator.

This find-module searches for the ``sphinx-build`` executable and provides the
usual ``find_package(Sphinx)`` result variables.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

.. cmake:variable:: SPHINX_FOUND

  Boolean result indicating whether Sphinx was found.

.. cmake:variable:: SPHINX_EXECUTABLE

  Full path to the discovered ``sphinx-build`` executable.

Hints
^^^^^

The following variable may be set to influence this module's behavior:

.. cmake:variable:: SPHINX_ROOT

  Preferred search prefix for the ``sphinx-build`` executable. If set, this
  path is searched with higher priority before falling back to the system
  search path.

#]=======================================================================]

#TODO export version.

find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build
             PATHS ${SPHINX_ROOT}
             NO_DEFAULT_PATH)

find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "Sphinx"
  DEFAULT_MSG
  SPHINX_EXECUTABLE
)
