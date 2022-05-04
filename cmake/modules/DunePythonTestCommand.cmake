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
#          The script will be executed using :code:`${Python3_EXECUTABLE} SCRIPT`.
#
#    .. cmake_param:: MODULE
#       :multi:
#
#       The Python module to execute using the python interpreter. It will be executed during :code:`make test_python`
#       and during `ctest`. You are required to either pass SCRIPT or MODULE.
#
#       .. note::
#
#          The script will be executed using :code:`${Python3_EXECUTABLE} -m MODULE`.
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
  set(SINGLE NAME)
  set(MULTI SCRIPT MODULE CMAKE_GUARD LABELS)
  cmake_parse_arguments(PYTEST "" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYTEST_COMMAND)
    message(FATAL_ERROR "dune_python_add_test: COMMAND argument should not be used, use SCRIPT instead providing only the Python script and not the Python interpreter")
  endif()

  # Apply defaults
  set(PYTEST_CMAKE_GUARD ${PYTEST_CMAKE_GUARD} DUNE_ENABLE_PYTHONBINDINGS)
  set(PYTEST_LABELS ${PYTEST_LABELS} python)

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
                CMAKE_GUARD ${PYTEST_CMAKE_GUARD}
                LABELS ${PYTEST_LABELS}
                ${PYTEST_UNPARSED_ARGUMENTS}
               )
endfunction()
