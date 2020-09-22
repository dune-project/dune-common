include_guard(GLOBAL)

function (add_feature TARGET SCOPE)
  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  message(STATUS "Looking for optional compiler attribute deprecated")

  cmake_push_check_state()
  set(CMAKE_REQUIRED_QUIET TRUE)
  set(CMAKE_REQUIRED_FLAGS "-std=c++14")
  check_cxx_source_compiles("
     class bar
     {
       bar() __attribute__((deprecated));
     };

     class peng { } __attribute__((deprecated));

     template <class T>
     class t_bar
     {
       t_bar() __attribute__((deprecated));
     };

     template <class T>
     class t_peng {
       t_peng() {};
     } __attribute__((deprecated));

     void foo() __attribute__((deprecated));

     void foo() {}

     int main()
     {
       return 0;
     };
  "  DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED
  )
  cmake_pop_check_state()

  if (DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED)
    target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED)
    target_compile_features(${TARGET} ${SCOPE} cxx_std_14)
    message(STATUS "Looking for optional compiler attribute deprecated - found")
  else ()
    message(STATUS "Looking for optional compiler attribute deprecated - not found")
  endif ()

  # additionally check whether a string literal can be passed as message to the deprecated attribute
  if (DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED)
    message(STATUS "Looking for optional compiler attribute deprecated(\"message\")")

    cmake_push_check_state()
    set(CMAKE_REQUIRED_QUIET TRUE)
    set(CMAKE_REQUIRED_FLAGS "-std=c++14")
    check_cxx_source_compiles("
      class bar
      {
        bar() __attribute__((deprecated(\"message\")));
      };

      class peng { } __attribute__((deprecated(\"message\")));

      template <class T>
      class t_bar
      {
        t_bar() __attribute__((deprecated(\"message\")));
      };

      template <class T>
      class t_peng {
        t_peng() {};
      } __attribute__((deprecated(\"message\")));

      void foo() __attribute__((deprecated(\"message\")));

      void foo() {}

      int main()
      {
        return 0;
      };
    "  DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED_MSG
    )
    cmake_pop_check_state()

    if (DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED_MSG)
      target_compile_definitions(${TARGET} ${SCOPE} DUNE_HAVE_CXX_ATTRIBUTE_DEPRECATED_MSG)
      target_compile_features(${TARGET} ${SCOPE} cxx_std_14)
      message(STATUS "Looking for optional compiler attribute deprecated(\"message\") - found")
    else ()
      message(STATUS "Looking for optional compiler attribute deprecated(\"message\") - not found")
    endif ()
  endif()
endfunction (add_feature)
