# Defines the functions to use Vc
#
# Vc is a library for high-level Vectorization support in C++
# see https://github.com/VcDevel/Vc
#
# .. cmake_function:: add_dune_vc_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use VC with.
#

function(add_dune_vc_flags _targets)
  if(Vc_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} Vc::Vc)
    endforeach(_target ${_targets})
  endif(Vc_FOUND)
endfunction(add_dune_vc_flags)
