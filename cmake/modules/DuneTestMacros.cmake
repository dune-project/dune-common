# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Module that provides tools for testing the Dune way.
#
# Note that "the Dune way" of doing this has changed after
# the 2.4 release. See the build system documentation for details.
#
# .. cmake_function:: dune_declare_test_label
#
#    .. cmake_brief::
#
#       Declare labels for :ref:`dune_add_test`.
#
#    .. cmake_param:: LABELS
#       :multi:
#
#       The names of labels to declare.  Label names must be nonempty and
#       consist only of alphanumeric characters plus :code:`-` and :code:`_`
#       to make sure it is easy to construct regular expressions from them for
#       :code:`ctest -L ${label_regex}`.
#
#    Labels need to be declared to ensure that the target
#    :code:`build_${label}_tests` exists.  They will normally be declared
#    on-demand by :ref:`dune_add_test`.  But sometimes it is useful to be able to
#    run :code:`make build_${label}_tests` whether or not any tests with that
#    label exists in a module.  For these cases :ref:`dune_declare_test_label` can
#    be called explicitly.
#
#    The label :code:`quick` is always predeclared.
#
# .. cmake_function:: dune_add_test
#
#    .. cmake_brief::
#
#       Adds a test to the Dune testing suite!
#
#    .. cmake_param:: NAME
#       :single:
#
#       The name of the test that should be added. If an executable
#       is also added (by specifying SOURCES), the executable is also
#       named accordingly. If omitted, the name will be deduced from
#       the (single) sources parameter or from the given target. Note
#       that this requires you to take care, that you only use a target
#       or source file for but one such test.
#
#    .. cmake_param:: SOURCES
#       :multi:
#
#       The source files that this test depends on. These are the
#       sources that will be passed to :ref:`add_executable`.
#
#       You *must* specify either :code:`SOURCES` or :code:`TARGET`.
#
#    .. cmake_param:: TARGET
#       :single:
#
#       An executable target which should be used for the test. Use
#       this option over the :code:`SOURCES` parameter if you want to
#       reuse already added targets.
#
#       You *must* specify either :code:`SOURCES` or :code:`TARGET`.
#
#    .. cmake_param:: COMPILE_DEFINITIONS
#       :multi:
#       :argname: def
#
#       A set of compile definitions to add to the target.
#       Only definitions beyond the application of :ref:`add_dune_all_flags`
#       have to be stated.
#       This is only used, if :code:`dune_add_test` adds the executable itself.
#
#    .. cmake_param:: COMPILE_FLAGS
#       :multi:
#       :argname: flag
#
#       A set of non-definition compile flags to add to the target.
#       Only flags beyond the application of :ref:`add_dune_all_flags`
#       have to be stated.
#       This is only used, if :code:`dune_add_test` adds the executable itself.
#
#    .. cmake_param:: LINK_LIBRARIES
#       :multi:
#       :argname: lib
#
#       A list of libraries to link the target to.
#       Only libraries beyond the application of :ref:`add_dune_all_flags`
#       have to be stated.
#       This is only used, if :code:`dune_add_test` adds the executable itself.
#
#    .. cmake_param:: EXPECT_COMPILE_FAIL
#       :option:
#
#       If given, the test is expected to not compile successfully!
#
#    .. cmake_param:: EXPECT_FAIL
#       :option:
#
#       If given, this test is expected to compile, but fail to run.
#
#    .. cmake_param:: CMD_ARGS
#       :multi:
#       :argname: arg
#
#       Command line arguments that should be passed to this test.
#
#    .. cmake_param:: MPI_RANKS
#       :multi:
#       :argname: ranks
#
#       The numbers of cores that this test should be executed with.
#       Note that one test (in the ctest sense) is created for each number
#       given here. Any number exceeding the user-specified processor maximum
#       :ref:`DUNE_MAX_TEST_CORES` will be ignored. Tests with a
#       processor number :code:`n` higher than one will have the suffix
#       :code:`-mpi-n` appended to their name. You need to specify the
#       TIMEOUT option when specifying the MPI_RANKS option.
#
#    .. cmake_param:: CMAKE_GUARD
#       :multi:
#       :argname: condition
#
#       A number of conditions that CMake should evaluate before adding this
#       test. If one of the conditions fails, the test should be shown
#       as skipped in the test summary. Use this feature instead of guarding
#       the call to :code:`dune_add_test` with an :code:`if` clause.
#
#       The passed condition can be a complex expression like
#       `( A OR B ) AND ( C OR D )`. Mind the spaces around the parentheses.
#
#       Example: Write CMAKE_GUARD dune-foo_FOUND if you want your test to only
#       build and run when the dune-foo module is present.
#
#    .. cmake_param:: COMMAND
#       :multi:
#       :argname: cmd
#
#       You may specify the COMMAND option to give the exact command line to be
#       executed when running the test. This defaults to the name of the executable
#       added by dune_add_test for this test or the name of the executable of the given TARGET.
#       Note that if you specify both CMD_ARGS
#       and COMMAND, the given CMD_ARGS will be put behind your COMMAND. If you use
#       this in combination with the MPI_RANKS parameter, the call to mpi will still be
#       wrapped around the given commands.
#
#    .. cmake_param:: COMPILE_ONLY
#       :option:
#
#       Set if the given test should only be compiled during :code:`make build_tests`,
#       but not run during :code:`make test`. This is useful if you compile the same
#       executable twice, but with different compile flags, where you want to assure that
#       it compiles with both sets of flags, but you already know they will produce the
#       same result.
#
#    .. cmake_param:: TIMEOUT
#       :single:
#
#       If set, the test will time out after the given number of seconds. This supersedes
#       any timeout setting in ctest (see `cmake --help-property TIMEOUT`). If you
#       specify the MPI_RANKS option, you need to specify a TIMEOUT.
#
#    .. cmake_param:: WORKING_DIRECTORY
#       :single:
#
#       Set the WORKING_DIRECTORY test property to specify the working directory in which to execute the test.
#       If not specified the test will be run with the current working directory set to the build directory corresponding to the current source directory.
#
#    .. cmake_param:: LABELS
#       :multi:
#
#       A list of labels to add to the test.  This has two effects: it sets
#       the LABELS property on the test so :code:`ctest -L ${label_regex}` can
#       be used to run all tests with certain labels.  It also adds any
#       targets created as dependencies to a custom target, so you can build
#       all tests with a particular label by doing :code:`make
#       build_${label}_tests` without having to build all the other tests as
#       well.
#
#       The :code:`build_${label}_tests` targets are created on-demand the
#       first time a test with that label is added.  In some situations it can
#       depend on the values of cmake cache variables whether a test is added,
#       and then it can happen that the :code:`build_${target}_tests` target
#       exists only sometimes.  If your workflow relies on the existence of
#       these targets, even if building them just returns successfully without
#       doing anything, you can ensure they exist by calling
#       :ref:`dune_declare_test_label` unconditionally.  The label
#       :code:`quick` is always predeclared in this way.
#
#       The label names must be non-empty, and must only contain alphanumeric
#       characters other than :code:`-` or :code:`_`.  This restriction is in
#       place to make it easy to construct regular expressions from the label
#       names for :code:`ctest -L ${label_regex}`.
#
#    This function defines the Dune way of adding a test to the testing suite.
#    You may either add the executable yourself through :ref:`add_executable`
#    and pass it to the :code:`TARGET` option, or you may rely on :ref:`dune_add_test`
#    to do so.
#
# .. cmake_variable:: DUNE_REENABLE_ADD_TEST
#
#    You may set this variable to True either through your opts file or in your module
#    (before the call to :code:`include(DuneMacros)`) to suppress the error that is thrown if
#    :code:`add_test` is used. You should only do that if you have proper reason to do so.
#
# .. cmake_variable:: DUNE_MAX_TEST_CORES
#
#    You may set this variable to give an upperbound to the number of processors, that
#    a single test may use. Defaults to 2, when MPI is found and to 1 otherwise.
#
# .. cmake_variable:: DUNE_BUILD_TESTS_ON_MAKE_ALL
#
#    You may set this variable through your opts file or on a per module level (in the toplevel
#    :code:`CMakeLists.txt` before :code:`include(DuneMacros)`) to have the Dune build system
#    build all tests during `make all`. Note, that this may take quite some time for some modules.
#    If not in use, you have to build tests through the target :code:`build_tests`.
#
# .. cmake_variable:: PYTHON_TEST
#
#    This flag specifies a python test and is set by the dune_python_add_test command. It disables the check on the existence of the target file.
#
include_guard(GLOBAL)

# enable the testing suite on the CMake side.
enable_testing()
include(CTest)

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

  # Add the executable if it is not already present
  if(ADDTEST_SOURCES)
    add_executable(${ADDTEST_NAME} ${ADDTEST_SOURCES})
    # add all flags to the target!
    add_dune_all_flags(${ADDTEST_NAME})
    # This is just a placeholder
    target_compile_definitions(${ADDTEST_NAME} PUBLIC ${ADDTEST_COMPILE_DEFINITIONS})
    target_compile_options(${ADDTEST_NAME} PUBLIC ${ADDTEST_COMPILE_FLAGS})
    target_link_libraries(${ADDTEST_NAME} PUBLIC ${ADDTEST_LINK_LIBRARIES})
    set(ADDTEST_TARGET ${ADDTEST_NAME})
  endif()

  # Make sure to exclude the target from all, even when it is user-provided
  if(DUNE_BUILD_TESTS_ON_MAKE_ALL AND (NOT ADDTEST_EXPECT_COMPILE_FAIL))
    set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 0)
  else()
    set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 1)
  endif()

  # make sure each label exists and its name is acceptable
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
      _add_test(NAME ${ACTUAL_NAME}
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

macro(add_directory_test_target)
  message(FATAL_ERROR "The function add_directory_test_target has been removed alongside all testing magic in dune-common. Check dune_add_test for the new way!")
endmacro()

macro(add_test)
  if(NOT DUNE_REENABLE_ADD_TEST)
    message(SEND_ERROR "Please use dune_add_test instead of add_test! If you need add_test in a downstream project, set the variable DUNE_REENABLE_ADD_TEST to True in that project to suppress this error.")
  else()
    _add_test(${ARGN})
  endif()
endmacro()
