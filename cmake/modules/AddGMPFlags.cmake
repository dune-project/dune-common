# Defines the functions to use GMP
#
# .. cmake_function:: add_dune_gmp_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use GMP with.
#


function(add_dune_gmp_flags _targets)
  if(GMP_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} GMP::gmpxx)
    endforeach(_target ${_targets})
  endif(GMP_FOUND)
endfunction(add_dune_gmp_flags)
