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

# register HAVE_METIS for config.h
set(HAVE_METIS ${METIS_FOUND})

# register METIS library as dune package
if(METIS_FOUND)
  dune_register_package_flags(LIBRARIES METIS::METIS)
endif()

# Add function to link targets against METIS library
function(add_dune_metis_flags _targets)
  if(METIS_FOUND)
    foreach(_target ${_targets})
      if(DUNE_USE_DEPRECATED_BUILDSYSTEM)
        target_link_libraries(${_target} METIS::METIS)
      else()
        target_link_libraries(${_target} PUBLIC METIS::METIS)
      endif()
    endforeach(_target)
  endif()
endfunction(add_dune_metis_flags _targets)
