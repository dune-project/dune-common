# Defines the functions to use GMP
#
# .. cmake_function:: add_dune_gmp_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use GMP with.
#


function(add_dune_gmp_flags _targets)
  if(GMP_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${GMP_LIBRARIES})
      set_property(TARGET ${_target}
        APPEND_STRING
        PROPERTY COMPILE_FLAGS "-DENABLE_GMP=1 ")
      foreach(_path ${GMP_INCLUDE_DIRS})
        set_property(TARGET ${_target}
          APPEND_STRING
          PROPERTY COMPILE_FLAGS "-I${_path}")
      endforeach(_path ${GMP_INCLUDE_DIRS})
    endforeach(_target ${_targets})
  endif(GMP_FOUND)
endfunction(add_dune_gmp_flags)
