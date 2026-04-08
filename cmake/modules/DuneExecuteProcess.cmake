# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneExecuteProcess
==================

Wrapper around :dune:cmake-command:`execute_process` with optional error and
warning handling.

.. cmake:command:: dune_execute_process

  Run :dune:cmake-command:`execute_process` and optionally stop or warn on
  non-zero exit status.

  .. code-block:: cmake

    dune_execute_process(
      [ERROR_MESSAGE <message>]
      [WARNING_MESSAGE <message>]
      [WORKING_DIRECTORY <dir>]
      [RESULT_VARIABLE <var>]
      [OUTPUT_VARIABLE <var>]
      [ERROR_VARIABLE <var>]
      COMMAND <command> [<args>...]
    )

  ``ERROR_MESSAGE``
    Fatal error message emitted when the command exits with non-zero status.

  ``WARNING_MESSAGE``
    Warning message emitted when the command exits with non-zero status.

  ``WORKING_DIRECTORY``
    Working directory for the spawned process. The default is
    ``${CMAKE_CURRENT_BINARY_DIR}``.

  ``RESULT_VARIABLE``
    Variable receiving the command exit status.

  ``OUTPUT_VARIABLE``
    Variable receiving standard output.

  ``ERROR_VARIABLE``
    Variable receiving standard error.

  If neither ``ERROR_MESSAGE`` nor ``WARNING_MESSAGE`` is given, the command
  behaves like a thin forwarding wrapper around
  :dune:cmake-command:`execute_process`.

#]=======================================================================]
include_guard(GLOBAL)

function(dune_execute_process)
  set(SINGLEARGS ERROR_MESSAGE WARNING_MESSAGE RESULT_VARIABLE OUTPUT_VARIABLE ERROR_VARIABLE WORKING_DIRECTORY)
  cmake_parse_arguments(EXECUTE "" "${SINGLEARGS}" "" ${ARGN})

  # Decide whether stdout and stderr have to be split
  if(EXECUTE_OUTPUT_VARIABLE AND EXECUTE_ERROR_VARIABLE)
    set(SPLIT_ERROR TRUE)
    set(ERRLOGVAR errlog)
  else()
    set(SPLIT_ERROR FALSE)
    set(ERRLOGVAR log)
  endif()

  if (NOT EXECUTE_WORKING_DIRECTORY)
    set(EXECUTE_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  # Call the original cmake function
  execute_process(${EXECUTE_UNPARSED_ARGUMENTS}
                  WORKING_DIRECTORY ${EXECUTE_WORKING_DIRECTORY}
                  RESULT_VARIABLE retcode
                  OUTPUT_VARIABLE log
                  ERROR_VARIABLE ${ERRLOGVAR}
                  )

  # Issue an error if requested!
  if(EXECUTE_ERROR_MESSAGE)
    if(NOT "${retcode}" STREQUAL "0")
      cmake_parse_arguments(ERR "" "" "COMMAND" ${EXECUTE_UNPARSED_ARGUMENTS})
      if(SPLIT_ERROR)
        set(log "stdout:\n${log}\n\nstderr:\b${errlog}")
      endif()
      message(FATAL_ERROR "${EXECUTE_ERROR_MESSAGE}\nRun command:${ERR_COMMAND}\nReturn code: ${retcode}\nDetailed log:\n${log}")
    endif()
  elseif(EXECUTE_WARNING_MESSAGE)
    if(NOT "${retcode}" STREQUAL "0")
      cmake_parse_arguments(ERR "" "" "COMMAND" ${EXECUTE_UNPARSED_ARGUMENTS})
      if(SPLIT_ERROR)
        set(log "stdout:\n${log}")
      endif()
      message(WARNING "${EXECUTE_WARNING_MESSAGE}\nRun command:${ERR_COMMAND}\nReturn code: ${retcode}\nOutput:\n${log}")
    endif()
  endif()

  # Propagate logs back to the calling scope
  if(EXECUTE_RESULT_VARIABLE)
    set(${EXECUTE_RESULT_VARIABLE} ${retcode} PARENT_SCOPE)
  endif()
  if(EXECUTE_OUTPUT_VARIABLE)
    set(${EXECUTE_OUTPUT_VARIABLE} ${log} PARENT_SCOPE)
  endif()
  if(EXECUTE_ERROR_VARIABLE)
    set(${EXECUTE_ERROR_VARIABLE} ${${ERROR_VARIABLE}} PARENT_SCOPE)
  endif()
endfunction()
