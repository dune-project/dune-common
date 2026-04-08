# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneTestMacros
---------------

Tools for integrating tests into the DUNE build system.

.. cmake:command:: dune_declare_test_label

  Declare labels for :cmake:command:`dune_add_test()`.

  .. code-block:: cmake

    dune_declare_test_label(LABELS <labels...>)

  ``LABELS``
    Names of labels to declare. Label names must be non-empty and may contain
    only alphanumeric characters plus ``-`` and ``_`` so they remain easy to
    use in ``ctest -L`` regular expressions.

  Labels are normally declared on demand by :cmake:command:`dune_add_test()`.
  Call this command explicitly if you want ``build_<label>_tests`` targets to
  exist even when no test with that label is added in the current configuration.
  The label ``quick`` is always predeclared.

.. cmake:command:: dune_add_test

  Add a test to the DUNE testing suite.

  .. code-block:: cmake

    dune_add_test(
      [NAME <name>]
      [SOURCES <sources...>]
      [TARGET <target>]
      [COMPILE_DEFINITIONS <def>...]
      [COMPILE_FLAGS <flag>...]
      [LINK_LIBRARIES <lib>...]
      [EXPECT_COMPILE_FAIL]
      [EXPECT_FAIL]
      [CMD_ARGS <arg>...]
      [MPI_RANKS <ranks>...]
      [CMAKE_GUARD <condition>...]
      [COMMAND <cmd>...]
      [COMPILE_ONLY]
      [TIMEOUT <seconds>]
      [WORKING_DIRECTORY <dir>]
      [LABELS <labels...>]
    )

  ``NAME``
    Name of the test. If omitted, the name is deduced from the single source
    file or the given target.

  ``SOURCES``
    Source files used to build the test executable. You must provide either
    ``SOURCES`` or ``TARGET``.

  ``TARGET``
    Existing executable target to use for the test. You must provide either
    ``SOURCES`` or ``TARGET``.

  ``COMPILE_DEFINITIONS``
    Extra compile definitions to add when :cmake:command:`dune_add_test()`
    creates the executable itself.

  ``COMPILE_FLAGS``
    Extra non-definition compile flags to add when
    :cmake:command:`dune_add_test()` creates the executable itself.

  ``LINK_LIBRARIES``
    Extra libraries to link when :cmake:command:`dune_add_test()` creates the
    executable itself.

  ``EXPECT_COMPILE_FAIL``
    Expect the test target to fail during compilation.

  ``EXPECT_FAIL``
    Expect the test to compile but fail at run time.

  ``CMD_ARGS``
    Additional command line arguments passed to the test command.

  ``MPI_RANKS``
    Numbers of MPI ranks with which the test should run. One ctest test is
    created for each rank count. Values larger than
    :cmake:variable:`DUNE_MAX_TEST_CORES` are ignored. When this option is
    used, ``TIMEOUT`` must also be specified.

  ``CMAKE_GUARD``
    Conditions evaluated by CMake before adding the test. Use this instead of
    guarding the call with a plain ``if()`` when you want the test summary to
    report the case as skipped.

  ``COMMAND``
    Exact command line to execute for the test. If ``CMD_ARGS`` is also given,
    those arguments are appended to the command.

  ``COMPILE_ONLY``
    Compile the test during ``make build_tests`` but do not run it during
    ``make test``.

  ``TIMEOUT``
    Timeout in seconds. This overrides the default ctest timeout for the test.

  ``WORKING_DIRECTORY``
    Working directory used when executing the test. By default this is the
    build directory corresponding to the current source directory.

  ``LABELS``
    Labels attached to the test. They also control creation of
    ``build_<label>_tests`` targets. Use :cmake:command:`dune_declare_test_label()`
    when you want such targets to exist unconditionally.

  You may either create the executable yourself with
  :cmake:command:`add_executable()` and pass it through ``TARGET``, or let
  :cmake:command:`dune_add_test()` create the executable from ``SOURCES``.

.. cmake:variable:: DUNE_MAX_TEST_CORES

  Upper bound for the number of processors a single test may use. The default
  is 2 when MPI is found and 1 otherwise.

.. cmake:variable:: DUNE_BUILD_TESTS_ON_MAKE_ALL

  If enabled, build all tests during ``make all``. Otherwise tests are built
  through the ``build_tests`` target. This option does not apply to immutable
  targets such as imported or aliased targets.

.. cmake:variable:: PYTHON_TEST

  Marker used by :cmake:command:`dune_python_add_test()` to identify python
  tests. It disables the check for the existence of the target file.

#]=======================================================================]
include_guard(GLOBAL)

# enable the testing suite on the CMake side.
enable_testing()
include(CTest)

include(DunePolicy)
dune_define_policy(DP_TEST_ADD_ALL_FLAGS
  "OLD behavior: Automatically call add_dune_all_flags on all test targets. NEW behavior: flags must be set for each test target separately, e.g., using add_dune_pkg_flags, or in directory scope using dune_enable_all_packages.")

# Introduce a target that triggers the building of all tests
add_custom_target(build_tests)

function(dune_declare_test_label)
  cmake_parse_arguments(arg "" "" "LABELS" ${ARGN})

  if( (DEFINED arg_UNPARSED_ARGUMENTS) AND NOT ( arg_UNPARSED_ARGUMENTS STREQUAL "" ) )
    message(FATAL_ERROR "Unhandled extra arguments given to dune_declare_test_label(): "
      "<${arg_UNPARSED_ARGUMENTS}>")
  endif()

  foreach(label IN LISTS arg_LABELS)
    # Make sure the label is not empty, and does not contain any funny
    # characters, in particular regex characters
    if(NOT (label MATCHES "[-_0-9a-zA-Z]+"))
      message(FATAL_ERROR "Refusing to add label \"${label}\" since it is "
        "empty or contains funny characters (characters other than "
        "alphanumeric ones and \"-\" or \"_\"; the intent of this restriction "
        "is to make construction of the argument to \"ctest -L\" easier")
    endif()
    set(target "build_${label}_tests")
    if(NOT TARGET "${target}")
      add_custom_target("${target}")
    endif()
  endforeach()
endfunction(dune_declare_test_label)

# predefine "quick" test label so build_quick_tests can be built
# unconditionally
dune_declare_test_label(LABELS quick)

# Set the default on the variable DUNE_MAX_TEST_CORES
if(NOT DUNE_MAX_TEST_CORES)
  set(DUNE_MAX_TEST_CORES 2)
endif()

function(dune_add_test)
  set(OPTIONS EXPECT_COMPILE_FAIL EXPECT_FAIL SKIP_ON_77 COMPILE_ONLY PYTHON_TEST)
  set(SINGLEARGS NAME TARGET TIMEOUT WORKING_DIRECTORY)
  set(MULTIARGS SOURCES COMPILE_DEFINITIONS COMPILE_FLAGS LINK_LIBRARIES CMD_ARGS MPI_RANKS COMMAND CMAKE_GUARD LABELS)
  cmake_parse_arguments(ADDTEST "${OPTIONS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})

  # Check whether the parser produced any errors
  if(ADDTEST_UNPARSED_ARGUMENTS)
    message(WARNING "Unrecognized arguments ('${ADDTEST_UNPARSED_ARGUMENTS}') for dune_add_test!")
  endif()

  # Check input for validity and apply defaults
  if(NOT ADDTEST_SOURCES AND NOT ADDTEST_TARGET)
    message(FATAL_ERROR "You need to specify either the SOURCES or the TARGET option for dune_add_test!")
  endif()
  if(ADDTEST_SOURCES AND ADDTEST_TARGET)
    message(FATAL_ERROR "You cannot specify both SOURCES and TARGET for dune_add_test")
  endif()
  if(NOT ADDTEST_NAME)
    # try deducing the test name from the executable name
    if(ADDTEST_TARGET)
      set(ADDTEST_NAME ${ADDTEST_TARGET})
    endif()
    # try deducing the test name form the source name
    if(ADDTEST_SOURCES)
      # deducing a name is only possible with a single source argument
      list(LENGTH ADDTEST_SOURCES len)
      if(NOT len STREQUAL "1")
        message(FATAL_ERROR "Cannot deduce test name from multiple sources!")
      endif()
      # strip file extension
      get_filename_component(ADDTEST_NAME ${ADDTEST_SOURCES} NAME_WE)
    endif()
  endif()
  if(NOT ADDTEST_COMMAND)
    if(ADDTEST_TARGET)
      set(ADDTEST_COMMAND ${ADDTEST_TARGET})
    else()
      set(ADDTEST_COMMAND ${ADDTEST_NAME})
    endif()
  endif()
  if(ADDTEST_MPI_RANKS AND (NOT ADDTEST_TIMEOUT))
    message(FATAL_ERROR "dune_add_test: You need to specify the TIMEOUT parameter if using the MPI_RANKS parameter.")
  endif()
  if(NOT ADDTEST_MPI_RANKS)
    set(ADDTEST_MPI_RANKS 1)
  endif()
  if(NOT ADDTEST_TIMEOUT)
    set(ADDTEST_TIMEOUT 300)
  endif()
  if(NOT ADDTEST_WORKING_DIRECTORY)
    set(ADDTEST_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endif()
  foreach(num ${ADDTEST_MPI_RANKS})
    if(NOT "${num}" MATCHES "[1-9][0-9]*")
      message(FATAL_ERROR "${num} was given to the MPI_RANKS argument of dune_add_test, but it does not seem like a correct processor number")
    endif()
  endforeach()
  if(ADDTEST_SKIP_ON_77)
    message(WARNING "The SKIP_ON_77 option for dune_add_test is obsolete, it is now enabled by default.")
  endif()

  # Discard all parallel tests if MPI was not found
  if(NOT HAVE_MPI)
    set(DUNE_MAX_TEST_CORES 1)
  endif()

  # Find out whether this test should be a dummy
  set(SHOULD_SKIP_TEST FALSE)
  set(FAILED_CONDITION_PRINTING "")
  foreach(condition ${ADDTEST_CMAKE_GUARD})
    separate_arguments(condition)
    if(NOT (${condition}))
      set(SHOULD_SKIP_TEST TRUE)
      set(FAILED_CONDITION_PRINTING "${FAILED_CONDITION_PRINTING}std::cout << \"  ${condition}\" << std::endl;\n")
    endif()
  endforeach()

  # If we do nothing, switch the sources for a dummy source
  if(SHOULD_SKIP_TEST)
    dune_module_path(MODULE dune-common RESULT scriptdir SCRIPT_DIR)
    set(ADDTEST_TARGET)
    set(dummymain ${CMAKE_CURRENT_BINARY_DIR}/main77_${ADDTEST_NAME}.cc)
    configure_file(${scriptdir}/main77.cc.in ${dummymain})
    set(ADDTEST_SOURCES ${dummymain})
  endif()

  # add some default libraries to link against
  list(APPEND ADDTEST_LINK_LIBRARIES Dune::Common)
  list(REMOVE_DUPLICATES ADDTEST_LINK_LIBRARIES)

  # Add the executable if it is not already present
  if(ADDTEST_SOURCES)
    add_executable(${ADDTEST_NAME} ${ADDTEST_SOURCES})

    # add all flags to the target if corresponding policy is OLD and not explicitly disabled.
    dune_policy(GET DP_TEST_ADD_ALL_FLAGS _add_all_flags)
    if(_add_all_flags STREQUAL "OLD")
      add_dune_all_flags(${ADDTEST_NAME})
    endif()
    unset(_add_all_flags)

    # This is just a placeholder
    target_compile_definitions(${ADDTEST_NAME} PUBLIC ${ADDTEST_COMPILE_DEFINITIONS})
    target_compile_options(${ADDTEST_NAME} PUBLIC ${ADDTEST_COMPILE_FLAGS})
    target_link_libraries(${ADDTEST_NAME} PUBLIC ${ADDTEST_LINK_LIBRARIES})
    set(ADDTEST_TARGET ${ADDTEST_NAME})
  endif()

  # If target is mutable, make sure to exclude the target from all, even when it is user-provided
  get_target_property(aliased ${ADDTEST_TARGET} ALIASED_TARGET)
  get_target_property(imported ${ADDTEST_TARGET} IMPORTED)
  if ((NOT aliased) AND (NOT imported))
    if(DUNE_BUILD_TESTS_ON_MAKE_ALL AND (NOT ADDTEST_EXPECT_COMPILE_FAIL))
      set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 0)
    else()
      set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 1)
    endif()
  endif()

  # make sure each label exists and its name is acceptable
  list(APPEND ADDTEST_LABELS ${PROJECT_NAME})
  dune_declare_test_label(LABELS ${ADDTEST_LABELS})

  # Have build_tests and build_${label}_tests depend on the given target in
  # order to trigger the build correctly
  if(NOT ADDTEST_EXPECT_COMPILE_FAIL)
    add_dependencies(build_tests ${ADDTEST_TARGET})
    foreach(label IN LISTS ADDTEST_LABELS)
      add_dependencies(build_${label}_tests ${ADDTEST_TARGET})
    endforeach()
  endif()

  # Process the EXPECT_COMPILE_FAIL option
  if(ADDTEST_EXPECT_COMPILE_FAIL)
    set(ADDTEST_COMMAND "${CMAKE_COMMAND}")
    set(ADDTEST_CMD_ARGS --build . --target ${ADDTEST_TARGET} --config "$<CONFIGURATION>")
  endif()

  # Add one test for each specified processor number
  foreach(procnum ${ADDTEST_MPI_RANKS})
    if((NOT "${procnum}" GREATER "${DUNE_MAX_TEST_CORES}") AND (NOT ADDTEST_COMPILE_ONLY))
      set(ACTUAL_NAME ${ADDTEST_NAME})
      set(ACTUAL_CMD_ARGS ${ADDTEST_CMD_ARGS})
      if(TARGET "${ADDTEST_COMMAND}")
        # if the target name is specified as command, expand to full path using the TARGET_FILE generator expression
        set(ACTUAL_TESTCOMMAND "$<TARGET_FILE:${ADDTEST_COMMAND}>")
      else()
        set(ACTUAL_TESTCOMMAND "${ADDTEST_COMMAND}")
      endif()

      # modify test name and command for parallel tests
      if(NOT ${procnum} STREQUAL "1")
        set(ACTUAL_NAME "${ACTUAL_NAME}-mpi-${procnum}")
        set(ACTUAL_CMD_ARGS ${MPIEXEC_PREFLAGS} ${MPIEXEC_NUMPROC_FLAG} ${procnum} "${ACTUAL_TESTCOMMAND}" ${MPIEXEC_POSTFLAGS} ${ACTUAL_CMD_ARGS})
        set(ACTUAL_TESTCOMMAND "${MPIEXEC}")
      endif()

      # if this is a skipped test because a guard was false, overwrite the command
      if(SHOULD_SKIP_TEST)
        set(ACTUAL_TESTCOMMAND "$<TARGET_FILE:${ADDTEST_TARGET}>")
        set(ACTUAL_CMD_ARGS)
      endif()

      # Now add the actual test
      add_test(NAME ${ACTUAL_NAME}
               COMMAND "${ACTUAL_TESTCOMMAND}" ${ACTUAL_CMD_ARGS}
               WORKING_DIRECTORY "${ADDTEST_WORKING_DIRECTORY}"
              )

      # Make the test depend on the existence of the target to trigger "Not Run" response
      if(NOT ADDTEST_EXPECT_COMPILE_FAIL AND NOT ADDTEST_PYTHON_TEST)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES REQUIRED_FILES $<TARGET_FILE:${ADDTEST_TARGET}>)
      endif()
      # Define the number of processors (ctest will coordinate this with the -j option)
      set_tests_properties(${ACTUAL_NAME} PROPERTIES PROCESSORS ${procnum})
      # Apply the timeout (which was defaulted to 5 minutes if not specified)
      set_tests_properties(${ACTUAL_NAME} PROPERTIES TIMEOUT ${ADDTEST_TIMEOUT})
      # Process the EXPECT_FAIL option
      if(ADDTEST_EXPECT_COMPILE_FAIL OR ADDTEST_EXPECT_FAIL)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES WILL_FAIL true)
      endif()
      # When using ninja, we must call the build command from ${PROJECT_BINARY_DIR}
      if(ADDTEST_EXPECT_COMPILE_FAIL)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
      endif()
      # Skip the test if the return code is 77!
      set_tests_properties(${ACTUAL_NAME} PROPERTIES SKIP_RETURN_CODE 77)
      # Set the labels on the test
      set_tests_properties(${ACTUAL_NAME} PROPERTIES LABELS "${ADDTEST_LABELS}")
    endif()
  endforeach()
endfunction()
