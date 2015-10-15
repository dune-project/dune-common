# Defines the functions to use METIS
#
# .. cmake_function:: add_dune_metis_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use METIS with.
#


function(add_dune_metis_flags _targets)
  if(METIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${METIS_LIBRARY})
    endforeach(_target ${_targets})
    set_property(TARGET ${_targets} APPEND PROPERTY
      INCLUDE_DIRECTORIES "${METIS_INCLUDE_DIRS}")
  endif(METIS_FOUND)
endfunction(add_dune_metis_flags _targets)
