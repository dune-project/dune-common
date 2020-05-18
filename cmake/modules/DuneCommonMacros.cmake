include(CMakePushCheckState)
include(CheckFunctionExists)

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

# find the MProtect header files
find_package(MProtect)
