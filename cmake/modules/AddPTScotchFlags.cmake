# Defines the functions to use PTScotch
#
# .. cmake_function:: add_dune_ptscotch_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use PTScotch with.
#


function(add_dune_ptscotch_flags _targets)
  if(PTSCOTCH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PTScotch::PTScotch)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_ptscotch_flags)
