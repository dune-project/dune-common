include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature std::identity")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  set(CMAKE_REQUIRED_FLAGS "-std=c++20")
  check_cxx_symbol_exists(
    "std::move<std::identity>"
    "utility;functional"
    DUNE_HAVE_CXX_IDENTITY)
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_IDENTITY)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_IDENTITY)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_20)
    message(STATUS "Looking for optional compiler feature std::identity - found")
  else ()
    message(STATUS "Looking for optional compiler feature std::identity - not found")
  endif ()
endfunction (add_feature)
