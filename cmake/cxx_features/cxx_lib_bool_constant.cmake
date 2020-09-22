include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature std::bool_constant")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  set(CMAKE_REQUIRED_FLAGS "-std=c++17")
  check_cxx_symbol_exists(
    "std::move<std::bool_constant<true>>"
    "utility;type_traits"
    DUNE_HAVE_CXX_LIB_BOOL_CONSTANT)
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_LIB_BOOL_CONSTANT)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_BOOL_CONSTANT)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_17)
    message(STATUS "Looking for optional compiler feature std::bool_constant - found")
  else ()
    message(STATUS "Looking for optional compiler feature std::bool_constant - not found")
  endif ()

  # fallback implementation using `std::experimental::bool_constant`
  if (NOT DUNE_HAVE_CXX_LIB_BOOL_CONSTANT)
    message(STATUS "Looking for optional compiler feature std::experimental::bool_constant")

    cmake_push_check_state()
    set(CMAKE_REQUIRED_QUIET TRUE)
    check_cxx_symbol_exists(
      "std::move<std::experimental::bool_constant<true>>"
      "utility;experimental/type_traits"
      DUNE_HAVE_CXX_LIB_EXPERIMENTAL_BOOL_CONSTANT)
    cmake_pop_check_state()

    if (DUNE_HAVE_CXX_LIB_EXPERIMENTAL_BOOL_CONSTANT)
      target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_EXPERIMENTAL_BOOL_CONSTANT)
      message(STATUS "Looking for optional compiler feature std::experimental::bool_constant - found")
    else ()
      message(STATUS "Looking for optional compiler feature std::experimental::bool_constant - not found")
    endif ()
  endif()
endfunction (add_feature)
