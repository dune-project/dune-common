# Module providing convenience functions for using ParMETIS
#
# Provides the following functions:
#
# add_dune_parmetis_flags(target1 target2 ...)
#
# Adds the necessary flags to compile and link the targets with ParMETIS support.
#
function(add_dune_parmetis_flags _targets)
  if(PARMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${PARMETIS_LIBRARY} ${METIS_LIBRARY})
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${PARMETIS_COMPILE_FLAGS} -DENABLE_PARMETS=1")
    endforeach(_target ${_targets})
    add_dune_mpi_flags(${_targets})
  endif(PARMETIS_FOUND)
endfunction(add_dune_parmetis_flags)
