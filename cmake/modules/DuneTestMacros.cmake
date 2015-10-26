# Module that provides tools for testing the Dune way.
#
# Note that "the Dune way" of doing this has changed after
# the 2.4 release. See the buildsystem documentation for details.
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
#       the (single) sources parameter or from the given target. Note,
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
#    .. cmake_param:: SKIP_ON_77
#
#       The test will be marked as skipped in ctest if it returned 77.
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
#       The numbers of cores that this tests should be executed with.
#       Note, that one test (in the ctest sense) is created for each number
#       given here. Any number exceeding the user-specified processor maximum
#       :ref:`DUNE_MAX_TEST_CORES` will be ignored. Tests with a
#       processor number :code:`n` higher than one will have the suffix
#       :code:`-mpi-n` appended to their name.
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

# enable the testing suite on the CMake side.
enable_testing()
include(CTest)

# Introduce a target that triggers the building of all tests
add_custom_target(build_tests)

# Set the default on the variable DUNE_MAX_TEST_CORES
if(NOT DUNE_MAX_TEST_CORES)
  set(DUNE_MAX_TEST_CORES 2)
endif()

function(dune_add_test)
  include(CMakeParseArguments)
  set(OPTIONS EXPECT_COMPILE_FAIL EXPECT_FAIL SKIP_ON_77)
  set(SINGLEARGS NAME TARGET)
  set(MULTIARGS SOURCES COMPILE_DEFINITIONS COMPILE_FLAGS LINK_LIBRARIES CMD_ARGS MPI_RANKS)
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
  if(NOT ADDTEST_MPI_RANKS)
    set(ADDTEST_MPI_RANKS 1)
  endif()
  foreach(num ${ADDTEST_MPI_RANKS})
    if(NOT "${num}" MATCHES "[1-9][0-9]*")
      message(FATAL_ERROR "${num} was given to the MPI_RANKS arugment of dune_add_test, but it does not seem like a correct processor number")
    endif()
  endforeach()

  # Discard all parallel tests if MPI was not found
  if(NOT MPI_FOUND)
    set(DUNE_MAX_TEST_CORES 1)
  endif()

  # Add the executable if it is not already present
  if(ADDTEST_SOURCES)
    add_executable(${ADDTEST_NAME} ${ADDTEST_SOURCES})
    # add all flags to the target!
    add_dune_all_flags(${ADDTEST_NAME})
    # This is just a placeholder
    target_compile_definitions(${ADDTEST_NAME} PUBLIC ${ADDTEST_COMPILE_DEFINITIONS})
    target_compile_options(${ADDTEST_NAME} PUBLIC ${ADDTEST_COMPILE_FLAGS})
    target_link_libraries(${ADDTEST_NAME} ${ADDTEST_LINK_LIBRARIES})
    set(ADDTEST_TARGET ${ADDTEST_NAME})
  endif()

  # Make sure to exclude the target from all, even when it is user-provided
  if(DUNE_BUILD_TESTS_ON_MAKE_ALL)
    set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 0)
  else()
    set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 1)
  endif()

  # Have the given target depend on build_tests in order to trigger the build correctly
  if(NOT ADDTEST_EXPECT_COMPILE_FAIL)
    add_dependencies(build_tests ${ADDTEST_TARGET})
  endif()

  # By default, the target itself is sufficient as a testing command.
  set(TESTCOMMAND ${ADDTEST_TARGET})

  # Process the EXPECT_COMPILE_FAIL option
  if(ADDTEST_EXPECT_COMPILE_FAIL)
    set(TESTCOMMAND ${CMAKE_COMMAND} --build . --target ${TESTCOMMAND} --config $<CONFIGURATION>)
  endif()

  # Add one test for each specified processor number
  foreach(procnum ${ADDTEST_MPI_RANKS})
    if(NOT "${procnum}" GREATER "${DUNE_MAX_TEST_CORES}")
      if(NOT ${procnum} STREQUAL "1")
        set(ACTUAL_TESTCOMMAND ${MPIEXEC} ${MPIEXEC_PREFLAGS} ${MPIEXEC_NUMPROC_FLAG} ${procnum} ${TESTCOMMAND} ${MPIEXEC_POSTFLAGS})
        set(ACTUAL_NAME "${ADDTEST_NAME}-mpi-${procnum}")
      else()
        set(ACTUAL_TESTCOMMAND ${TESTCOMMAND})
        set(ACTUAL_NAME ${ADDTEST_NAME})
      endif()

      # Now add the actual test
      _add_test(NAME ${ACTUAL_NAME}
                COMMAND ${ACTUAL_TESTCOMMAND} ${ADDTEST_CMD_ARGS}
               )

      # Define the number of processors (ctest will coordinate this with the -j option)
      set_tests_properties(${ACTUAL_NAME} PROPERTIES PROCESSORS ${procnum})
      # Process the EXPECT_FAIL option
      if(ADDTEST_EXPECT_COMPILE_FAIL OR ADDTEST_EXPECT_FAIL)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES WILL_FAIL true)
      endif()
      # Process the SKIP_ON_77 option
      if(ADDTEST_SKIP_ON_77)
        set_tests_properties(${ACTUAL_NAME} PROPERTIES SKIP_RETURN_CODE 77)
      endif()
    endif()
  endforeach()
endfunction()

macro(add_directory_test_target)
  message(FATAL_ERROR "The function add_directory_test_target has been removed alongside all testing magic in dune-common. Check dune_add_test for the new way!")
endmacro()

macro(add_test)
  if(NOT DUNE_REENABLE_ADD_TEST)
    message(SEND_ERROR "Please dune_add_test instead of add_test! If you need add_test in a downstream project, set the variable DUNE_REENABLE_ADD_TEST to True in that project to suppress this error.")
  else()
    _add_test(${ARGN})
  endif()
endmacro()
