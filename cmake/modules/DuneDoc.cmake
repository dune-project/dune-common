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
  include(UseLATEX)
endif()

add_custom_target(doc)

# add the Sphinx-generated build system documentation
include(DuneSphinxCMakeDoc)
# Support building documentation with doxygen.
include(DuneDoxygen)

macro(dune_add_latex_document tex_file)
  set(latex_arguments "${ARGN}")
  # replace old DEFAULT_SAFEPDF by FORCE_DVI and emit warning
  # this compatibility code can be removed after Dune 3.0
  list(FIND latex_arguments DEFAULT_SAFEPDF position_safepdf)
  if(NOT position_safepdf EQUAL -1)
    list(REMOVE_AT latex_arguments ${position_safepdf})
    list(APPEND latex_arguments FORCE_DVI)
    message(WARNING "dune_add_latex_document's argument DEFAULT_SAFEPDF is deprecated, use FORCE_DVI instead")
  endif()
  # replace old DEFAULT_PDF and emit warning
  # this compatibility code can be removed after Dune 3.0
  list(FIND latex_arguments DEFAULT_PDF position_defaultpdf)
  if(NOT position_defaultpdf EQUAL -1)
    list(REMOVE_AT latex_arguments ${position_defaultpdf})
    message(WARNING "dune_add_latex_document's argument DEFAULT_PDF is deprecated, just drop it")
  endif()
  # remove old argument FATHER_TARGET which is dropped and handled by an automatism now
  # this compatibility code can be removed after Dune 3.0
  list(FIND latex_arguments FATHER_TARGET position_fathertarget)
  if(NOT position_fathertarget EQUAL -1)
    list(REMOVE_AT latex_arguments ${position_fathertarget})
    # father has an according argument trailing which is removed, too
    list(REMOVE_AT latex_arguments ${position_fathertarget})
    message(WARNING "dune_add_latex_document's argument FATHER_TARGET <target> is deprecated, this is handled automatically now")
  endif()

  if(LATEX_USABLE)
    # add rule to create latex document
    add_latex_document(${tex_file} ${latex_arguments}
      EXCLUDE_FROM_ALL
      EXCLUDE_FROM_DEFAULTS)
    # add tependency for target doc, but first construct document's target name
    string(REGEX REPLACE "(.+).tex" "\\1" tex_file_base_name ${tex_file})
    list(FIND latex_arguments FORCE_DVI has_forcedvi)
    if(has_forcedvi EQUAL -1)
      add_dependencies(doc "${tex_file_base_name}")
    else()
      add_dependencies(doc "${tex_file_base_name}_safepdf")
    endif()
  else()
    message(WARNING "Not adding rule to create ${file} as LaTeX is not usable!")
  endif()
endmacro(dune_add_latex_document tex_file)

# this compatibility code can be removed after Dune 3.0
macro(create_doc_install)
  message(WARNING "create_doc_install is no longer needed, you can install these files directly")
endmacro(create_doc_install)
