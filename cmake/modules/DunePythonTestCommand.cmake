# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DunePythonTestCommand
---------------------

Helpers for integrating Python-based tests into the DUNE test infrastructure.

.. cmake:command:: dune_python_add_test

  Register a Python script or module as a DUNE test.

  .. code-block:: cmake

    dune_python_add_test(
      [SCRIPT <script...>]
      [MODULE <module...>]
      [LABELS <labels...>]
      [WORKING_DIRECTORY <dir>]
      [TIMEOUT <seconds>]
      [NAME <name>]
      ...
    )

  ``SCRIPT``
    Script path to execute with the Python interpreter. Pass the script path
    only, not the interpreter itself. Either ``SCRIPT`` or ``MODULE`` must be
    provided.

  ``MODULE``
    Python module to execute with ``python -m``. Either ``SCRIPT`` or
    ``MODULE`` must be provided.

  ``LABELS``
    Labels attached to the test. The label ``python`` is added by default. The
    labels are forwarded to :cmake:command:`dune_add_test()`.

  ``WORKING_DIRECTORY``
    Working directory used for the command. The default is the current build
    directory.

  ``TIMEOUT``
    Timeout in seconds. This overrides the default ctest timeout.

  ``NAME``
    Explicit ctest name for the generated test. If omitted, the name is
    derived from the command and working directory.

  The command integrates a Python-based test into the DUNE build system so it
  is built through ``test_python`` and executed through ``ctest``. Unparsed
  arguments are forwarded to :cmake:command:`dune_add_test()`.

#]=======================================================================]
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
  set(PYTEST_CMD_ARGS python3 ${PYTEST_SCRIPT})

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
