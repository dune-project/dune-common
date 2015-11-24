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
      target_link_libraries(${_target} ${SuiteSparse_DUNE_LIBRARIES})
      get_target_property(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      set_target_properties(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${SuiteSparse_DUNE_COMPILE_FLAGS} -DENABLE_SUITESPARSE=1")
    endforeach(_target ${_targets})
  endif(SuiteSparse_FOUND)
endfunction(add_dune_suitesparse_flags)
