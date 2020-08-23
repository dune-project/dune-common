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

include_guard(GLOBAL)

# text for feature summary
set_package_properties("Vc" PROPERTIES
  DESCRIPTION "C++ Vectorization library"
  URL "https://github.com/VcDevel/Vc"
  PURPOSE "For use of SIMD instructions")

set(HAVE_VC ${Vc_FOUND})

function(add_dune_vc_flags _targets)
  if(Vc_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC ${Vc_LIBRARIES})
      target_compile_options(${_target} PUBLIC ${Vc_COMPILE_FLAGS})
      target_compile_definitions(${_target} PUBLIC ENABLE_VC=1)
      target_include_directories(${_target} SYSTEM PUBLIC ${Vc_INCLUDE_DIR})
    endforeach(_target ${_targets})
  endif(Vc_FOUND)
endfunction(add_dune_vc_flags)