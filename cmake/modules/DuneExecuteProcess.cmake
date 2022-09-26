# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# An error checking wrapper around the cmake command execute_process
#
# .. cmake_command:: dune_execute_process
#
#    .. cmake_param:: ERROR_MESSAGE
#       :single:
#
#       Error message to show if command exited with non-zero exit code.
#       This also implies abortion of the current cmake run with a fatal error.
#
#    .. cmake_param:: WARNING_MESSAGE
#       :single:
#
#       A warning message to show if command exited with non-zero exit code.
#       This will not abort the current cmake run.
#
#    .. cmake_param:: WORKING_DIRECTORY
#       :single:
#
#       The named directory will be set as the current working directory of the
#       child processes.
#
#    Note, that if neither warning or error message is provided, no return
#    code checking is done. If both are given the error message will be
#    used and the cmake run aborted.
#
#    A thin wrapper around the cmake command :code:`execute_process`, that
#    exits on non-zero exit codes. All arguments are forwarded to the actual
#    cmake command.
#
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
