include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature std::experimental::make_array")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  check_cxx_symbol_exists(
    "std::experimental::make_array<int,int>"
    "experimental/array"
    DUNE_HAVE_CXX_LIB_EXPERIMENTAL_MAKE_ARRAY)
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_LIB_EXPERIMENTAL_MAKE_ARRAY)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_EXPERIMENTAL_MAKE_ARRAY)
    message(STATUS "Looking for optional compiler feature std::experimental::make_array - found")
  else ()
    message(STATUS "Looking for optional compiler feature std::experimental::make_array - not found")
  endif ()
endfunction (add_feature)
