# Module that provides conversion routines using inkscape
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

include_guard(GLOBAL)

include(CMakeParseArguments)

# text for feature summary
set_package_properties("Inkscape" PROPERTIES
  DESCRIPTION "converts SVG images"
  URL "www.inkscape.org"
  PURPOSE "To generate the documentation with LaTeX")

function(inkscape_generate_png_from_svg)
  if(NOT INKSCAPE)
    return()
  endif()
  cmake_parse_arguments(INKSCAPE "" "OUTPUT_DIR;DPI" "" ${ARGN})
  if(NOT INKSCAPE_OUTPUT_DIR)
    set(INKSCAPE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  if(NOT INKSCAPE_DPI)
    set(INKSCAPE_DPI 90)
  endif()

  foreach(pic ${INKSCAPE_UNPARSED_ARGUMENTS})
    string(REGEX REPLACE "\\.[a-zA-Z]+" ".svg" input ${pic})
    execute_process(
      COMMAND ${INKSCAPE} -z --export-dpi=${INKSCAPE_DPI} -e ${pic} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
      WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR})
  endforeach()
endfunction()
