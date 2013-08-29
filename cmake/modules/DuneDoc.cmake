#
# Module that provides a custom target make doc at the top level
# directory and utility macros for creating install directives
# that make sure that the files to be installed are previously
# generated even if make doc was not called.
#
# All documentation (Latex, Doxygen) will be generated during
# make doc.
# It provides the following macros:
#
# create_doc_install(FILENAME  TARGETDIR [DEPENDENCY])
#
# creates a target for installing the file FILENAME
# to the directory TARGETDIR.
# If DEPENDENCY is specified, this is a dependency for
# the installation. Otherwise FILENAME becomes the dependency
#
# dune_add_latex_document()

FIND_PACKAGE(LATEX)
FIND_PROGRAM(IMAGEMAGICK_CONVERT convert
  DOC "The convert program that comes with ImageMagick (available at http://www.imagemagick.org)."
  )
set(LATEX_USABLE "ON")

if(NOT LATEX_COMPILER)
  message(WARNING " Need latex to create documentation!")
  set(LATEX_USABLE)
endif(NOT LATEX_COMPILER)
if(NOT BIBTEX_COMPILER)
  message(WARNING " Need bibtex to create documentation!")
  set(LATEX_USABLE)
endif(NOT BIBTEX_COMPILER)
if(NOT MAKEINDEX_COMPILER)
  message(WARNING " Need makeindex to create documentation!")
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


add_custom_target(doc)


MACRO(create_doc_install filename targetdir)
  dune_common_script_dir(SCRIPT_DIR)
  get_filename_component(targetfile ${filename} NAME)
  # The doc file might be in CMAKE_CURRENT_<SOURCE|BINARY>_DIR
  # Depending on whether this is a tarball or not
  set(_src_file _src_file-NOTFOUND)
  find_file(_src_file ${targetfile} ${CMAKE_CURRENT_SOURCE_DIR})
  if(NOT _src_file)
    set(_src_file ${filename})
    set(_need_to_generate TRUE)
  endif(NOT _src_file)
  set(install_command ${CMAKE_COMMAND} -D FILES=${_src_file} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${SCRIPT_DIR}/InstallFile.cmake)
  # create a custom target for the installation
  if("${ARGC}" EQUAL "3" AND _need_to_generate)
    set(_depends ${ARGV2})
  else("${ARGC}" EQUAL "3" AND _need_to_generate)
    set(_depends  ${_src_file})
  endif("${ARGC}" EQUAL "3" AND _need_to_generate)
  add_custom_target(install_${targetfile} ${install_command}
    COMMENT "Installing ${filename} to ${targetdir}"
    DEPENDS ${_depends})
  # When installing, call cmake install with the above install target and add the file to install_manifest.txt
  install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target install_${targetfile} )
            LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_PREFIX}/${targetdir}/${targetfile})")
ENDMACRO(create_doc_install)


MACRO(dune_add_latex_document tex_file)
  # We assume that we always generate a PDF file.
  # If the corresponding pdf file already exists in the source tree
  # we do not add a rule to build it.
  string(REGEX REPLACE "(.+).tex" "\\1.pdf" file ${tex_file})
  string(REGEX REPLACE "/" "_" "${CMAKE_CURRENT_SOURCE_DIR}/${file}" filevar ${file})
  set(filevar "filevar-NOTFOUND")
  find_file(filevar ${tex_file} ${CMAKE_CURRENT_SOURCE_DIR})
  if(filevar)
    if(LATEX_USABLE)
      # add rule to create latex document
      add_latex_document(${tex_file} ${ARGN})
    else(LATEX_USABLE)
      message(WARNING "Not adding rule to create ${file} as LaTEX is not usable!")
    endif(LATEX_USABLE)
  else(filevar)
    # Check for the pdf file
    set(pdffilevar "pdffilevar-NOTFOUND")
    find_file(pdffilevar ${file} ${CMAKE_CURRENT_SOURCE_DIR})
    if(NOT pdffilevar)
      message(SEND_ERROR "No tex source ${tex_file} and no generated ${file} found!")
    endif(NOT pdffilevar)
  endif(filevar)
ENDMACRO(dune_add_latex_document tex_file)

# Support building documentation with doxygen.
include(DuneDoxygen)
