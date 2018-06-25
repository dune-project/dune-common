# .. cmake_module::
#
#    Find the GCC Quad-Precision library
#
#    Sets the following variables:
#
#    :code:`QUADMATH_FOUND`
#       True if the Quad-Precision library was found.
#
#


# search for the header quadmath.h
include(CheckIncludeFile)
check_include_file(quadmath.h QUADMATH_HEADER)

include(CheckCSourceCompiles)
include(CMakePushCheckState)

cmake_push_check_state() # Save variables
set(CMAKE_REQUIRED_LIBRARIES quadmath)
check_c_source_compiles("
#include <quadmath.h>

int main ()
{
  __float128 r = 1.0q;
  r = strtoflt128(\"1.2345678\", NULL);
  return 0;
}" QUADMATH_COMPILES)
cmake_pop_check_state()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "QuadMath"
  DEFAULT_MSG
  QUADMATH_HEADER
  QUADMATH_COMPILES
)

# text for feature summary
set_package_properties("QuadMath" PROPERTIES
  DESCRIPTION "GCC Quad-Precision library")

# set HAVE_QUADMATH for config.h
set(HAVE_QUADMATH ${QUADMATH_FOUND})

# register all QuadMath related flags
if(HAVE_QUADMATH)
  dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_QUADMATH=1" "_GLIBCXX_USE_FLOAT128=1"
                              COMPILE_OPTIONS "-fext-numeric-literals"
                              LIBRARIES "quadmath")
endif()
