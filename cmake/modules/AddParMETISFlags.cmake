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

# add function to link against the ParMETIS library
function(add_dune_parmetis_flags _targets)
  if(ParMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC ParMETIS::ParMETIS)
      target_compile_definitions(${_target} PUBLIC ENABLE_PARMETIS=1)
    endforeach(_target)
  endif()
endfunction(add_dune_parmetis_flags)
