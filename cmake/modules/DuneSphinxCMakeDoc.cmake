# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Module to generate CMake API documentation with Sphinx
#
# .. cmake_function:: dune_cmake_sphinx_doc
#
#    .. cmake_brief::
#
#       Generate the documentation that you are just browsing!!!
#
#    .. cmake_param:: BUILDTYPE
#       :multi:
#
#       Set the type of build that is requested. By default, "html" is chosen.
#       The list of available build types:
#
#       * `html`
#
#    .. cmake_param:: SPHINX_CONF
#       :single:
#       :argname: conf
#
#       A template for a conf file to be passed to :code:`sphinx-build`.
#       The real configuration file will be generated through CMakes
#       :code:`configure_file` mechanism. A reasonable default file is
#       provided by dune-common. Only use this if you want to create
#       custom documentation.
#
#    .. cmake_param:: RST_SOURCES
#       :multi:
#       :argname: src
#
#       A list of rst sources, that should be configured into the build tree
#       (using :code:`configure_file`). If omitted, this defaults to
#       :code:`index.rst` and :code:`contents.rst` with suitable content.
#       Only use this if you want to create custom documentation.
#
#    .. cmake_param:: MODULE_ONLY
#       :option:
#
#       Only document CMake functionality from the current Dune module.
#
#    Generate a documentation for the CMake API. A set of cmake
#    modules defined by the parameters and all functions and macros
#    there in are automatically generated. The top level directory
#    of the documentation is the current build directory (aka the
#    directory that this function is called from)
#
#    There are some assumptions on how the documentation in
#    the CMake modules is written:
#
#    * At the beginning of each CMake module there is a comment block that is written in restructured text.
#      The first two characters of each line (the comment character
#      and a blank) are ignored. Any resulting content of lines most form valid rst.
#    * TODO document more
#
include_guard(GLOBAL)

find_package(Sphinx)
# text for feature summary
set_package_properties("Sphinx" PROPERTIES
  DESCRIPTION "Documentation generator"
  URL "www.sphinx-doc.org"
  PURPOSE "To generate the documentation from CMake and Python sources")

function(dune_cmake_sphinx_doc)
  # Only proceed if Sphinx was found on the system
  if(NOT SPHINX_FOUND)
    message("-- Skipping building CMake API documentation (Sphinx was not found!)")
    return()
  endif()

  # Only proceed if the python interpreter was found by cmake
  if(NOT Python3_Interpreter_FOUND)
    message("-- Skipping building CMake API documentation (Python interpreter was not found!)")
    return()
  endif()

  # Parse Arguments
  set(MULTI BUILDTYPE RST_SOURCES)
  cmake_parse_arguments(SPHINX_CMAKE "MODULE_ONLY" "SPHINX_CONF" "${MULTI}" ${ARGN})
  if(SPHINX_CMAKE_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_cmake_sphinx_doc: This often indicates typos!")
  endif()

  # Apply defaults
  if(NOT SPHINX_CMAKE_BUILDTYPE)
    set(SPHINX_CMAKE_BUILDTYPE html)
  endif()

  # Extract the script directory from dune-common
  dune_module_path(MODULE dune-common RESULT DUNE_SPHINX_EXT_PATH SCRIPT_DIR)

  # Find the configuration file template.
  if(NOT SPHINX_CMAKE_SPHINX_CONF)
    set(SPHINX_CMAKE_SPHINX_CONF ${DUNE_SPHINX_EXT_PATH}/conf.py.in)
  endif()

  # Apply defaults to the rst sources that are not module dependent.
  if(NOT SPHINX_CMAKE_RST_SOURCES)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/contents.rst "")
    set(SPHINX_CMAKE_RST_SOURCES ${DUNE_SPHINX_EXT_PATH}/index.rst.in ${CMAKE_CURRENT_BINARY_DIR}/contents.rst)
  endif()

  # Write the conf.py, which sets up Sphinx into the build directory
  configure_file(${SPHINX_CMAKE_SPHINX_CONF} ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

  # Check whether we need to look through all dependencies
  set(DOC_CMAKE_MODULES)
  if(NOT SPHINX_CMAKE_MODULE_ONLY)
    set(DOC_CMAKE_MODULES ${ALL_DEPENDENCIES})
  endif()

  # Now treat the module dependent rst sources.
  set(CMAKE_DOC_DEPENDENCIES "")
  set(${PROJECT_NAME}_PREFIX ${PROJECT_SOURCE_DIR})
  foreach(dep ${DOC_CMAKE_MODULES} ${PROJECT_NAME})
    # Look for a build system documentation exported by the module dep
    set(RSTFILE "")
    # check in the correct path for non-installed modules
    if(EXISTS ${${dep}_PREFIX}/doc/buildsystem/${dep}.rst)
      set(RSTFILE ${${dep}_PREFIX}/doc/buildsystem/${dep}.rst)
    endif()
    # now check for the correct path taking into account installed ones
    if(EXISTS ${${dep}_PREFIX}/share/doc/${dep}/${dep}.rst)
      set(RSTFILE ${${dep}_PREFIX}/share/doc/${dep}/${dep}.rst)
    endif()
    # Now process the file, if we have found one
    if(RSTFILE)
      # add it to index.rst then.
      set(CMAKE_DOC_DEPENDENCIES "${CMAKE_DOC_DEPENDENCIES}   ${dep}\n")
      # ... and copy the rst file to the current build.
      configure_file(${RSTFILE} ${CMAKE_CURRENT_BINARY_DIR}/${dep}.rst)
    endif()
  endforeach()

  # Write the non-module dependent rst source files from templates
  foreach(rstin ${SPHINX_CMAKE_RST_SOURCES})
    get_filename_component(rst ${rstin} NAME_WE)
    configure_file(${rstin} ${CMAKE_CURRENT_BINARY_DIR}/${rst}.rst)
  endforeach()

  # Generate the list of modules by looking through the module paths
  # of all dependencies for files matching *.cmake
  set(SPHINX_DOC_MODULE_LIST)
  set(${PROJECT_NAME}_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
  foreach(dep ${DOC_CMAKE_MODULES} ${PROJECT_NAME})
    file(GLOB modules "${${dep}_MODULE_PATH}/*.cmake")
    set(SPHINX_DOC_MODULE_LIST ${SPHINX_DOC_MODULE_LIST} ${modules})
  endforeach()

  # Initialize a variable that collects all dependencies of the documentation
  set(DOC_DEPENDENCIES)

  # Generate the rst files for all cmake modules
  foreach(module ${SPHINX_DOC_MODULE_LIST})
    get_filename_component(modname ${module} NAME)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/modules/${modname}
                       COMMAND ${Python3_EXECUTABLE} ${DUNE_SPHINX_EXT_PATH}/extract_cmake_data.py
                         --module=${module}
                         --builddir=${CMAKE_CURRENT_BINARY_DIR}
                       DEPENDS ${module}
                       COMMENT "Extracting CMake API documentation from ${modname}"
                      )
    set(DOC_DEPENDENCIES ${DOC_DEPENDENCIES} ${CMAKE_CURRENT_BINARY_DIR}/modules/${modname})
  endforeach()

  # Call Sphinx once for each requested build type
  foreach(type ${SPHINX_CMAKE_BUILDTYPE})
    # Call the sphinx executable
    add_custom_target(sphinx_${type}
                      COMMAND ${SPHINX_EXECUTABLE}
                                -b ${type}
                                -w ${PROJECT_BINARY_DIR}/SphinxError.log
                                -c ${CMAKE_CURRENT_BINARY_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}/${type}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      DEPENDS ${DOC_DEPENDENCIES}
                     )
    add_dependencies(doc sphinx_${type})
  endforeach()
endfunction()
