# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
FindQuadMath
------------

Find the GCC Quad-Precision library

This module checks if the used compiler has built-in support for QuadMath
by compiling a small source file.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``QuadMath::QuadMath``
  Library to link against if QuadMath should be used.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``QuadMath_FOUND``
  True if the Quad-Precision library was found.

#]=======================================================================]

# Add a feature summary for this package
include(FeatureSummary)
set_package_properties(QuadMath PROPERTIES
  DESCRIPTION "GCC Quad-Precision Math Library"
  URL "https://gcc.gnu.org/onlinedocs/libquadmath"
)

# Check if QuadMath support is built into the compiler
include(CheckCXXSourceCompiles)
include(CMakePushCheckState)
cmake_push_check_state()
set(CMAKE_REQUIRED_LIBRARIES quadmath)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  set(CMAKE_REQUIRED_FLAGS "-fext-numeric-literals")
endif()
check_cxx_source_compiles("
#include <quadmath.h>

int main ()
{
  __float128 r = 1.0q;
  r = strtoflt128(\"1.2345678\", NULL);
  return 0;
}" QuadMath_COMPILES)
cmake_pop_check_state()  # Reset CMAKE_REQUIRED_XXX variables

if(QuadMath_COMPILES)
  # Use additional variable for better report message
  set(QuadMath_VAR "(Supported by compiler)")
endif()

# Report that package was found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QuadMath
  DEFAULT_MSG
  QuadMath_VAR QuadMath_COMPILES
)

# add imported target for quadmath
if(QuadMath_FOUND AND NOT TARGET QuadMath::QuadMath)
  # Compiler supports QuadMath: Add appropriate linker flag
  add_library(QuadMath::QuadMath INTERFACE IMPORTED)
  target_link_libraries(QuadMath::QuadMath INTERFACE quadmath)

  target_compile_definitions(QuadMath::QuadMath INTERFACE
    _GLIBCXX_USE_FLOAT128
  )
  target_compile_options(QuadMath::QuadMath INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:-fext-numeric-literals>
  )
endif()
