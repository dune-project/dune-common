# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindMathJax2
============

Find MathJax2 for rendering LaTeX formulas in generated web documentation,
especially Doxygen output.

This find-module searches for a local installation containing ``MathJax.js``
and provides the usual ``find_package(MathJax2)`` result variables.

.. cmake:variable:: MathJax2_ROOT

  Preferred search prefix for the MathJax2 installation. If set, this path is
  searched before the built-in system locations.

.. cmake:variable:: MATHJAX2_FOUND

  Boolean result indicating whether MathJax2 was found.

.. cmake:variable:: MATHJAX2_PATH

  Directory containing the discovered ``MathJax.js`` script.

#]=======================================================================]

find_path(MATHJAX2_PATH
  MathJax.js
  PATHS
    "/usr/share/javascript/mathjax"
    "/usr/local/share/javascript/mathjax/"
    "/usr/share/yelp/mathjax"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "MathJax2"
  DEFAULT_MSG
  MATHJAX2_PATH
)
