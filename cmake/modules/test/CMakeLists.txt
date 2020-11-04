set(DUNE_REENABLE_ADD_TEST 1)

set(build_generator_args --build-generator ${CMAKE_GENERATOR})
if(CMAKE_GENERATOR_PLATFORM)
  list(APPEND build_generator_args --build-generator-platform ${CMAKE_GENERATOR_PLATFORM})
endif()
if(CMAKE_GENERATOR_TOOLSET)
  list(APPEND build_generator_args --build-generator-toolset ${CMAKE_GENERATOR_TOOLSET})
endif()

macro(add_cmake_test _name _dir)
  add_test(NAME ${_name}
    COMMAND "${CMAKE_CTEST_COMMAND}"
      --build-and-test "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}"
                       "${CMAKE_CURRENT_BINARY_DIR}/${_dir}"
      ${build_generator_args}
      --build-options
        -DCMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/cmake/modules
      --test-command "${CMAKE_CTEST_COMMAND}")
endmacro(add_cmake_test)

add_cmake_test(cmake-metistest metis)
add_cmake_test(cmake-parmetistest parmetis)

set(DUNE_REENABLE_ADD_TEST 0)