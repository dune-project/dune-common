# Module that provides convertion routines using inkscape
#
# .. cmake_function:: inkscape_generate_png_from_svg
#
#    .. cmake_param:: OUTPUT_DIR
#       :single:
#
#       The output directory for the generated png files.
#       Defaults to the current build directory.
#
#    .. cmake_param:: pngfiles
#       :single:
#       :positional:
#       :required:
#
#       The files that should be converted.
#
#    .. cmake_param:: DPI
#       :single:
#
#       dpi value for the generated image (default: 90)
#
#    TODO Switch to named arguments!
#

include(CMakeParseArguments)

function(inkscape_generate_png_from_svg)
  if(NOT INKSCAPE)
    return()
  endif(NOT INKSCAPE)
  cmake_parse_arguments(INKSCAPE "" "OUTPUT_DIR;DPI" "" ${ARGN})
  if(NOT INKSCAPE_OUTPUT_DIR)
    set(INKSCAPE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif(NOT INKSCAPE_OUTPUT_DIR)
  if(NOT INKSCAPE_DPI)
    set(INKSCAPE_DPI 90)
  endif(NOT INKSCAPE_DPI)

  foreach(pic ${INKSCAPE_UNPARSED_ARGUMENTS})
    string(REGEX REPLACE "\\.[a-zA-Z]+" ".svg" input ${pic})
    add_custom_command(OUTPUT ${pic}
      COMMAND ${INKSCAPE} --export-dpi=${INKSCAPE_DPI} -e ${pic} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${input}
      COMMENT "Generating ${INKSCAPE_OUTPUT_DIR}/${pic} from ${CMAKE_CURRENT_SOURCE_DIR}/${input}"
      WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR})
  endforeach(pic)
endfunction(inkscape_generate_png_from_svg)

function(inkscape_generate_eps_from_svg)
  cmake_parse_arguments(INKSCAPE "" "INPUT_DIR;OUTPUT_DIR;DPI" "" ${ARGN})
  if(NOT INKSCAPE_INPUT_DIR)
    set(INKSCAPE_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif(NOT INKSCAPE_INPUT_DIR)
  if(NOT INKSCAPE_INPUT_DIR)
    set(INKSCAPE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif(NOT INKSCAPE_INPUT_DIR)

  foreach(_pic ${INKSCAPE_UNPARSED_ARGUMENTS})
    string(REGEX REPLACE "\\.[a-zA-Z]+" ".png" input "${_pic}")
    string(REGEX REPLACE "\\.[a-zA-Z]+" ".svg" svginput "${_pic}")

    add_custom_target(${input}
      COMMAND ${INKSCAPE} --export-dpi=${INKSCAPE_DPI} -e ${input} ${CMAKE_CURRENT_SOURCE_DIR}/${svginput}
      COMMENT "Generating ${INKSCAPE_OUTPUT_DIR}/${svginput} from ${CMAKE_CURRENT_SOURCE_DIR}/${input}")
    add_custom_command(OUTPUT ${_pic}
      COMMAND ${CONVERT} ${INKSCAPE_OUTPUT_DIR}/${input} EPS:${_pic}
      DEPENDS ${input}
      COMMENT "Converting ${INKSCAPE_OUTPUT_DIR}/${input} to ${INKSCAPE_OUTPUT_DIR}/${_pic}"
      WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR})
  endforeach(_pic ${INKSCAPE_UNPARSED_ARGUMENTS})
endfunction(inkscape_generate_eps_from_svg)
