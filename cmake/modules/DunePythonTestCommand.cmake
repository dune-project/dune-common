# Wrap python testing commands into the CMake build system
#
# .. cmake_function:: dune_python_add_test
#
#    .. cmake_param:: COMMAND
#       :multi:
#       :required:
#
#       The command to run. It will be executed during :code:`make test_python`
#       and during `ctest`.
#
#       .. note::
#
#          If your testing command involves an invocation of the python
#          interpreter you should use :code:`${PYTHON_EXECUTABLE}` for that.
#          Also calling python executables through :code:`-m` is generally to
#          be favored, e.g. :code:`${PYTHON_EXECUTABLE} -m pytest` instead of
#          :code:`py.test`.
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

function(dune_python_add_test)
  # Parse Arguments
  include(CMakeParseArguments)
  set(OPTION)
  set(SINGLE WORKING_DIRECTORY NAME)
  set(MULTI COMMAND LABELS)
  cmake_parse_arguments(PYTEST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYTEST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_add_test: This often indicates typos!")
  endif()

  # Apply defaults
  if(NOT PYTEST_WORKING_DIRECTORY)
    set(PYTEST_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  if(NOT PYTEST_COMMAND)
    message(FATAL_ERROR "dune_python_add_test: no COMMAND to execute specified!")
  endif()
  if(NOT PYTEST_NAME)
    set(commandstr "")
    foreach(comm ${PYTEST_COMMAND})
      set(commandstr "${commandstr}_${comm}")
    endforeach()
    set(commandstr "${commandstr}_${PYTEST_WORKING_DIRECTORY}")
    string(REPLACE "/" "_" PYTEST_NAME ${commandstr})
  endif()

  # Actually run the command
  add_custom_target(target_${PYTEST_NAME}
                    COMMAND ${PYTEST_COMMAND}
                    WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY})

  # Build this during make test_python
  add_dependencies(test_python target_${PYTEST_NAME})

  # make sure each label exists and its name is acceptable
  dune_declare_test_label(LABELS ${PYTEST_LABELS})
  # Also build this during ctest
  _add_test(NAME ${PYTEST_NAME}
            COMMAND ${PYTEST_COMMAND}
            WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY}
            )
  # Set the labels on the test
  set_tests_properties(${PYTEST_NAME} PROPERTIES LABELS "${PYTEST_LABELS}")
endfunction()
