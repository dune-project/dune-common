# Defines the functions to use PTScotch
#
# .. cmake_function:: add_dune_ptscotch_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use PTScotch with.
#

# set HAVE_PTSCOTCH for config.h
set(HAVE_PTSCOTCH ${PTScotch_FOUND})

# register all PTScotch related flags
if(PTScotch_FOUND)
  dune_register_package_flags(LIBRARIES "PTScotch::PTScotch")

  dune_create_and_install_pkg_config("scotch"
    NAME "scotch"
    VERSION "${PTScotch_VERSION}"
    DESCRIPTION "Sequential and Parallel Graph Partitioning"
    URL "https://www.labri.fr/perso/pelegrin/scotch"
    TARGET PTScotch::Scotch)
  dune_create_and_install_pkg_config("ptscotch"
    NAME "PTScotch"
    VERSION "${PTScotch_VERSION}"
    DESCRIPTION "Sequential and Parallel Graph Partitioning"
    URL "https://www.labri.fr/perso/pelegrin/scotch"
    TARGET PTScotch::PTScotch)
  dune_add_pkg_config_requirement("ptscotch=${PTScotch_VERSION}")
  dune_add_pkg_config_flags("-DHAVE_PTSCOTCH")
endif()

function(add_dune_ptscotch_flags _targets)
  if(PTScotch_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC PTScotch::PTScotch)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_ptscotch_flags)
