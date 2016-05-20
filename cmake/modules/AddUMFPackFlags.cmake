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
      target_link_libraries(${_target} ${UMFPACK_DUNE_LIBRARIES})
      get_target_property(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      set_target_properties(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${UMFPACK_DUNE_COMPILE_FLAGS} -DENABLE_UMFPACK")
    endforeach(_target ${_targets})
  endif(UMFPACK_FOUND)
endfunction(add_dune_umfpack_flags)
