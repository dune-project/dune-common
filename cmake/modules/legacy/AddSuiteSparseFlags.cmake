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
endif()

# Provide function to set target properties for linking to SuiteSparse
function(add_dune_suitesparse_flags _targets)
  if (SuiteSparse_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} SuiteSparse::SuiteSparse)
<<<<<<< HEAD:cmake/modules/AddSuiteSparseFlags.cmake
      target_compile_definitions(${_target} PUBLIC ENABLE_SUITESPARSE=1)
    endforeach(_target)
  endif()
=======
    endforeach(_target ${_targets})
  endif (SuiteSparse_FOUND)
>>>>>>> initial commit of new cmake build system:cmake/modules/legacy/AddSuiteSparseFlags.cmake
endfunction(add_dune_suitesparse_flags)
