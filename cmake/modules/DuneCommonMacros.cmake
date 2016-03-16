# .. cmake_module::
#
#    This modules content is executed whenever a module required or suggests dune-common!
#

include(DuneStreams)
dune_set_minimal_debug_level()

if(Fortran_Works)
  # search for lapack
  find_package(LAPACK)
  set(HAVE_LAPACK ${LAPACK_FOUND})
  if(${HAVE_LAPACK})
    dune_register_package_flags(LIBRARIES "${LAPACK_LIBRARIES}")
  endif(${HAVE_LAPACK})
  set(HAVE_BLAS ${BLAS_FOUND})
else(Fortran_Works)
  set(HAVE_LAPACK Off)
  set(HAVE_BLAS Off)
  # Write an empty FC header
  file(WRITE ${CMAKE_BINARY_DIR}/FC.h "")
endif(Fortran_Works)

find_package(GMP)
include(AddGMPFlags)
find_package(Inkscape)
include(UseInkscape)
include(FindMProtect)
