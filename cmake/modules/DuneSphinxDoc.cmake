# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

include_guard(GLOBAL)
find_package(Sphinx)
find_package(Python3 COMPONENTS Interpreter Development)

function(dune_sphinx_doc)
  # Only proceed if Sphinx was found on the system
  if(NOT SPHINX_FOUND)
    message("-- Skipping building Sphinx documentation (Sphinx was not found!)")
    return()
  endif()

  # Only proceed if the python interpreter was found by cmake
  if(NOT Python3_Interpreter_FOUND)
    message("-- Skipping building Sphinx documentation (Python interpreter was not found!)")
    return()
  endif()

  # Parse Arguments
  cmake_parse_arguments(SPHINX_DOC "" "CONF" "BUILDTYPE" ${ARGN})
  if(SPHINX_DOC_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_sphinx_doc")
  endif()

  # copy conf.py into build directory
  if(NOT SPHINX_DOC_CONF)
    set(SPHINX_DOC_CONF conf.py)
  endif()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SPHINX_DOC_CONF}.in)
    configure_file(${SPHINX_DOC_CONF}.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)
  elseif(EXISTS ${CMAKE_CURRENT_SOUREC_DIR}/${SPHINX_DOC_CONF})
    configure_file(${SPHINX_DOC_CONF} ${CMAKE_CURRENT_BINARY_DIR}/conf.py COPYONLY)
  else()
    message(SEND_ERROR "Sphinx configuration '${SPHINX_DOC_CONF}' not found.")
  endif()

  # call Sphinx for each requested build type
  if(NOT SPHINX_DOC_BUILDTYPE)
    set(SPHINX_DOC_BUILDTYPE html)
  endif()
  foreach(type ${SPHINX_DOC_BUILDTYPE})
    add_custom_target(sphinx_doc_${type}
                      COMMAND ${SPHINX_EXECUTABLE}
                                -b ${type}
                                -w ${PROJECT_BINARY_DIR}/Sphinx-${type}.log
                                -c ${CMAKE_CURRENT_BINARY_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR}/${type}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/conf.py
                     )
    add_dependencies(sphinx_doc_${type} sphinx_files)
    add_dependencies(doc sphinx_doc_${type})
  endforeach()
endfunction()

function(add_sphinx_target base file)
  find_program(JUPYTER jupyter)
  get_filename_component(extension ${file} EXT)
  set(SPHINXDIR ${PROJECT_BINARY_DIR}/doc/sphinx)
  set(OUT ${SPHINXDIR}/${file})
  set(IN ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  string(REGEX REPLACE "\\.[^.]*$" "" filebase ${file})
  set(TARGET ${base}.${file})
  add_custom_target(${TARGET} DEPENDS ${OUT})
  add_dependencies(sphinx_files ${TARGET})
  add_custom_command(
    OUTPUT ${OUT}
    DEPENDS ${IN}
    COMMAND ${CMAKE_COMMAND} -E copy ${IN} ${OUT}
    VERBATIM
  )
  if ("${extension}" STREQUAL ".ipynb")
    if (JUPYTER)
      set(TARGET ${base}.${filebase}.rst)
      set(OUTRST ${SPHINXDIR}/${filebase}.rst)
      add_custom_target(${TARGET} DEPENDS ${OUTRST})
      add_dependencies(sphinx_files ${TARGET})
      add_custom_command(
        OUTPUT ${OUTRST}
        DEPENDS ${OUT}
        COMMAND jupyter nbconvert --ExecutePreprocessor.timeout=-1 --execute --allow-errors --to="rst" ${OUT} --output ${filebase}.rst
        COMMAND sed -i "s/raw:: latex/math::/g" ${OUTRST}
        WORKING_DIRECTORY ${SPHINXDIR}
        VERBATIM
      )
    endif()
  endif()
endfunction()

function(add_sphinx_files base)
  foreach(file ${ARGN})
    add_sphinx_target(${base} ${file})
  endforeach()
endfunction()
function(add_sphinx_targets base)
  add_custom_target(sphinx_files)
  add_sphinx_files(${base} ${ARGN})
  dune_sphinx_doc()
endfunction()
