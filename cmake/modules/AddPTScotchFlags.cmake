# Module providing convenience functions for using PT-Scotch
#
# Provides the following functions:
#
# add_dune_ptscotch_flags(target1 target2 ...)
#
# Adds the necessary flags to compile and link the targets with PT-Scotch support.
#
function(add_dune_ptscotch_flags _targets)
  if(PTSCOTCH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${PTSCOTCH_LIBRARY} ${PTSCOTCHERR_LIBRARY})
      GET_TARGET_PROPERTY(_props ${_target} INCLUDE_DIRECTORIES)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES INCLUDE_DIRECTORIES
        "${_props};${PTSCOTCH_INCLUDE_DIRS}")
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_DEFINITIONS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_DEFINITIONS
        "${_props};")
    endforeach(_target ${_targets})
    add_dune_mpi_flags(${_targets})
  endif(PTSCOTCH_FOUND)
endfunction(add_dune_ptscotch_flags)
