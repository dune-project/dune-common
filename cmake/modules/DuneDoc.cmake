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
  set(LATEX_MANGLE_TARGET_NAMES "ON" CACHE INTERNAL "Mangle target names to allow multiple latex documents")
  include(UseLATEX)
endif(LATEX_USABLE)


# Module that provides a custom target make doc at the top level
# directory and utility macros for creating install directives
# that make sure that the files to be installed are previously
# generated even if make doc was not called.
#
add_custom_target(doc)


MACRO(create_doc_install filename targetdir)
  dune_common_script_dir(SCRIPT_DIR)
  get_filename_component(targetfile ${filename} NAME)
  set(install_command ${CMAKE_COMMAND} -D FILES=${filename} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${SCRIPT_DIR}/InstallFile.cmake)
  # create a custom target for the installation
  message("cdi ${filename}, ${ARGC}, ${ARGV2} ")
  if("${ARGC}" EQUAL "3")
    set(_depends ${ARGV2})
    message("_depends=${_depends}")
  else("${ARGC}" EQUAL "3")
    set(_depends  ${filename})
  endif("${ARGC}" EQUAL "3")
  add_custom_target(install_${targetfile} ${install_command}
    COMMENT "Installing ${filename} to ${targetdir}"
    DEPENDS ${_depends})
  # When installing, call cmake install with the above install target and add the file to install_manifest.txt
  install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target install_${targetfile} )
            LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_PREFIX}/${targetdir}/${targetfile})")
ENDMACRO(create_doc_install)

# Support building documentation with doxygen.
include(DuneDoxygen)
