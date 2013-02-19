# Module thaat checks for inkscape
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

find_program(INKSCAPE inkscape DOC "Path to inkscape to generate .png's form .svg'")

function(inkscape_generate_png_from_svg)
  if(NOT INKSCAPE)
    return()
  endif(NOT INKSCAPE)
  include(CMakeParseArguments)
  cmake_parse_arguments(INKSCAPE "" "OUTPUT_DIR DPI" "" ${ARGN})
  if(NOT INKSCAPE_OUTPUT_DIR)
    set(INKSCAPE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif(NOT INKSCAPE_OUTPUT_DIR)
  if(NOT INKSCAPE_DPI)
    set(INKSCAPE_DPI 90)
  endif(NOT INKSCAPE_DPI)
  foreach(pic ${INKSCAPE_UNPARSED_ARGUMENTS})
    string(REGEX REPLACE "\\.[^.]+" ".svg" input ${pic})
    #message("pic=${pic} input=${input}")
    add_custom_command(OUTPUT ${pic} COMMAND ${INKSCAPE} --export-dpi=${DPI} -e ${pic} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${input} COMMENT "Generating ${pic}"
      WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR})
    #message("    add_custom_command(OUTPUT ${pic} COMMAND ${INKSCAPE} -e ${pic} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
    #  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${input} COMMENT \"Generating ${pic}\"
    #  WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR})")
  endforeach(pic)
endfunction(inkscape_generate_png_from_svg)

function(inkscape_generate_eps_from_svg)
  string(REGEX REPLACE "\\.[^.]+" ".png" PNGS ${ARGN})
  inkscape_generate_png_from_svg(${ARGN})
  cmake_parse_arguments(INKSCAPE "" "OUTPUT_DIR DPI" "" ${PNGS})
  foreach(_pic ${INKSCAPE_UNPARSED_ARGUMENTS})
    string(REGEX REPLACE "\\.[^.]+" ".png" input ${pic})
    add_custom_command(OUTPUT ${pic}
      COMMAND ${CONVERT} ${input} EPS:${pic}
      DEPENDS ${input}
      COMMENT "Generating {INKSCAPE_OUTPUT_DIR}/${pic}"
      WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR})
  endforeach(_pic ${INKSCAPE_UNPARSED_ARGUMENTS})
endfunction(inkscape_generate_eps_from_svg)
