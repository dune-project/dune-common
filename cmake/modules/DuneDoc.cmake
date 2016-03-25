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
# .. cmake_function:: dune_add_latex_document
#
#    .. cmake_brief::
#
#       build a pdf document through the Dune build system.
#
#    .. cmake_param:: texfile
#       :single:
#       :required:
#       :positional:
#
#       The texfile to compile into a pdf.
#
#    .. note::
#
#       This function will be rewritten for Dune 3.0 as it currently
#       shadows all options provided by the base implementation
#       :code:`add_latex_document`.
#
# .. cmake_function:: create_doc_install
#
#    TODO doc me
#    What are use cases for this function?
#

find_package(LATEX)
find_program(IMAGEMAGICK_CONVERT convert
  DOC "The convert program that comes with ImageMagick (available at http://www.imagemagick.org)."
  )
set(LATEX_USABLE TRUE)

# UseLATEX.cmake does only work in out-of-source builds
if(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  message(WARNING "In-source detected, disabling LaTeX documentation. Use an out-of-source build to generate documentation.")
  set(LATEX_USABLE FALSE)
endif()
# check needed LaTeX executables
if(NOT LATEX_COMPILER)
  message(WARNING " Need latex to create documentation!")
  set(LATEX_USABLE FALSE)
endif()
if(NOT BIBTEX_COMPILER)
  message(WARNING " Need bibtex to create documentation!")
  set(LATEX_USABLE FALSE)
endif()
if(NOT MAKEINDEX_COMPILER)
  message(WARNING " Need makeindex to create documentation!")
  set(LATEX_USABLE FALSE)
endif()
if(NOT IMAGEMAGICK_CONVERT)
  message(WARNING " Need imagemagick to create latex documentation!")
  set(LATEX_USABLE FALSE)
endif()
if(LATEX_USABLE)
  set(LATEX_MANGLE_TARGET_NAMES "ON" CACHE INTERNAL "Mangle target names to allow multiple latex documents")
  include(UseLATEX)
endif()


add_custom_target(doc)

# add the Sphinx-generated build system documentation
include(DuneSphinxCMakeDoc)


macro(create_doc_install filename targetdir)
  dune_module_path(MODULE dune-common RESULT scriptdir SCRIPT_DIR)
  get_filename_component(targetfile ${filename} NAME)
  # The doc file might be in CMAKE_CURRENT_<SOURCE|BINARY>_DIR
  # Depending on whether this is a tarball or not
  set(_src_file _src_file-NOTFOUND)
  find_file(_src_file ${targetfile} ${CMAKE_CURRENT_SOURCE_DIR})
  if(NOT _src_file)
    set(_src_file ${filename})
    set(_need_to_generate TRUE)
  endif(NOT _src_file)
  set(install_command ${CMAKE_COMMAND} -D FILES=${_src_file} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${scriptdir}/InstallFile.cmake)
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
endmacro(create_doc_install)


macro(dune_add_latex_document tex_file)
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
      add_latex_document(${tex_file} ${ARGN} MANGLE_TARGET_NAMES)
    else(LATEX_USABLE)
      message(WARNING "Not adding rule to create ${file} as LaTeX is not usable!")
    endif(LATEX_USABLE)
  else(filevar)
    # Check for the pdf file
    set(pdffilevar "pdffilevar-NOTFOUND")
    find_file(pdffilevar ${file} ${CMAKE_CURRENT_SOURCE_DIR})
    if(NOT pdffilevar)
      message(SEND_ERROR "No tex source ${tex_file} and no generated ${file} found!")
    endif(NOT pdffilevar)
  endif(filevar)
endmacro(dune_add_latex_document tex_file)

# Support building documentation with doxygen.
include(DuneDoxygen)
