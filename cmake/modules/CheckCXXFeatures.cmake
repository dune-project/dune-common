# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# .. cmake_module::
#
#    Module that checks for supported C++20, C++17 and non-standard features.
#
#    The behaviour of this module can be modified by the following variable:
#
#    :ref:`DISABLE_CXX_VERSION_CHECK`
#       Disable checking for std=c++20 (c++23, ...)
#
#    This module internally sets the following variables, which are then
#    exported into the config.h of the current dune module.
#
# .. cmake_variable:: DISABLE_CXX_VERSION_CHECK
#
#    You may set this variable to TRUE to disable checking for
#    std=c++11 (c++14, c++1y). For more details, check :ref:`CheckCXXFeatures`.
#


include(CMakePushCheckState)
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckCXXSymbolExists)

# C++ standard versions that this test knows about
set(CXX_VERSIONS 20 17)


# Compile tests for the different standard revisions; these test both the compiler
# and the associated library to avoid problems like using a C++20 user-installed
# compiler together with a non C++20-compliant stdlib from the system compiler.

# we need to escape semicolons in the tests to be able to stick them into a list
string(REPLACE ";" "\;" cxx_20_test
  "
  #include <type_traits>

  // `if constexpr` is a C++20 compiler feature
  template<typename T>
  void f()
  { if constexpr (T::anything) {} }

  int main() {
    // std::is_bounded_array_v is a C++20 library feature
    return std::is_bounded_array_v<int[2]>;
  }
  ")

string(REPLACE ";" "\;" cxx_17_test
  "
  #include <type_traits>

  // nested namespaces are a C++17 compiler feature
  namespace A::B {
    using T = int;
  }

  int main() {
    // std::void_t is a C++17 library feature
    return not std::is_same<void,std::void_t<A::B::T> >{};
  }
  ")

# build a list out of the pre-escaped tests
set(CXX_VERSIONS_TEST "${cxx_20_test}" "${cxx_17_test}")

# these are appended to "-std=c++" and tried in this order
# note the escaped semicolons; that's necessary to create a nested list
set(CXX_VERSIONS_FLAGS "20\;2a" "17\;1z")

# by default, we enable C++17 for now, but not C++20
# The user can override this choice by explicitly setting this variable
set(CXX_MAX_STANDARD 17
    CACHE STRING
    "highest version of the C++ standard to enable. This version is also used if the version check is disabled")


function(dune_require_cxx_standard)
  cmake_parse_arguments("" "" "MODULE;VERSION" "" ${ARGN})

  if(_UNPARSED_ARGUMENTS)
    message(WARNING "Unknown arguments in call to dune_require_cxx_standard(${ARGN})")
  endif()

  if(${_VERSION} GREATER ${CXX_MAX_SUPPORTED_STANDARD})

    if(NOT _MODULE)
      set(_MODULE "This module")
    endif()

    if(${_VERSION} GREATER ${CXX_MAX_STANDARD})
      message(FATAL_ERROR "\
${_MODULE} requires compiler support for C++${_VERSION}, but the build system is currently \
set up to not allow newer language standards than C++${CXX_MAX_STANDARD}. Try setting the \
CMake variable CXX_MAX_STANDARD to at least ${_VERSION}."
        )
    else()
      if(${CXX_MAX_SUPPORTED_STANDARD} EQUAL 17)
        set(CXX_STD_NAME 17)
      else()
        set(CXX_STD_NAME ${CXX_MAX_SUPPORTED_STANDARD})
      endif()
      message(FATAL_ERROR "${_MODULE} requires support for C++${_VERSION}, but your compiler failed our compatibility test."
        )
    endif()
  endif()
endfunction()


# try to enable all of the C++ standards that we know about, in descending order
if(NOT DISABLE_CXX_VERSION_CHECK)

  foreach(version ${CXX_VERSIONS})

    # skip versions that are newer than allowed
    if(NOT(version GREATER CXX_MAX_STANDARD))

      list(FIND CXX_VERSIONS ${version} version_index)
      list(GET CXX_VERSIONS_FLAGS ${version_index} version_flags)

      # First try whether the compiler accepts one of the command line flags for this standard
      foreach(flag ${version_flags})

        set(cxx_std_flag_works "cxx_std_flag_${flag}")
        check_cxx_compiler_flag("-std=c++${flag}" ${cxx_std_flag_works})

        if(${cxx_std_flag_works})
          set(cxx_std_flag "-std=c++${flag}")
          break()
        endif()

      endforeach()

      # and if it did, run the compile test
      if(cxx_std_flag)

        list(GET CXX_VERSIONS_TEST ${version_index} version_test)
        set(test_compiler_output "compiler_supports_cxx${version}")

        cmake_push_check_state()
        set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${cxx_std_flag}")
        check_cxx_source_compiles("${version_test}" ${test_compiler_output})
        cmake_pop_check_state()

        if(${test_compiler_output})
          set(CXX_MAX_SUPPORTED_STANDARD ${version})
          set(CMAKE_CXX_FLAGS "${cxx_std_flag} ${CMAKE_CXX_FLAGS}")
          break()
        else()
          # Wipe the variable, as this version of the standard doesn't seem to work
          unset(cxx_std_flag)
        endif()

      endif()
    endif()
  endforeach()

  if(NOT DEFINED CXX_MAX_SUPPORTED_STANDARD)
    # Let's just assume every compiler at least claims C++03 compliance by now
    message(WARNING "\
Unable to determine C++ standard support for your compiler, falling back to C++17. \
If you know that your compiler supports a newer version of the standard, please set the CMake \
variable DISABLE_CXX_VERSION_CHECK to true and the CMake variable CXX_MAX_SUPPORTED_STANDARD \
to the highest version of the standard supported by your compiler (e.g. 20). If your compiler \
needs custom flags to switch to that standard version, you have to manually add them to \
CMAKE_CXX_FLAGS."
      )
    set(CXX_MAX_SUPPORTED_STANDARD 17)
  endif()
else()
  # We did not check version but need to set maximum supported
  # version for some checks. Therefore we set it to CXX_MAX_STANDARD.
  set(CXX_MAX_SUPPORTED_STANDARD ${CXX_MAX_STANDARD})
endif()

# make sure we have at least C++17
dune_require_cxx_standard(MODULE "DUNE" VERSION 17)

# ******************************************************************************
#
# Checks for standard library features
#
# While there are __cpp_lib_* feature test macros for all of these, those are
# unfortunately unreliable, as libc++ does not have feature test macros yet.
#
# In order to keep the tests short, they use check_cxx_symbol_exists(). That
# function can only test for macros and linkable symbols, however, so we wrap
# tested types into a call to std::move(). That should be safe, as std::move()
# does not require a complete type.
#
# ******************************************************************************

check_cxx_symbol_exists(
  "std::experimental::make_array<int,int>"
  "experimental/array"
  DUNE_HAVE_CXX_EXPERIMENTAL_MAKE_ARRAY
  )

check_cxx_symbol_exists(
  "std::move<std::experimental::detected_t<std::decay_t,int>>"
  "utility;experimental/type_traits"
  DUNE_HAVE_CXX_EXPERIMENTAL_IS_DETECTED
  )

check_cxx_symbol_exists(
  "std::identity"
  "functional"
  DUNE_HAVE_CXX_STD_IDENTITY
  )

string(REPLACE ";" "\;" cxx_lambda_test "using F = decltype([](){}); int main() {return 0;}")
check_cxx_source_compiles(${cxx_lambda_test} DUNE_HAVE_CXX_UNEVALUATED_CONTEXT_LAMBDA)
