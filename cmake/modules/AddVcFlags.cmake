# Defines the functions to use GMP
#
# .. cmake_function:: add_dune_vc_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use GMP with.
#

MESSAGE(STATUS "{Vc_LIBRARIES} ${Vc_LIBRARIES}")
MESSAGE(STATUS "{Vc_ALL_FLAGS} ${Vc_ALL_FLAGS}")
MESSAGE(STATUS "{Vc_INCLUDE_DIR} ${Vc_INCLUDE_DIR}")

function(add_dune_vc_flags _targets)
  if(Vc_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${Vc_LIBRARIES})
      target_compile_options(${_target} ${Vc_ALL_FLAGS})
      target_include_directories(${_target} SYSTEM ${Vc_INCLUDE_DIR})
    endforeach(_target ${_targets})
  endif(Vc_FOUND)
endfunction(add_dune_vc_flags)
