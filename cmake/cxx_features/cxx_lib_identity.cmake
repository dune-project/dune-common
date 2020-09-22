include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler feature std::identity")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  foreach (cxx_std_flag "-std=c++20" "-std=c++2a")
    set(CMAKE_REQUIRED_FLAGS ${cxx_std_flag})
    check_cxx_symbol_exists(
      "std::move<std::identity>"
      "utility;functional"
      DUNE_HAVE_CXX_LIB_IDENTITY)
    if (DUNE_HAVE_CXX_LIB_IDENTITY)
      break()
    else ()
      unset(DUNE_HAVE_CXX_LIB_IDENTITY CACHE)
    endif()
  endforeach(cxx_std_flag)
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_LIB_IDENTITY)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_LIB_IDENTITY)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_20)
    message(STATUS "Looking for optional compiler feature std::identity - found")
  else ()
    message(STATUS "Looking for optional compiler feature std::identity - not found")
  endif ()
endfunction (add_feature)
