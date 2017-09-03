# .. cmake_module::
#
#    This modules content is executed whenever a module required or suggests dune-common!
#

# enforce C++-14
dune_require_cxx_standard(MODULE "dune-common" VERSION 14)

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
endif(Fortran_Works)
set_package_properties("BLAS" PROPERTIES
  DESCRIPTION "fast linear algebra routines")
set_package_properties("LAPACK" PROPERTIES
  DESCRIPTION "fast linear algebra routines")

find_package(GMP)
include(AddGMPFlags)
find_package(Inkscape)
include(UseInkscape)
include(FindMProtect)

find_package(TBB OPTIONAL_COMPONENTS cpf allocator)

# try to find the Vc library
find_package(Vc NO_MODULE)
include(AddVcFlags)
# text for feature summary
set_package_properties("Vc" PROPERTIES
  DESCRIPTION "C++ Vectorization library"
  URL "https://github.com/VcDevel/Vc"
  PURPOSE "For use of SIMD instructions")

# Run the python extension of the Dune cmake build system
include(DunePythonCommonMacros)
