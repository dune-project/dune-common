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

  message(STATUS "Generate PNG from SVG in ${INKSCAPE_OUTPUT_DIR}")
  set(_converted 0)
  foreach(pic ${INKSCAPE_UNPARSED_ARGUMENTS})
    string(REGEX REPLACE "\\.[a-zA-Z]+" ".svg" input ${pic})
    if(NOT EXISTS ${INKSCAPE_OUTPUT_DIR}/${pic})
      set(_converted 1)
      if(INKSCAPE_NEW_VERSION)
        execute_process(
          COMMAND ${INKSCAPE} --export-dpi=${INKSCAPE_DPI} --export-type=png --export-filename=${pic} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
          WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR}
          OUTPUT_QUIET)
      else()
        execute_process(
          COMMAND ${INKSCAPE} -z --export-dpi=${INKSCAPE_DPI} -e ${pic} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
          WORKING_DIRECTORY  ${INKSCAPE_OUTPUT_DIR}
          OUTPUT_QUIET)
      endif()
    endif()
  endforeach()
  if(_converted)
    message(STATUS "Generate PNG from SVG in ${INKSCAPE_OUTPUT_DIR} - done")
  else()
    message(STATUS "Generate PNG from SVG in ${INKSCAPE_OUTPUT_DIR} - skipped")
  endif()
endfunction()
