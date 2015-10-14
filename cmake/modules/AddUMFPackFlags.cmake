# module providing convenience mehtods for compiling bianries with UMFPack support
#
# Provides the following functions:
#
# add_dune_umfpack_flags(target1 target2...)
#
# adds UMFPack flags to the targets for compilation and linking
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