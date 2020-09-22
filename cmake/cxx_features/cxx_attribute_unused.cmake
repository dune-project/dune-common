include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler attribute unused")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  set(CMAKE_REQUIRED_FLAGS "-std=c++17")
  check_cxx_source_compiles("
  int main()
  {
    int __attribute__((unused)) foo;
    return 0;
  };
  "  DUNE_HAVE_CXX_ATTRIBUTE_UNUSED
  )
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_ATTRIBUTE_UNUSED)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_ATTRIBUTE_UNUSED)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_17)
    message(STATUS "Looking for optional compiler attribute unused - found")
  else ()
    message(STATUS "Looking for optional compiler attribute unused - not found")
  endif ()
endfunction (add_feature)
