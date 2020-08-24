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

# set HAVE_PTSCOTCH for config.h
set(HAVE_PTSCOTCH ${PTScotch_FOUND})

# add function to link against the PTScotch library
function(add_dune_ptscotch_flags _targets)
  if(PTScotch_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC PTScotch::PTScotch)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_ptscotch_flags)
