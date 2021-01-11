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

  dune_create_and_install_pkg_config("metis"
    NAME "METIS"
    VERSION "${METIS_VERSION}"
    DESCRIPTION "Serial Graph Partitioning"
    URL "http://glaros.dtc.umn.edu/gkhome/metis/metis/overview"
    TARGET METIS::METIS)
  dune_add_pkg_config_requirement("metis")
  dune_add_pkg_config_flags("-DHAVE_METIS")
endif()

# Add function to link targets against METIS library
function(add_dune_metis_flags _targets)
  if(METIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC METIS::METIS)
    endforeach(_target)
  endif()
endfunction(add_dune_metis_flags _targets)
