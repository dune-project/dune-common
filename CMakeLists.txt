cmake_minimum_required(VERSION 3.13)
project(dune-common VERSION 2.8.0 LANGUAGES CXX)

# make sure our own modules are found
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# set the script dir for the macros.
set(DUNE_COMMON_SCRIPT_DIR ${PROJECT_SOURCE_DIR}/cmake/scripts)

#include the dune macros
include(DuneMacros)

# start a dune project, by creating the library dune-common and Dune::dune-common
dune_project()

# set minimal required c++ standard version
target_compile_features(dune-common PUBLIC cxx_std_17)

# check for optional compile rfeatures and set cpp flags
target_optional_compile_features(dune-common PUBLIC
  cxx_identity
  cxx_experimental_is_detected
  cxx_experimental_make_array)

# link found packages to dune-common
if(LAPACK_FOUND AND BLAS_FOUND)
  target_link_libraries(dune-common PUBLIC BLAS::BLAS LAPACK::LAPACK)
  set(HAVE_BLAS TRUE)
  set(HAVE_LAPACK TRUE)
endif()

add_dune_gmp_flags(dune-common)
add_dune_quadmath_flags(dune-common)
add_dune_mpi_flags(dune-common)
add_dune_tbb_flags(dune-common)

# add subdirectories to execute CMakeLists.txt there
add_subdirectory(bin)
add_subdirectory(cmake)
add_subdirectory(doc)
add_subdirectory(dune)
add_subdirectory(lib)
add_subdirectory(share)

# if Python bindings are enabled, include necessary sub directories.
if(DUNE_ENABLE_PYTHONBINDINGS)
  add_subdirectory(python)
  dune_python_install_package(PATH "python")
endif()

# finalize the dune project, e.g. generating config.h etc.
finalize_dune_project()
