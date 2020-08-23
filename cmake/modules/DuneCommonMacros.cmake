include(DuneStreams)
dune_set_minimal_debug_level()

find_package(GMP)
include(AddGMPFlags)

# find BLAS and LAPACK
find_package(LAPACK)
if (LAPACK_FOUND)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_LIBRARIES ${LAPACK_LIBRARIES})
  check_function_exists("dsyev_" LAPACK_NEEDS_UNDERLINE)
  cmake_pop_check_state()
endif ()

# find the MPI library
find_package(MPI)
include(AddMPIFlags)

# find library for quad-precision arithmetics
find_package(QuadMath)
include(AddQuadMathFlags)

# find program for image manipulation
find_package(Inkscape)
include(UseInkscape)

# find the MProtect header files
find_package(MProtect)

find_package(TBB OPTIONAL_COMPONENTS cpf allocator)
include(AddTBBFlags)

# try to find the Vc library
set(MINIMUM_VC_VERSION)
if((CMAKE_CXX_COMPILER_ID STREQUAL Clang) AND
    (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7))
  message("Raising minimum acceptable Vc version to 1.4.1 due to use of Clang 7 (or later), see https://gitlab.dune-project.org/core/dune-common/issues/132")
  set(MINIMUM_VC_VERSION 1.4.1)
endif()
find_package(Vc ${MINIMUM_VC_VERSION} NO_MODULE)
include(AddVcFlags)

# Run the python extension of the Dune cmake build system
include(DunePythonCommonMacros)
