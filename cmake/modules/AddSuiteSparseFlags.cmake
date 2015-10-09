# module providing convenience methods for compiling
# binaries with SuiteSparse support
#
# Provides the following functions:
#
# add_dune_suitsparse_flags(target1 target2...)
#
# adds SuiteSparse flags to the targets for compilation and linking
function(add_dune_suitesparse_flags _targets)
  if(SUITESPARSE_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${SUITESPARSE_DUNE_LIBRARIES})
      get_target_property(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      set_target_properties(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${SUITESPARSE_DUNE_COMPILE_FLAGS}")
    endforeach(_target ${_targets})
  endif(SUITESPARSE_FOUND)
endfunction(add_dune_suitesparse_flags)
