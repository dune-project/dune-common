# Defines the functions to use SuiteSparse
#
# .. cmake_function:: add_dune_suitesparse_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use SuiteSparse with.
#

# set HAVE_SUITESPARSE for config.h
set(HAVE_SUITESPARSE ${SuiteSparse_FOUND})
set(HAVE_UMFPACK ${SuiteSparse_UMFPACK_FOUND})

# register all SuiteSparse related flags
if(SuiteSparse_FOUND)
  dune_register_package_flags(
    COMPILE_DEFINITIONS "ENABLE_SUITESPARSE=1"
    LIBRARIES SuiteSparse::SuiteSparse)

  list(TRANSFORM SuiteSparse_INCLUDE_DIRS PREPEND "-I" OUTPUT_VARIABLE SuiteSparse_CFLAGS)
  dune_create_and_install_pkg_config("suitesparse"
    NAME "SuiteSparse"
    VERSION "${SuiteSparse_VERSION}"
    DESCRIPTION "A suite of sparse matrix software"
    URL "http://faculty.cse.tamu.edu/davis/suitesparse.html"
    CFLAGS "${SuiteSparse_CFLAGS}"
    LIBS "${SuiteSparse_LIBRARIES}"
    REQUIRES "blas;lapack")
  dune_add_pkg_config_requirement("suitesparse")
  dune_add_pkg_config_flags("-DHAVE_SUITESPARSE")
  dune_add_pkg_config_flags("-DHAVE_UMFPACK")
endif()

# Provide function to set target properties for linking to SuiteSparse
function(add_dune_suitesparse_flags _targets)
  if(SuiteSparse_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC SuiteSparse::SuiteSparse)
      target_compile_definitions(${_target} PUBLIC ENABLE_SUITESPARSE=1)
    endforeach(_target)
  endif()
endfunction(add_dune_suitesparse_flags)
