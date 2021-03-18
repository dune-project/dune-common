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
#          The script will be executed using :code:`${Python3_EXECUTABLE} SCRIPT`. If the INTERPRETER
#          option is given, that interpreter is used instead.
#
#    .. cmake_param:: MODULE
#       :multi:
#
#       The Python module to be executed.  It will be executed during :code:`make test_python`
#       and during `ctest`. You are required to either pass SCRIPT or MODULE.
#
#       .. note::
#
#          The script will be executed using :code:`${Python3_EXECUTABLE} -m MODULE`. If the INTERPRETER
#          option is given, that interpreter is used instead.
#
#    .. cmake_param:: INTERPRETER
#       :single:
#
#       The Python interpreter to use for this test. It defaults to the one found by CMake.
#
#    .. cmake_param:: WORKING_DIRECTORY
#       :single:
#       :argname: dir
#
#       The working directory of the command. Defaults to
#       the current build directory.
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
include_guard(GLOBAL)

function(dune_python_add_test)
  # Parse Arguments
  include(CMakeParseArguments)
  set(OPTION)
  set(SINGLE WORKING_DIRECTORY NAME INTERPRETER)
  set(MULTI SCRIPT COMMAND LABELS MODULE)

  cmake_parse_arguments(PYTEST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYTEST_COMMAND)
    message(FATAL_ERROR "dune_python_add_test: COMMAND argument should not be used, use SCRIPT instead providing only the Python script and not the Python interpreter")
  endif()
  if(PYTEST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_add_test: This often indicates typos!")
  endif()

  # Apply defaults
  if(NOT PYTEST_INTERPRETER)
    set(PYTEST_INTERPRETER ${Python3_EXECUTABLE})
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

  # Actually run the command
  add_custom_target(target_${PYTEST_NAME}
                    COMMAND ${PYTEST_INTERPRETER} ${PYTEST_SCRIPT}
                    WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY})

  # Build this during make test_python
  add_dependencies(test_python target_${PYTEST_NAME})

  # make sure each label exists and its name is acceptable
  dune_declare_test_label(LABELS ${PYTEST_LABELS})
  # Also build this during ctest
  _add_test(NAME ${PYTEST_NAME}
            COMMAND ${PYTEST_INTERPRETER} ${PYTEST_SCRIPT}
            WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY}
            )
  # Set the labels on the test
  set_tests_properties(${PYTEST_NAME} PROPERTIES LABELS "${PYTEST_LABELS}")
endfunction()
