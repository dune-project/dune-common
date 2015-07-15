# Module provides macros that allow for on demand test building.
# In DUNE tests are not built by make all but by make test.
#
# Provides the following macros:
#
# test_dep()
#
# Finds all directories called test and creates a dependency for
# testing that calls builds the tests for this directory.
#
#
# add_directory_test_target(_target)
#
# Creates a custom target for building
# the tests in the current directory.
#
# The target name will be the path of the
# current directory relative to ${CMAKE_BINARY_DIR}
# with all slashes replaced by underlines.
# E.g. for dune/istl/test the target will be dune_istl_test.
#
macro(test_dep)
  dune_common_script_dir(SCRIPT_DIR)
  get_filename_component(RELPATH "${CMAKE_SOURCE_DIR}"  REALPATH)
  execute_process(COMMAND ${CMAKE_COMMAND} -D RELPATH=${RELPATH} -P ${SCRIPT_DIR}/FindFiles.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _res ERROR_VARIABLE _dirs)

  foreach(_dir ${_dirs})
    string(REGEX REPLACE "([^ \t\n]+)[ \n\t]*$" "\\1" _dir ${_dir})
    set_property(DIRECTORY ${_dir} PROPERTY TEST_INCLUDE_FILE ${CMAKE_BINARY_DIR}/${_dir}/BuildTests.cmake)
  endforeach(_dir ${_dirs})
endmacro(test_dep)

macro(get_directory_test_target _target _dir)
  string(REPLACE "${CMAKE_BINARY_DIR}" "" _relative_dir "${_dir}")
  string(REPLACE "/" "_" ${_target} "${_relative_dir}")
endmacro(get_directory_test_target _target _dir)

#
# - Create a custom target for building
# the tests in the current directory.
#
# The target name will be the path of the
# current directory relative to ${CMAKE_BINARY_DIR}
# with all slashes replaced by underlines.
# E.g. for dune/istl/test the target will be dune_istl_test.
#
macro(add_directory_test_target _target)
  if(DUNE_TEST_MAGIC)
    get_directory_test_target(${_target} "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_target(${${_target}})
    dune_common_script_dir(SCRIPT_DIR)
    configure_file(${SCRIPT_DIR}/BuildTests.cmake.in BuildTests.cmake @ONLY)
  endif()
endmacro(add_directory_test_target)
