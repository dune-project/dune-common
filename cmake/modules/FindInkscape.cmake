# Module that checks for inkscape
#
# Sets the following variables
#
# INSCAPE: Path to inkscape to generate .png's form .svg's
#
# Provides the following functions:
#
# inkscape_generate_png_from_svg([OUTPUT_DIR <output_dir>] <pngfile1.png> [<pngfile2.png> ....])
#
# Generates pngfile1, ... from svg input files pngfile1.svg, ....
# The output directory can be specified with the option OUTPUT_DIR. If it is omitted
# the files will be generated in CMAKE_CURRENT_BINARY_DIR.

find_program(INKSCAPE inkscape DOC "Path to inkscape to generate png files from svg files")
find_program(CONVERT convert DOC "Path to convert program")
if(INKSCAPE)
  set(INKSCAPE_FOUND True)
endif(INKSCAPE)

