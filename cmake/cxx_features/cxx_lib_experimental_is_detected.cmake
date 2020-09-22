include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature std::experimental::detected_t")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  check_cxx_symbol_exists(
    "std::move<std::experimental::detected_t<std::decay_t,int>>"
    "utility;experimental/type_traits"
    DUNE_HAVE_CXX_LIB_EXPERIMENTAL_IS_DETECTED)
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_LIB_EXPERIMENTAL_IS_DETECTED)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_EXPERIMENTAL_IS_DETECTED)
    message(STATUS "Looking for optional compiler feature std::experimental::detected_t - found")
  else ()
    message(STATUS "Looking for optional compiler feature std::experimental::detected_t - not found")
  endif ()
endfunction (add_feature)
