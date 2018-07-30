# Defines the functions to use MPFR
#
# .. cmake_function:: add_dune_mpfr_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use MPFR with.
#


function(add_dune_mpfr_flags _targets)
  if(MPFR_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${MPFR_LIBRARIES})
      set_property(TARGET ${_target}
        APPEND_STRING
        PROPERTY COMPILE_FLAGS "-DENABLE_MPFR=1 ")
      foreach(_path ${MPFR_INCLUDE_DIRS})
        set_property(TARGET ${_target}
          APPEND_STRING
          PROPERTY COMPILE_FLAGS "-I${_path}")
      endforeach(_path ${MPFR_INCLUDE_DIRS})
    endforeach(_target ${_targets})
  endif(MPFR_FOUND)
endfunction(add_dune_mpfr_flags)
