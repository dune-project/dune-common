#[=======================================================================[.rst:
DuneAddTest
-----------

Function that adds a test and registers it in the custom build_tests target

.. code-block:: cmake

  # signature 1
  dune_add_test(<target> [...])

  # signature 2
  dune_add_test(<name> <target> [...])

  # signature 3
  dune_add_test(NAME <name>
                TARGET <target>
                [EXPECT_COMPILE_FAIL]
                [EXPECT_FAIL]
                [CMD_ARGS <args>...]
                [MPI_RANKS <ranks>...]
                [CMAKE_GUARD <bool_variable>...]
                [COMMAND <command>]
                [COMPILE_ONLY]
                [TIMEOUT <timeout>]
                [LABELS <label>...] )

The function :ref:`dune_add_test` has three signatures. In the most simple one,
signature 1, just provide a test executable target. The name for the test will be the
same as the name of the target. In signature 2 additionally the name can be specified.
For the full signature 3 several arguments are allowed to control the test configuration.

The following options are allowed in :ref:`dune_add_test`:

  .. cmake_param:: NAME
      :single:

      The name of the test that should be added.

  .. cmake_param:: TARGET
      :single:

      An executable target which should be used for the test.

  .. cmake_param:: EXPECT_COMPILE_FAIL
      :option:

      If given, the test is expected to not compile successfully!

  .. cmake_param:: EXPECT_FAIL
      :option:

      If given, this test is expected to compile, but fail to run.

  .. cmake_param:: CMD_ARGS
      :multi:
      :argname: arg

      Command line arguments that should be passed to this test.

  .. cmake_param:: MPI_RANKS
      :multi:
      :argname: ranks

      The numbers of cores that this test should be executed with.
      Note that one test (in the ctest sense) is created for each number
      given here. Any number exceeding the user-specified processor maximum
      :ref:`DUNE_MAX_TEST_CORES` will be ignored. Tests with a
      processor number :code:`n` higher than one will have the suffix
      :code:`-mpi-n` appended to their name. You need to specify the
      TIMEOUT option when specifying the MPI_RANKS option.

  .. cmake_param:: CMAKE_GUARD
      :multi:
      :argname: condition

      A number of conditions that CMake should evaluate before adding this
      test. If one of the conditions fails, the test should be shown
      as skipped in the test summary. Use this feature instead of guarding
      the call to :code:`dune_add_test` with an :code:`if` clause.

      The passed condition can be a complex expression like
      `( A OR B ) AND ( C OR D )`. Mind the spaces around the parentheses.

      Example: Write CMAKE_GUARD dune-foo_FOUND if you want your test to only
      build and run when the dune-foo module is present.

  .. cmake_param:: COMMAND
      :multi:
      :argname: cmd

      You may specify the COMMAND option to give the exact command line to be
      executed when running the test. This defaults to the name of the executable
      added by dune_add_test for this test. Note that if you specify both CMD_ARGS
      and COMMAND, the given CMD_ARGS will be put behind your COMMAND. If you use
      this in combination with the MPI_RANKS parameter, the call to mpi will still be
      wrapped around the given commands.

  .. cmake_param:: COMPILE_ONLY
      :option:

      Set if the given test should only be compiled during :code:`make build_tests`,
      but not run during :code:`make test`. This is useful if you compile the same
      executable twice, but with different compile flags, where you want to assure that
      it compiles with both sets of flags, but you already know they will produce the
      same result.

  .. cmake_param:: TIMEOUT
      :single:

      If set, the test will time out after the given number of seconds. This supersedes
      any timeout setting in ctest (see `cmake --help-property TIMEOUT`). If you
      specify the MPI_RANKS option, you need to specify a TIMEOUT.

  .. cmake_param:: LABELS
      :multi:

      A list of labels to add to the test.  This has two effects: it sets
      the LABELS property on the test so :code:`ctest -L ${label_regex}` can
      be used to run all tests with certain labels.  It also adds any
      targets created as dependencies to a custom target, so you can build
      all tests with a particular label by doing :code:`make
      build_${label}_tests` without having to build all the other tests as
      well.

      The :code:`build_${label}_tests` targets are created on-demand the
      first time a test with that label is added.  In some situations it can
      depend on the values of cmake cache variables whether a test is added,
      and then it can happen that the :code:`build_${target}_tests` target
      exists only sometimes.  If your workflow relies on the existance of
      these targets, even if building them just returns successfully without
      doing anything, you can ensure they exist by calling
      :ref:`dune_declare_test_label` unconditionally.  The label
      :code:`quick` is always predeclared in this way.

      The label names must be non-empty, and must only contain alphanumeric
      characters other than :code:`-` or :code:`_`.  This restriction is in
      place to make it easy to construct regular expressions from the label
      names for :code:`ctest -L ${label_regex}`.

  This function defines the Dune way of adding a test to the testing suite.
  You may either add the executable yourself through :ref:`add_executable`
  and pass it to the :code:`TARGET` option, or you may rely on :ref:`dune_add_test`
  to do so.

.. cmake_variable:: DUNE_MAX_TEST_CORES

  You may set this variable to give an upperbound to the number of processors, that
  a single test may use. Defaults to 2, when MPI is found and to 1 otherwise.

.. cmake_variable:: DUNE_BUILD_TESTS_ON_MAKE_ALL

  You may set this variable through your opts file or on a per module level (in the toplevel
  :code:`CMakeLists.txt` before :code:`include(DuneMacros)`) to have the Dune build system
  build all tests during `make all`. Note, that this may take quite some time for some modules.
  If not in use, you have to build tests through the target :code:`build_tests`.

#]=======================================================================]

include_guard(GLOBAL)

function (dune_add_test)
  set(OPTIONS EXPECT_COMPILE_FAIL EXPECT_FAIL COMPILE_ONLY)
  set(SINGLEARGS NAME TARGET TIMEOUT)
  set(MULTIARGS CMD_ARGS MPI_RANKS COMMAND CMAKE_GUARD LABELS)
  cmake_parse_arguments(ADDTEST "${OPTIONS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})

  # try deducing the test name from the executable name
  if (ADDTEST_TARGET AND NOT ADDTEST_NAME)
    set(ADDTEST_NAME ${ADDTEST_TARGET})
  endif ()

  # first (two) argument(s) define (name and) target
  if (NOT ADDTEST_TARGET AND NOT ADDTEST_NAME)
    if (ARGC GREATER_EQUAL 2 AND TARGET ${ARGV1})
      set(ADDTEST_NAME ${ARGV0})
      set(ADDTEST_TARGET ${ARGV1})
    elseif (ARGC GREATER_EQUAL 1 AND TARGET ${ARGV0})
      set(ADDTEST_NAME ${ARGV0})
      set(ADDTEST_TARGET ${ARGV0})
    endif ()
  endif ()

  if (NOT ADDTEST_TARGET OR NOT TARGET ${ADDTEST_TARGET})
    message(FATAL_ERROR "dune_add_test: You need to specify the TARGET option for dune_add_test!")
  endif ()
  if (NOT ADDTEST_COMMAND)
    set(ADDTEST_COMMAND ${ADDTEST_NAME})
  endif ()

  if (ADDTEST_MPI_RANKS AND (NOT ADDTEST_TIMEOUT))
    message(FATAL_ERROR "dune_add_test: You need to specify the TIMEOUT parameter if using the MPI_RANKS parameter.")
  endif ()
  if (ADDTEST_MPI_RANKS)
    find_package(MPI QUIET)
    if (NOT MPI_FOUND)
      set(ADDTEST_MPI_RANKS 1)
    endif ()
  endif ()
  if (NOT ADDTEST_MPI_RANKS)
    set(ADDTEST_MPI_RANKS 1)
  endif ()
  if (NOT ADDTEST_TIMEOUT)
    set(ADDTEST_TIMEOUT 300)
  endif ()

  # Check whether the parser produced any errors
  if (NOT ADDTEST_TARGET AND NOT ADD_TEST_NAME AND ADDTEST_UNPARSED_ARGUMENTS)
    message(WARNING "dune_add_test: Unrecognized arguments ('${ADDTEST_UNPARSED_ARGUMENTS}')!")
  endif ()

  foreach (num ${ADDTEST_MPI_RANKS})
    if (NOT "${num}" MATCHES "[1-9][0-9]*")
      message(FATAL_ERROR "dune_add_test: ${num} was given to the MPI_RANKS arugment,
                           but it does not seem like a correct processor number")
    endif ()
  endforeach (num)

  # Discard all parallel tests if MPI was not found
  if (NOT MPI_FOUND)
    set(DUNE_MAX_TEST_CORES 1)
  endif ()

  # Find out whether this test should be skipped
  foreach (condition ${ADDTEST_CMAKE_GUARD})
    separate_arguments(condition)
    if (NOT (${condition}))
      return()
    endif ()
  endforeach (condition)

  # Make sure to exclude the target from all, even when it is user-provided
  if (DUNE_BUILD_TESTS_ON_MAKE_ALL AND (NOT ADDTEST_EXPECT_COMPILE_FAIL))
    set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 0)
  else ()
    set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 1)
  endif ()

  # make sure each label exists and its name is acceptable
  include(DuneDeclareTestLabel)
  dune_declare_test_label(LABELS ${ADDTEST_LABELS})

  # Have build_tests and build_${label}_tests depend on the given target in
  # order to trigger the build correctly
  if (NOT ADDTEST_EXPECT_COMPILE_FAIL)
    add_dependencies(build_tests ${ADDTEST_TARGET})
    foreach (label IN LISTS ADDTEST_LABELS)
      add_dependencies(build_${label}_tests ${ADDTEST_TARGET})
    endforeach (label)
  endif ()

  # Process the EXPECT_COMPILE_FAIL option
  if (ADDTEST_EXPECT_COMPILE_FAIL)
    set(ADDTEST_COMMAND "${CMAKE_COMMAND}")
    set(ADDTEST_CMD_ARGS --build . --target ${ADDTEST_TARGET} --config "$<CONFIGURATION>")
  endif ()

  # Add one test for each specified processor number
  foreach (np ${ADDTEST_MPI_RANKS})
    if ((NOT "${np}" GREATER "${DUNE_MAX_TEST_CORES}") AND (NOT ADDTEST_COMPILE_ONLY))
      set(ACTUAL_NAME ${ADDTEST_NAME})
      set(ACTUAL_CMD_ARGS ${ADDTEST_CMD_ARGS})
      if (TARGET "${ADDTEST_COMMAND}")
        # if the target name is specified as command, expand to full path using the TARGET_FILE generator expression
        set(ACTUAL_TESTCOMMAND "$<TARGET_FILE:${ADDTEST_COMMAND}>")
      else ()
        set(ACTUAL_TESTCOMMAND "${ADDTEST_COMMAND}")
      endif ()

      # modify test name and command for parallel tests
      if (NOT ${np} STREQUAL "1")
        set(ACTUAL_NAME "${ACTUAL_NAME}-mpi-${np}")
        set(ACTUAL_CMD_ARGS ${MPIEXEC_PREFLAGS} ${MPIEXEC_NUMPROC_FLAG} ${np} "${ACTUAL_TESTCOMMAND}" ${MPIEXEC_POSTFLAGS} ${ACTUAL_CMD_ARGS})
        set(ACTUAL_TESTCOMMAND "${MPIEXEC}")
      endif ()

      # Now add the actual test
      add_test(NAME ${ACTUAL_NAME}
               COMMAND "${ACTUAL_TESTCOMMAND}" ${ACTUAL_CMD_ARGS})

      # Make the test depend on the existence of the target to trigger "Not Run" response
      if (NOT ADDTEST_EXPECT_COMPILE_FAIL)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES REQUIRED_FILES $<TARGET_FILE:${ADDTEST_TARGET}>)
      endif ()
      # Define the number of processors (ctest will coordinate this with the -j option)
      set_tests_properties(${ACTUAL_NAME} PROPERTIES PROCESSORS ${np})
      # Apply the timeout (which was defaulted to 5 minutes if not specified)
      set_tests_properties(${ACTUAL_NAME} PROPERTIES TIMEOUT ${ADDTEST_TIMEOUT})
      # Process the EXPECT_FAIL option
      if (ADDTEST_EXPECT_COMPILE_FAIL OR ADDTEST_EXPECT_FAIL)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES WILL_FAIL true)
      endif ()
      # When using ninja, we must call the build command from ${PROJECT_BINARY_DIR}
      if (ADDTEST_EXPECT_COMPILE_FAIL)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
      endif ()
      # Skip the test if the return code is 77!
      set_tests_properties(${ACTUAL_NAME} PROPERTIES SKIP_RETURN_CODE 77)
      # Set the labels on the test
      set_tests_properties(${ACTUAL_NAME} PROPERTIES LABELS "${ADDTEST_LABELS}")
    endif ()
  endforeach (np)
endfunction (dune_add_test)
