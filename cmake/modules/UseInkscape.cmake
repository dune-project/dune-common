# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
UseInkscape
-----------

Helpers for converting SVG images to PNG images with Inkscape.

.. cmake:command:: dune_create_inkscape_image_converter_target

  Create a custom target that converts SVG images to PNG files with Inkscape.

  The command always creates the requested target. If Inkscape is not found,
  the target remains empty and no images are converted.

  .. code-block:: cmake

    dune_create_inkscape_image_converter_target(
      TARGET <target-name>
      IMAGES <png-file>...
      [OUTPUT_DIR <directory>]
      [DPI <dpi>]
      [ALL]
    )

  ``TARGET``
    Name of the custom target to create.

  ``IMAGES``
    Output PNG file names. For each entry, the corresponding SVG input file is
    derived by replacing the file extension with ``.svg``.

  ``OUTPUT_DIR``
    Output directory for generated PNG files. Defaults to the current binary
    directory.

  ``DPI``
    DPI value used for the generated images. Defaults to ``90``.

  ``ALL``
    Add the generated target to the default ``all`` target.

#]=======================================================================]

include_guard(GLOBAL)

set_package_properties("Inkscape" PROPERTIES
  PURPOSE "Convert SVG images to PNG images for the LaTeX documentation")

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
