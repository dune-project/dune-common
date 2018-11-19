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
#    .. cmake_brief::
#
#       creates a target for creating and installing a file
#       to a given directory.
#
#    .. cmake_param:: filename
#       :single:
#       :required:
#       :positional:
#
#       The name of the file to be installed.
#
#    .. cmake_param:: targetdir
#       :single:
#       :required:
#       :positional:
#
#       The directory into which the beforementioned file will be installed.
#
#    .. cmake_param:: dependency
#       :single:
#       :required:
#       :positional:
#
#       A target that gets called to create the file that will be installed.
#
#    .. note::
#
#       This macro is needed, as we cannot add dependencies to the install
#       target. See https://gitlab.kitware.com/cmake/cmake/issues/8438
#       and https://gitlab.dune-project.org/core/dune-common/issues/36
#

find_package(LATEX)
set_package_properties("LATEX" PROPERTIES
  DESCRIPTION "Type setting system"
  PURPOSE "To generate the documentation")
find_program(IMAGEMAGICK_CONVERT convert)
set(HAVE_IMAGEMAGICK_CONVERT IMAGEMAGICK_CONVERT)
set_package_properties("IMAGEMAGICK_CONVERT" PROPERTIES
  DESCRIPTION "convert program that comes with ImageMagick"
  URL "www.imagemagick.org"
  PURPOSE "To generate the documentation with LaTeX")
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
  set_package_properties("UnixCommands" PROPERTIES
    DESCRIPTION "Some common Unix commands"
    PURPOSE "To generate the documentation with LaTeX")
endif()

add_custom_target(doc)

# add the Sphinx-generated build system documentation
include(DuneSphinxCMakeDoc)
# Support building documentation with doxygen.
include(DuneDoxygen)

macro(create_doc_install filename targetdir dependency)
  if(LATEX_USABLE)
    dune_module_path(MODULE dune-common RESULT scriptdir SCRIPT_DIR)
    get_filename_component(targetfile ${filename} NAME)
    set(install_command ${CMAKE_COMMAND} -D FILES=${filename} -D DIR=${CMAKE_INSTALL_PREFIX}/${targetdir} -P ${scriptdir}/InstallFile.cmake)
    # create a custom target for the installation
    add_custom_target(install_${targetfile} ${install_command}
      COMMENT "Installing ${filename} to ${targetdir}"
      DEPENDS ${dependency})
    # When installing, call cmake install with the above install target and add the file to install_manifest.txt
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target install_${targetfile} )
              LIST(APPEND CMAKE_INSTALL_MANIFEST_FILES ${CMAKE_INSTALL_PREFIX}/${targetdir}/${targetfile})")
  endif()
endmacro(create_doc_install)


macro(dune_add_latex_document tex_file)
  set(latex_arguments "${ARGN}")

  if(LATEX_USABLE)
    # add rule to create latex document
    add_latex_document(${tex_file} ${latex_arguments}
      EXCLUDE_FROM_ALL
      EXCLUDE_FROM_DEFAULTS)
    # add dependency for target doc, but first construct document's target name
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
