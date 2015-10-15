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
#
#       A set of compile definitions to add to the target.
#       Only definitions beyond the application of :ref:`add_dune_all_flags`
#       have to be stated.
#       This is only used, if :code:`dune_add_test` adds the executable itself.
#
#    .. cmake_param:: COMPILE_FLAGS
#       :multi:
#
#       A set of non-definition compile flags to add to the target.
#       Only flags beyond the application of :ref:`add_dune_all_flags`
#       have to be stated.
#       This is only used, if :code:`dune_add_test` adds the executable itself.
#
#    .. cmake_param:: LINK_LIBRARIES
#       :multi:
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
#    This function defines the Dune way of adding a test to the testing suite.
#    You may either add the executable yourself through :ref:`add_executable`
#    and pass it to the :code:`TARGET` option, or you may rely on :ref:`dune_add_test`
#    to do so.
#
# .. cmake_variable:: REENABLE_ADD_TEST
#
#    You may set this variable to True either through your opts file or in your module
#    (before the call to :code:`include(DuneMacros)`) to suppress the error that is thrown if
#    :code:`add_test` is used. You should only do that if you have proper reason to do so.
#

# enable the testing suite on the CMake side.
enable_testing()
include(CTest)

# Introduce a target that triggers the building of all tests
add_custom_target(build_tests)

function(dune_add_test)
  include(CMakeParseArguments)
  set(OPTIONS EXPECT_COMPILE_FAIL EXPECT_FAIL SKIP_ON_77)
  set(SINGLEARGS NAME TARGET)
  set(MULTIARGS SOURCES COMPILE_DEFINITIONS COMPILE_FLAGS LINK_LIBRARIES)
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
  set_property(TARGET ${ADDTEST_TARGET} PROPERTY EXCLUDE_FROM_ALL 1)

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

  # Now add the actual test
  _add_test(NAME ${ADDTEST_NAME}
            COMMAND ${TESTCOMMAND}
           )

  # Process the EXPECT_FAIL option
  if(ADDTEST_EXPECT_COMPILE_FAIL OR ADDTEST_EXPECT_FAIL)
    set_tests_properties(${ADDTEST_NAME} PROPERTIES WILL_FAIL true)
  endif()
  # Process the SKIP_ON_77 option
  if(ADDTEST_SKIP_ON_77)
    set_tests_properties(${ADDTEST_NAME} PROPERTIES SKIP_RETURN_CODE 77)
  endif()
endfunction()

macro(add_directory_test_target)
  message(FATAL_ERROR "The function add_directory_test_target has been removed alongside all testing magic in dune-common. Check dune_add_test for the new way!")
endmacro()

macro(add_test)
  if(NOT REENABLE_ADD_TEST)
    message(SEND_ERROR "Please dune_add_test instead of add_test! If you need add_test in a downstream project, set the variable REENABLE_ADD_TEST to True in that project to suppress this error.")
  else()
    _add_test(${ARGN})
  endif()
endmacro()
