#[=======================================================================[.rst:
FindQuadMath
------------

Find the GCC Quad-Precision library

This module checks if the used compiler has built-in support for QuadMath
by compiling a small source file. If this is not the case, the module
searches for a QuadMath installation.

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

Cache Variables
^^^^^^^^^^^^^^^

You may set the following variables to modify the behaviour of
this module. Those variables are not considered, if the compiler
already supports QuadMath.

``QuadMath_INCLUDE_DIR``
  The directory containing ``quadmath.h``.
``QuadMath_LIB``
  The path to the quadmath library.

#]=======================================================================]

# Add a feature summary for this package
include(FeatureSummary)
set_package_properties(QuadMath PROPERTIES
  DESCRIPTION "GCC Quad-Precision Math Library"
  URL "https://gcc.gnu.org/onlinedocs/libquadmath"
)

# Check if QuadMath support is built into the compiler
include(CheckIncludeFile)
check_include_file(quadmath.h QuadMath_HEADER)

include(CheckCSourceCompiles)
include(CMakePushCheckState)
cmake_push_check_state()
set(CMAKE_REQUIRED_LIBRARIES quadmath)
check_c_source_compiles("
#include <quadmath.h>

int main ()
{
  __float128 r = 1.0q;
  r = strtoflt128(\"1.2345678\", NULL);
  return 0;
}" QuadMath_COMPILES)
cmake_pop_check_state()  # Reset CMAKE_REQUIRED_XXX variables

include(FindPackageHandleStandardArgs)
if(QuadMath_HEADER AND QuadMath_COMPILES)
  # Use additional variable for better report message
  set(QuadMath_VAR "(Supported by compiler)")
  # Report that package was found
  find_package_handle_standard_args(QuadMath
    DEFAULT_MSG
    QuadMath_VAR QuadMath_HEADER QuadMath_COMPILES
  )
else()
  # Try to find an installation of QuadMath
  find_path(QuadMath_INCLUDE_DIR quadmath.h)
  find_library(QuadMath_LIB quadmath)
  mark_as_advanced(QuadMath_INCLUDE_DIR QuadMath_LIB)

  # Report if package was found
  find_package_handle_standard_args(QuadMath
    DEFAULT_MSG
    QuadMath_LIB QuadMath_INCLUDE_DIR
  )
endif()

# add imported target for quadmath
if(QuadMath_FOUND AND NOT TARGET QuadMath::QuadMath)
  if(QuadMath_LIB)
    # Library was found: Set imported location
    add_library(QuadMath::QuadMath UNKNOWN IMPORTED)
    set_target_properties(QuadMath::QuadMath PROPERTIES
      IMPORTED_LOCATION ${QuadMath_LIB}
      INTERFACE_INCLUDE_DIRECTORIES ${QuadMath_INCLUDE_DIR}
    )
  else()
    # Compiler supports QuadMath: Add appropriate linker flag
    add_library(QuadMath::QuadMath INTERFACE IMPORTED)
    target_link_libraries(QuadMath::QuadMath INTERFACE quadmath)
  endif()

  target_compile_definitions(QuadMath::QuadMath INTERFACE
    _GLIBCXX_USE_FLOAT128
  )
  target_compile_options(QuadMath::QuadMath INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:-fext-numeric-literals>
  )
endif()

# set HAVE_QUADMATH for config.h
set(HAVE_QUADMATH ${QUADMATH_FOUND})

# register the QuadMath imported target
if(HAVE_QUADMATH)
  dune_register_package_flags(
    COMPILE_DEFINITIONS "ENABLE_QUADMATH=1"
    LIBRARIES "QuadMath::QuadMath"
  )
endif()
