# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindInkscape
============

Find Inkscape and related conversion tools used for documentation asset
generation.

This find-module locates the ``inkscape`` executable and a compatible
ImageMagick ``convert`` executable.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

.. cmake:variable:: INKSCAPE_FOUND

  Boolean result indicating whether Inkscape was found.

.. cmake:variable:: INKSCAPE

  Full path to the ``inkscape`` executable.

.. cmake:variable:: CONVERT

  Full path to the ``convert`` executable, if found.

.. cmake:variable:: INKSCAPE_NEW_VERSION

  Boolean result indicating whether the detected Inkscape behaves like version
  1.0 or newer with respect to command-line options.

#]=======================================================================]

# text for feature summary
include(FeatureSummary)
set_package_properties("Inkscape" PROPERTIES
  DESCRIPTION "Free and open source vector graphics editor"
  URL "www.inkscape.org")

find_program(INKSCAPE inkscape DOC "Path to inkscape to generate png files from svg files")
find_program(CONVERT convert DOC "Path to convert program")
if(INKSCAPE)
  set(INKSCAPE_FOUND True)
  # check for inkscape >= 1.0
  execute_process(COMMAND ${INKSCAPE} -z -e OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE INKSCAPE_RETURNED_ONE)
  # if error (i.e. 1) was returned we have new inkscape version (>=1.0)
  if(INKSCAPE_RETURNED_ONE)
    set(INKSCAPE_NEW_VERSION True)
  endif()

endif(INKSCAPE)
