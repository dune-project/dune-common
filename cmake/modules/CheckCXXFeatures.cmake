# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
CheckCXXFeatures
----------------

This module checks for several CXX compiler features and introduces a macro to check if
given C++ source compiles and links into an executable.

.. command:: dune_check_cxx_source_compiles

  .. code-block:: cmake

    dune_check_cxx_source_compiles(<sources> <var>)

  The macro checks that the given sources compiles and links into an executable. Thus,
  it must at least provide a valid ``main()`` function. The macro is inspired by the
  cmake function ``check_cxx_source_compiles()`` but does not provide all of its
  functionality. It is also based on the :command:`try_compile` command but without any
  additional flags set, like include directories or link options. The only fixed flag that
  is passed to :command:`try_compile` is ``CXX_STANDARD`` with value 17. This can only be
  influenced by setting the global variable ``CMAKE_CXX_STANDARD``.

#]=======================================================================]

macro(dune_check_cxx_source_compiles SOURCE VAR)
  message(STATUS "Performing Test ${VAR}")
  file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src_${VAR}.cxx" "${SOURCE}\n")

  set(_cxx_std 17)
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
