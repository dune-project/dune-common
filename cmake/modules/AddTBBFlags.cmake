# Defines the functions to use TBB
#
# .. cmake_function:: add_dune_tbb_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use TBB with.
#

include_guard(GLOBAL)

# set variable for config.h
set(HAVE_TBB ${TBB_FOUND})

# function for adding TBB flags to a list of targets
function(add_dune_tbb_flags _targets)
  foreach(_target ${_targets})
    target_compile_definitions(${_target} PUBLIC ENABLE_TBB=1)
    if(TBB_COMPILE_DEFINITIONS)
      target_compile_definitions(${_target} PUBLIC ${TBB_COMPILE_DEFINITIONS})
    endif()
    if(TBB_COMPILE_OPTIONS)
      target_compile_options(${_target} PUBLIC ${TBB_COMPILE_OPTIONS})
    endif()
    if(TBB_INCLUDE_DIRS)
      target_include_directories(${_target} PUBLIC ${TBB_INCLUDE_DIRS})
    endif()
    if(TBB_LIBRARIES)
      target_link_libraries(${_target} PUBLIC ${TBB_LIBRARIES})
    endif()
  endforeach(_target)
endfunction(add_dune_tbb_flags)
