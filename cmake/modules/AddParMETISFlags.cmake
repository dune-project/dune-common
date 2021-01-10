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

# set HAVE_PARMETIS for config.h
set(HAVE_PARMETIS ${ParMETIS_FOUND})

# register all ParMETIS related flags
if(ParMETIS_FOUND)
  dune_register_package_flags(LIBRARIES ParMETIS::ParMETIS)

  dune_create_and_install_pkg_config("parmetis"
    NAME "ParMETIS"
    VERSION "${ParMETIS_VERSION}"
    DESCRIPTION "Parallel Graph Partitioning"
    URL "http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview"
    TARGET ParMETIS::ParMETIS)
  dune_add_pkg_config_requirement("parmetis")
endif()

# add function to link against the ParMETIS library
function(add_dune_parmetis_flags _targets)
  if(ParMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC ParMETIS::ParMETIS)
    endforeach(_target)
  endif()
endfunction(add_dune_parmetis_flags)
