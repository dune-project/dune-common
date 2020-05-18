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
  if (SuiteSparse_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} SuiteSparse::SuiteSparse)
    endforeach(_target ${_targets})
  endif (SuiteSparse_FOUND)
endfunction(add_dune_suitesparse_flags)
