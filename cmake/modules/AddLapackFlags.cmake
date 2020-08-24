# Defines the functions to use BLAS and LAPACK
#
# .. cmake_function:: add_dune_lapack_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use BLAS and LAPACK with.
#

# set HAVE_LAPACK and HAVE_BLAS for the config.h file
set(HAVE_BLAS ${BLAS_FOUND})
set(HAVE_LAPACK ${LAPACK_FOUND})

if(LAPACK_FOUND)
  include(CMakePushCheckState)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_LIBRARIES ${LAPACK_LIBRARIES})
  check_function_exists("dsyev_" LAPACK_NEEDS_UNDERLINE)
  cmake_pop_check_state()
endif()

# register all BLAS and LAPACK related flags
if(BLAS_FOUND AND LAPACK_FOUND)
  dune_register_package_flags(
    LIBRARIES BLAS::BLAS LAPACK::LAPACK
  )
endif()

# add function to link against the BLAS and LAPACK library
function(add_dune_lapack_flags _targets)
  if(BLAS_FOUND AND LAPACK_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC BLAS::BLAS LAPACK::LAPACK)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_lapack_flags)
