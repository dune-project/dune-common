# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
#
#
# .. cmake_function:: dune_create_inkscape_image_converter_target
#
#    Creates a target that when built, converts svg images to png images using inkscape
#
#    .. cmake_param:: TARGET
#       :single:
#
#       Name of the target to be created.
#
#    .. cmake_param:: OUTPUT_DIR
#       :single:
#       :required:
#
#       The output directory for the generated png files.
#       Defaults to the current build directory.
#
#    .. cmake_param:: IMAGES
#       :multi:
#       :required:
#
#       The files that should be converted.
#
#    .. cmake_param:: DPI
#       :single:
#
#       dpi value for the generated image (default: 90)
#
#    .. cmake_param:: ALL
#       :option:
#
#       If given, will add the created target to the all target
#
include_guard(GLOBAL)

function(inkscape_generate_png_from_svg)
  message(DEPRECATION "inkscape_generate_png_from_svg is deprecated (will be removed after release 2.9). "
                      "Use dune_create_inkscape_image_converter_target.")
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


function(dune_create_inkscape_image_converter_target)
  set(OPTION ALL)
  set(SINGLE TARGET OUTPUT_DIR DPI)
  set(MULTI IMAGES)
  cmake_parse_arguments(INKSCAPE_CONV "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})

  if(NOT INKSCAPE_CONV_TARGET)
    message(FATAL_ERROR "Specifying a target name by setting TARGET is required.")
  endif()

  if(TARGET ${INKSCAPE_CONV_TARGET})
    get_property(TARGET_SOURCE_DIR TARGET ${INKSCAPE_CONV_TARGET} PROPERTY SOURCE_DIR)
    message(FATAL_ERROR "dune_create_inkscape_image_converter_target cannot create target "
                        "\"${INKSCAPE_CONV_TARGET}\" because another target with the same "
                        "name already exists. The existing target is a target created in "
                        "source directory \"${TARGET_SOURCE_DIR}\".")
  endif()

  # we always create the target even if it stays empty
  # (might happen because inkscape is not found or the image list is empty)
  if(INKSCAPE_CONV_ALL)
    add_custom_target(${INKSCAPE_CONV_TARGET} ALL)
  else()
    add_custom_target(${INKSCAPE_CONV_TARGET})
  endif()

  if(NOT INKSCAPE)
    message(STATUS "Inkscape not found so no images will be converted")
    return()
  endif()

  if(NOT INKSCAPE_CONV_OUTPUT_DIR)
    set(INKSCAPE_CONV_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if(NOT INKSCAPE_CONV_DPI)
    set(INKSCAPE_CONV_DPI 90)
  endif()

  foreach(_output_png ${INKSCAPE_CONV_IMAGES})
    string(REGEX REPLACE "\\.[a-zA-Z]+" ".svg" _input_svg ${_output_png})
    if(INKSCAPE_NEW_VERSION)
      add_custom_command(TARGET ${INKSCAPE_CONV_TARGET} PRE_BUILD
        COMMAND ${INKSCAPE} --export-dpi=${INKSCAPE_CONV_DPI} --export-type=png --export-filename=${_output_png} ${CMAKE_CURRENT_SOURCE_DIR}/${_input_svg}
        WORKING_DIRECTORY ${INKSCAPE_CONV_OUTPUT_DIR}
        COMMENT "Generating ${_output_png} from ${_input_svg} with inkscape"
      )
    else()
      add_custom_command(TARGET ${INKSCAPE_CONV_TARGET} PRE_BUILD
        COMMAND ${INKSCAPE} -z --export-dpi=${INKSCAPE_CONV_DPI} -e ${_output_png} ${CMAKE_CURRENT_SOURCE_DIR}/${_input_svg}
        WORKING_DIRECTORY ${INKSCAPE_CONV_OUTPUT_DIR}
        COMMENT "Generating ${_output_png} from ${_input_svg} with inkscape"
      )
    endif()
  endforeach()
endfunction()
