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
      target_link_libraries(${_target} ${PARMETIS_LIBRARY} ${METIS_LIBRARY})
      GET_TARGET_PROPERTY(_props ${_target} INCLUDE_DIRECTORIES)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES INCLUDE_DIRECTORIES
        "${_props};${PARMETIS_INCLUDE_DIRS}")
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_DEFINITIONS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_DEFINITIONS
        "${_props};ENABLE_PARMETIS")
    endforeach(_target ${_targets})
    add_dune_mpi_flags(${_targets})
  endif(PARMETIS_FOUND)
endfunction(add_dune_parmetis_flags)
