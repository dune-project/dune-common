# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# .. cmake_module::
#
#    Find MathJax2 for rendering LaTeX in webpages like Doxygen documentation
#
#    You may set the following variables to modify the
#    behaviour of this module:
#
#    :ref:`MathJax2_ROOT`
#       the path to look for sphinx with the highest priority
#
#    The following variables are set by this module:
#
#    :code:`MATHJAX2_FOUND`
#       whether MathJax2 was found
#
#    :code:`MATHJAX2_PATH`
#       the path where the MathJax.js script is located
#
# .. cmake_variable:: MathJax2_ROOT
#
#   You may set this variable to have :ref:`FindMathJax2` look
#   for the :code:`MathJax.js` script in the given path
#   before inspecting system paths.
#

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
