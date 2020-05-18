cmake_minimum_required(VERSION 3.13)
project(dune-common VERSION 2.8.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
include(DuneMacros)
include(DuneCommonMacros)

# create the dune-common library
dune_add_library(dune-common)

# set minimal required c++ standard version
target_compile_features(dune-common PUBLIC cxx_std_17)

# check for optional compile rfeatures and set cpp flags
dune_target_optional_compile_features(dune-common PUBLIC
  cxx_identity
  cxx_experimental_is_detected
  cxx_experimental_make_array)

if (LAPACK_FOUND AND BLAS_FOUND)
  target_link_libraries(dune-common PUBLIC BLAS::BLAS LAPACK::LAPACK)
  target_compile_definitions(dune-common PUBLIC HAVE_BLAS HAVE_LAPACK
    $<$<BOOL:${LAPACK_NEEDS_UNDERLINE}>:LAPACK_NEEDS_UNDERLINE>)
endif ()

if (MPI_FOUND)
  target_link_libraries(dune-common PUBLIC MPI::MPI_CXX)
  target_compile_definitions(dune-common PUBLIC HAVE_MPI)
endif ()

if (MProtect_FOUND)
  target_link_libraries(dune-common PUBLIC MProtect::MProtect)
  target_compile_definitions(dune-common PUBLIC HAVE_MPROTECT HAVE_SYS_MMAN_H) # NOTE: the latter should be removed
endif ()

# traverse all directories
add_subdirectory(cmake/modules)
add_subdirectory(dune)

# specify installation of targets, create exports and config files
dune_install_library(dune-common)
dune_feature_summary(WHAT ALL)
