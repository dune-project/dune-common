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

find_package(Sphinx)
find_package(PythonInterp)


function(dune_cmake_sphinx_doc)
  # Only proceed if Sphinx was found on the system
  if(NOT SPHINX_FOUND)
    message("-- Skipping building CMake API documentation (Sphinx was not found!)")
    return()
  endif()

  # Only proceed if the python interpreter was found by cmake
  if(NOT PYTHONINTERP_FOUND)
    message("-- Skipping building CMake API documentation (Python interpreter was not found!)")
    return()
  endif()

  # Parse Arguments
  set(OPTION)
  set(SINGLE)
  set(MULTI BUILDTYPE)
  include(CMakeParseArguments)
  cmake_parse_arguments(SPHINX_CMAKE "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(SPHINX_CMAKE_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_cmake_sphinx_doc: This often indicates typos!")
  endif()

  # Apply defaults
  if(NOT SPHINX_CMAKE_BUILDTYPE)
    set(SPHINX_CMAKE_BUILDTYPE html)
  endif()

  # Write the conf.py, which sets up Sphinx into the build directory
  set(DUNE_SPHINX_EXT_PATH)
  dune_common_script_dir(DUNE_SPHINX_EXT_PATH)
  configure_file(${DUNE_SPHINX_EXT_PATH}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

  # Write the index.rst and the contents.rst files from templates
  set(CMAKE_DOC_DEPENDENCIES "")
  set(${CMAKE_PROJECT_NAME}_PREFIX ${CMAKE_SOURCE_DIR})
  foreach(dep ${ALL_DEPENDENCIES} ${CMAKE_PROJECT_NAME})
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
  configure_file(${DUNE_SPHINX_EXT_PATH}/index.rst.in ${CMAKE_CURRENT_BINARY_DIR}/index.rst)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/contents.rst "")

  # Generate the list of modules by looking through the module paths
  # of all dependencies for files matching *.cmake
  set(SPHINX_DOC_MODULE_LIST)
  set(${CMAKE_PROJECT_NAME}_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
  foreach(dep ${ALL_DEPENDENCIES} ${CMAKE_PROJECT_NAME})
    file(GLOB modules "${${dep}_MODULE_PATH}/*.cmake")
    set(SPHINX_DOC_MODULE_LIST ${SPHINX_DOC_MODULE_LIST} ${modules})
  endforeach()

  # Initialize a variable that collects all dependencies of the documentation
  set(DOC_DEPENDENCIES)

  # Generate the rst files for all cmake modules
  foreach(module ${SPHINX_DOC_MODULE_LIST})
    get_filename_component(modname ${module} NAME)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/modules/${modname}
                       COMMAND ${PYTHON_EXECUTABLE} ${DUNE_SPHINX_EXT_PATH}/extract_cmake_data.py
                         --module=${module}
                         --builddir=${CMAKE_CURRENT_BINARY_DIR}
                       DEPENDS ${module}
                       COMMENT "Extracting CMake API documentation from ${modname}"
                      )
    set(DOC_DEPENDENCIES ${DOC_DEPENDENCIES} ${CMAKE_CURRENT_BINARY_DIR}/modules/${modname})
  endforeach()

  # copy the rst files that are fixed to the build directory during configure
  file(GLOB rstfiles "${CMAKE_CURRENT_SOURCE_DIR}/*.rst")
  foreach(rst ${rstfiles})
    get_filename_component(rstname ${rst} NAME)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${rstname}
                       COMMAND ${CMAKE_COMMAND} -E copy ${rst} ${CMAKE_CURRENT_BINARY_DIR}
                       DEPENDS ${rst})
    set(DOC_DEPENDENCIES ${DOC_DEPENDENCIES} ${CMAKE_CURRENT_BINARY_DIR}/${rstname})
  endforeach()

  # Call Sphinx once for each requested build type
  foreach(type ${SPHINX_CMAKE_BUILDTYPE})
    # Call the sphinx executable
    add_custom_target(sphinx_${type}
                      COMMAND ${SPHINX_EXECUTABLE}
                                -b ${type}
                                -w ${CMAKE_BINARY_DIR}/SphinxError.log
                                -c ${CMAKE_CURRENT_BINARY_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}/${type}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      DEPENDS ${DOC_DEPENDENCIES}
                     )
    add_dependencies(doc sphinx_${type})
  endforeach()
endfunction()
