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

function(add_dune_suitesparse_flags _targets)
  if(SuiteSparse_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} SuiteSparse::SuiteSparse)
      target_compile_definitions(${_target} "ENABLE_SUITESPARSE=1")
    endforeach(_target)
  endif()
endfunction(add_dune_suitesparse_flags)
