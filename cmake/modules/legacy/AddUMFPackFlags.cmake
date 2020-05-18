# Defines the functions to use UMFPack
#
# .. cmake_function:: add_dune_umfpack_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use UMFPack with.
#


function(add_dune_umfpack_flags _targets)
  if(UMFPACK_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} UMFPack::UMFPack)
    endforeach(_target ${_targets})
  endif(UMFPACK_FOUND)
endfunction(add_dune_umfpack_flags)
