MACRO(test_dep)
  execute_process(COMMAND ${CMAKE_COMMAND} -D RELPATH=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/FindFiles.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _res ERROR_VARIABLE _dirs)

  foreach(_dir ${_dirs})
    string(REGEX REPLACE "([^ \t\n]+)[ \n\t]*$" "\\1" _dir ${_dir})
    set_property(DIRECTORY ${_dir} PROPERTY TEST_INCLUDE_FILE ${CMAKE_BINARY_DIR}/${_dir}/BuildTests.cmake)
  endforeach(_dir ${_dirs})
ENDMACRO(test_dep)

MACRO(get_directory_test_target _target _dir)
  string(REPLACE "${CMAKE_BINARY_DIR}" "" _relative_dir "${_dir}")
  string(REPLACE "/" "_" ${_target} "${_relative_dir}")
ENDMACRO(get_directory_test_target _target _dir)

MACRO(add_directory_test_target _target)
  get_directory_test_target(${_target} "${CMAKE_CURRENT_BINARY_DIR}")
  add_custom_target(${${_target}})
  configure_file(${DUNE_COMMON_SCRIPT_DIR}/BuildTests.cmake.in BuildTests.cmake @ONLY)
ENDMACRO(add_directory_test_target)
