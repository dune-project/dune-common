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
endif(INKSCAPE)

