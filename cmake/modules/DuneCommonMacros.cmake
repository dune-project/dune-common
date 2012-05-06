# This cmake file holds test and directives that are executed
# for the module dune-common and have to be executed by
# all dependent modules
#
include(DuneStreams)
dune_set_minimal_debug_level()

find_package(SharedPtr)
if(Fortran_Works)
  # search for lapack
  find_package(LAPACK)
  set(HAVE_LAPACK ${LAPACK_FOUND})
  set(HAVE_BLAS ${BLAS_FOUND})
else(Fortran_Works)
  set(HAVE_LAPACK Off)
  set(HAVE_BLAS Off)
  # Write an empty FC header
  file(WRITE ${CMAKE_BINARY_DIR}/FC.h "")
endif(Fortran_Works)

