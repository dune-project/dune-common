include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature std::apply")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  set(CMAKE_REQUIRED_FLAGS "-std=c++17")
  check_cxx_symbol_exists(
    "std::apply<std::negate<int>,std::tuple<int>>"
    "functional;tuple"
    DUNE_HAVE_CXX_LIB_APPLY)
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_LIB_APPLY)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_APPLY)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_17)
    message(STATUS "Looking for optional compiler feature std::apply - found")
  else ()
    message(STATUS "Looking for optional compiler feature std::apply - not found")
  endif ()

  # fallback implementation using `std::experimental::apply`
  if (NOT DUNE_HAVE_CXX_LIB_APPLY)
    message(STATUS "Looking for optional compiler feature std::experimental::apply")

    cmake_push_check_state()
    set(CMAKE_REQUIRED_QUIET TRUE)
    check_cxx_symbol_exists(
      "std::experimental::apply<std::negate<int>,std::tuple<int>>"
      "functional;experimental/tuple"
      DUNE_HAVE_CXX_LIB_EXPERIMENTAL_APPLY)
    cmake_pop_check_state()

    if (DUNE_HAVE_CXX_LIB_EXPERIMENTAL_APPLY)
      target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_EXPERIMENTAL_APPLY)
      message(STATUS "Looking for optional compiler feature std::experimental::apply - found")
    else ()
      message(STATUS "Looking for optional compiler feature std::experimental::apply - not found")
    endif ()
  endif()
endfunction (add_feature)
