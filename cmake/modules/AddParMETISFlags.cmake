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
      target_compile_definitions(${_target} ENABLE_PARMETIS=1)
    endforeach(_target)
  endif()
endfunction(add_dune_parmetis_flags)
