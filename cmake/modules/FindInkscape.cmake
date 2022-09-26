# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# .. cmake_module::
#
#    Module that checks for inkscape
#
#    Sets the following variables
#
#    :code:`INKSCAPE_FOUND`
#       Whether inkscape was found
#
#    :code:`INKSCAPE`
#       Path to inkscape to generate .png's form .svg's
#

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

# text for feature summary
set_package_properties("Inkscape" PROPERTIES
  DESCRIPTION "converts SVG images"
  URL "www.inkscape.org"
  PURPOSE "To generate the documentation with LaTeX")
