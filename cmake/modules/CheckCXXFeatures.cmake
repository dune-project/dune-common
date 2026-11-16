# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
CheckCXXFeatures
----------------

This module checks for several CXX compiler features and introduces a macro to check if
given C++ source compiles and links into an executable.

.. cmake:command:: dune_check_cxx_source_compiles

  .. code-block:: cmake

    dune_check_cxx_source_compiles(<sources> <var>)

  The macro checks that the given sources compiles and links into an executable. Thus,
  it must at least provide a valid ``main()`` function. The macro is inspired by the
  CMake module command
  `check_cxx_source_compiles() <https://cmake.org/cmake/help/latest/module/CheckCXXSourceCompiles.html#command:check_cxx_source_compiles>`_
  but does not provide all of its functionality. It is also based on the
  :dune:cmake-command:`try_compile` command but without any additional flags
  set, like include directories or link options. The only fixed flag that is
  passed to :dune:cmake-command:`try_compile` is
  `CXX_STANDARD <https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html>`_
  with value 17. This can only be influenced by setting the global variable
  `CMAKE_CXX_STANDARD <https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_STANDARD.html>`_.

#]=======================================================================]

macro(dune_check_cxx_source_compiles SOURCE VAR)
  message(STATUS "Performing Test ${VAR}")
  file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src_${VAR}.cxx" "${SOURCE}\n")

  set(_cxx_std 20)
  if(DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD GREATER_EQUAL 17)
    set(_cxx_std ${CMAKE_CXX_STANDARD})
  endif()

  try_compile(${VAR} ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src_${VAR}.cxx
    CXX_STANDARD ${_cxx_std}
    OUTPUT_VARIABLE OUTPUT)
  unset(_cxx_std)

  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    "Performing C++ SOURCE FILE Test ${VAR} succeeded with the following output:\n"
    "${OUTPUT}\n"
    "Source file was:\n${SOURCE}\n")
  if(${VAR})
    message(STATUS "Performing Test ${VAR} - Success")
    set(${VAR} 1 CACHE INTERNAL "Test ${VAR}")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Performing C++ SOURCE FILE Test ${VAR} succeeded with the following output:\n"
      "${OUTPUT}\n"
      "Source file was:\n${SOURCE}\n")
  else()
    message(STATUS "Performing Test ${VAR} - Failed")
    set(${VAR} "" CACHE INTERNAL "Test ${VAR}")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Performing C++ SOURCE FILE Test ${VAR} failed with the following output:\n"
      "${OUTPUT}\n"
      "Source file was:\n${SOURCE}\n")
  endif()
endmacro(dune_check_cxx_source_compiles)


function(dune_require_cxx_standard)
  message(DEPRECATION "No need to call dune_require_cxx_standard() any more. If you want to "
    "enforce a higher standard than requested in the core modules, simply use the cmake "
    "command target_compile_features(<target> <PRIVATE|PUBLIC|INTERFACE> cxx_std_[17|20|...]) "
    "to set the requirement on your target directly. Note, it is recommended to compile all "
    "modules with the same c++ standard flags. This can be achieved by setting the global "
    "cmake variable CMAKE_CXX_STANDARD to the requested maximal standard for all dune modules.")
endfunction()

if(NOT CMAKE_CXX_STANDARD AND CXX_MAX_STANDARD)
  message(DEPRECATION "The cmake variable CXX_MAX_STANDARD is deprecated. Use CMAKE_CXX_STANDARD instead!")
  set(CMAKE_CXX_STANDARD ${CXX_MAX_STANDARD})
elseif(CMAKE_CXX_STANDARD AND CXX_MAX_STANDARD)
  message(DEPRECATION "The cmake variable CXX_MAX_STANDARD is deprecated. The variable CMAKE_CXX_STANDARD "
    "takes precedence over CXX_MAX_STANDARD.")
endif()


# perform tests

# Check for `std::is_detected<...>`
dune_check_cxx_source_compiles("
  #include <experimental/type_traits>
  int main() { std::experimental::detected_t<std::decay_t,int>{}; }
" DUNE_HAVE_CXX_EXPERIMENTAL_IS_DETECTED)

# Check whether compiler supports lambdas in unevaluated contexts
string(REPLACE ";" "\;" cxx_lambda_test "using F = decltype([](){}); int main() {return 0;}")
dune_check_cxx_source_compiles(${cxx_lambda_test} DUNE_HAVE_CXX_UNEVALUATED_CONTEXT_LAMBDA)

# Check for `std::identity<...>`
dune_check_cxx_source_compiles("
  #include <functional>
  int main() { std::identity{}; }
" DUNE_HAVE_CXX_STD_IDENTITY)

# Check for `std::experimental::simd` (Parallelism TS v2) with the parts
# needed by the SIMD abstraction.
#
# Clang up to at least 17 (and probably 18) crashes or does not terminate
# (nondeterministically, depending on the memory layout) while compiling
# `operator/` of `std::experimental::simd` from older libstdc++ headers
# (libstdc++ 11 and 12.2; fixed in 12.3/12.4, see issue #439).  A small
# program does not trigger this, so we cannot detect it by compiling, and
# instead reject the combination by the version macros: the macro
# `_GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA` (defined, empty for Clang, by
# <experimental/simd>) marks the fixed generation of the libstdc++ headers.
# Compilers other than Clang, and Clang 19 and newer, are not affected.
dune_check_cxx_source_compiles("
  #include <experimental/simd>
  #if defined(__clang__) && __clang_major__ < 19 && !defined(_GLIBCXX_SIMD_ALWAYS_INLINE_LAMBDA)
  #error Clang < 19 crashes on std::experimental::simd of libstdc++ < 12.3
  #endif
  namespace stdx = std::experimental;
  int main() {
    stdx::native_simd<double> v(1.0);
    stdx::fixed_size_simd<float, 3> f([](auto i) { return float(i); });
    stdx::rebind_simd_t<int, decltype(v)> i(1);
    stdx::where(v > 0.0, v) = 2.0;
    v[0] = 3.0;
    return (stdx::hmax(v) > stdx::hmin(f) && stdx::any_of(i == 1)) ? 0 : 1;
  }
" DUNE_HAVE_CXX_EXPERIMENTAL_SIMD)
