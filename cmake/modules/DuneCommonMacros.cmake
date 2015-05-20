# This cmake file holds tests and directives that are executed
# for the module dune-common and have to be executed by
# all dependent modules
#
# Specifically it configure the DUNE debug streams and
# tests whether LAPACK and BLAS are available.
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
include(DuneBoost)

find_package(TBB OPTIONAL_COMPONENTS cpf allocator)

# actual defines are in dune-common/config.h.cmake
# could problably use cmake's configure file mechanism,
# but that requries dealing with file location for source
macro(CHECK_ALIGNOF TYPENAME)
  string(TOUPPER ${TYPENAME} TYPENAME_UPPER )
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config/alignof_${TYPENAME}.c "
#include <stdint.h>
#include <stddef.h>
#ifndef offsetof
# define offsetof(type, member) ((char *) &((type *) 0)->member - (char *) 0)
#endif

typedef struct {
    char x;
    ${TYPENAME} y;
} Align;

int main(int argc, char** argv) {
    return offsetof(Align, y);
}")
  try_run(ALIGNOF_${TYPENAME_UPPER} ALIGNOF_COMPILE_${TYPENAME_UPPER}
    ${CMAKE_CURRENT_BINARY_DIR}/config/
    ${CMAKE_CURRENT_BINARY_DIR}/config/alignof_${TYPENAME}.c
    COMPILE_OUTPUT_VARIABLE ALIGNOF_COMPILE_OUT_${TYPENAME_UPPER} )
  if(NOT ALIGNOF_COMPILE_${TYPENAME_UPPER})
    message(FATAL_ERROR "Could not calculate align of ${TYPENAME}:\n${ALIGNOF_COMPILE_OUT_${TYPENAME_UPPER}}")
  endif(NOT ALIGNOF_COMPILE_${TYPENAME_UPPER})
endmacro(CHECK_ALIGNOF TYPENAME)

CHECK_ALIGNOF(double)
CHECK_ALIGNOF(float)
CHECK_ALIGNOF(size_t)
CHECK_ALIGNOF(uint64_t)
CHECK_ALIGNOF(uint32_t)
