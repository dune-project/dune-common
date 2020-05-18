# Defines the functions to use ParMETIS
#
# .. cmake_function:: add_dune_parmetis_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use ParMETIS with.
#


function(add_dune_parmetis_flags _targets)
  if(PARMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ParMETIS::ParMETIS)
    endforeach(_target ${_targets})
    add_dune_mpi_flags(${_targets})
  endif(PARMETIS_FOUND)
endfunction(add_dune_parmetis_flags)
