FIND_PACKAGE(LATEX)
FIND_PROGRAM(IMAGEMAGICK_CONVERT convert
  DOC "The convert program that comes with ImageMagick (available at http://www.imagemagick.org)."
  )
set(LATEX_USABLE "ON")

if(NOT LATEX_COMPILER)
  message(WARING " Need latex to create documentation!")
  set(LATEX_USABLE)
endif(NOT LATEX_COMPILER)
if(NOT BIBTEX_COMPILER)
  message(WARING " Need bibtex to create documentation!")
  set(LATEX_USABLE)
endif(NOT BIBTEX_COMPILER)
if(NOT MAKEINDEX_COMPILER)
  message(WARING " Need makeindex to create documentation!")
  set(LATEX_USABLE)
endif(NOT MAKEINDEX_COMPILER)
if(NOT IMAGEMAGICK_CONVERT)
  message(WARNING " Need imagemagick to create latex documentation!")
  set(LATEX_USABLE)
endif(NOT IMAGEMAGICK_CONVERT)
if(LATEX_USABLE)
  set(LATEX_MANGLE_TARGET_NAMES "ON" CACHE INTERNAL "Mangel target names to allow multiple latex documents")
  include(UseLATEX)
endif(LATEX_USABLE)


# Module that provides a custom target make doc at the top level
# directory and utility macros for creating install directives
# that make sure that the files to be installed are previously
# generated even if make doc was not called.
#
add_custom_target(doc)


MACRO(create_doc_install filename targetdir)
  if("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(SCRIPT_DIR ${CMAKE_SOURCE_DIR}/cmake/scripts)
  else("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(SCRIPT_DIR ${DUNE_COMMON_SCRIPT_DIR})
  endif("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
  get_filename_component(targetfile ${filename} NAME)
  set(install_command ${CMAKE_COMMAND} -D FILES=${filename} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${SCRIPT_DIR}/InstallFile.cmake)
  # create a custom target for the installation
  add_custom_target(install_${targetfile} ${install_command}
    COMMENT "Installing ${filename} to ${targetdir}"
    DEPENDS ${filename})
  # When installing, call cmake install with the above install target and add the file to install_manifest.txt
  install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target install_${targetfile} )
            LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_PREFIX}/${targetdir}/${targetfile})")
ENDMACRO(create_doc_install)

# Support building documentation with doxygen.
include(DuneDoxygen)
