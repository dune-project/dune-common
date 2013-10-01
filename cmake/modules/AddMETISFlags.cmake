# Module providing convenience functions for using METIS
#
# Provides the following functions:
#
# add_dune_metis_flags(target1 target2 ...)
#
# Adds the necessary flags to compile and link the targets with ParMETIS support.
#
function(add_dune_metis_flags _targets)
  if(METIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${METIS_LIBRARY})
    endforeach(_target ${_targets})
    set_property(TARGET ${_targets} APPEND PROPERTY
      COMPILE_DIRECTORIES ${METIS_INCLUDE_DIR})
  endif(METIS_FOUND)
endfunction(add_dune_metis_flags _targets)
