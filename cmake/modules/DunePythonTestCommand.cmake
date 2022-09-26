# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Wrap python testing commands into the CMake build system
#
# .. cmake_function:: dune_python_add_test
#
#    .. cmake_param:: SCRIPT
#       :multi:
#
#       The script to execute using the python interpreter. It will be executed during :code:`make test_python`
#       and during `ctest`. You are required to either pass SCRIPT or MODULE.
#
#       .. note::
#
#    .. cmake_param:: MODULE
#       :multi:
#
#       The Python module to execute using the python interpreter. It will be executed during :code:`make test_python`
#       and during `ctest`. You are required to either pass SCRIPT or MODULE.
#
#    .. cmake_param:: LABELS
#       :multi:
#
#       A list of labels to add to the test. This sets
#       the LABELS property on the test so :code:`ctest -L ${label_regex}` can
#       be used to run all tests with certain labels. We always add the label
#       :code:`python` per default. The labels are forwarded to
#       :ref:`dune_add_test`. See :ref:`dune_add_test` for a
#       more detailed documentation.
#
#    .. cmake_param:: WORKING_DIRECTORY
#       :single:
#       :argname: dir
#
#       The working directory of the command. Defaults to
#       the current build directory.
#
#    .. cmake_param:: TIMEOUT
#       :single:
#
#       If set, the test will time out after the given number of seconds. This supersedes
#       any timeout setting in ctest (see `cmake --help-property TIMEOUT`). If you
#       specify the MPI_RANKS option, you need to specify a TIMEOUT.
#
#    .. cmake_param:: NAME
#       :single:
#
#       A name to identify this test in ctest. Names must be unique throughout
#       the project. If omitted, defaults to mangling of the command.
#
#    Integrates a python testing framework command into the Dune
#    build system. Added commands are run, when the target
#    :code:`test_python` is built and during :code:`ctest`.
#
#    This function uses `dune_add_test` and forwards all unparsed arguments.
#
include_guard(GLOBAL)

function(dune_python_add_test)
  # Parse Arguments
  set(OPTION)
  set(SINGLE NAME WORKING_DIRECTORY)
  set(MULTI SCRIPT MODULE LABELS TIMEOUT)
  cmake_parse_arguments(PYTEST "" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYTEST_COMMAND)
    message(FATAL_ERROR "dune_python_add_test: COMMAND argument should not be used, use SCRIPT instead providing only the Python script and not the Python interpreter")
  endif()

  # Apply defaults
  set(PYTEST_LABELS ${PYTEST_LABELS} python)
  if(NOT PYTEST_TIMEOUT)
    set(PYTEST_TIMEOUT 3600)
  endif()

  if(NOT PYTEST_WORKING_DIRECTORY)
    set(PYTEST_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if((NOT PYTEST_MODULE) AND (NOT PYTEST_SCRIPT))
    message(FATAL_ERROR "dune_python_add_test: Either SCRIPT or MODULE need to be specified!")
  endif()
  if(PYTEST_MODULE AND PYTEST_SCRIPT)
    message(FATAL_ERROR "dune_python_add_test: You can only specify either SCRIPT or MODULE, not both!")
  endif()
  if(PYTEST_MODULE)
    set(PYTEST_SCRIPT -m ${PYTEST_MODULE})
  endif()
  if(NOT PYTEST_NAME)
    set(commandstr "")
    foreach(comm ${PYTEST_SCRIPT})
      set(commandstr "${commandstr}_${comm}")
    endforeach()
    set(commandstr "${commandstr}_${PYTEST_WORKING_DIRECTORY}")
    string(REPLACE "/" "_" PYTEST_NAME ${commandstr})
  endif()

  set(PYTEST_COMMAND ${CMAKE_BINARY_DIR}/run-in-dune-env)
  set(PYTEST_CMD_ARGS python ${PYTEST_SCRIPT})

  add_custom_target(target_${PYTEST_NAME})
  dune_add_test(NAME ${PYTEST_NAME}
                TARGET target_${PYTEST_NAME}
                COMMAND ${PYTEST_COMMAND}
                CMD_ARGS ${PYTEST_CMD_ARGS}
                PYTHON_TEST
                LABELS ${PYTEST_LABELS}
                WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY}
                TIMEOUT ${PYTEST_TIMEOUT}
                ${PYTEST_UNPARSED_ARGUMENTS}
               )
endfunction()
