# Defines the functions to use QuadMath
#
# .. cmake_function:: add_dune_quadmath_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use QuadMath with.
#


function(add_dune_quadmath_flags _targets)
  if(QUADMATH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} "quadmath")
      set_property(TARGET ${_target}
        APPEND_STRING
        PROPERTY COMPILE_FLAGS "-DENABLE_QUADMATH=1 -D_GLIBCXX_USE_FLOAT128=1 ")
      if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set_property(TARGET ${_target}
          APPEND_STRING
          PROPERTY COMPILE_FLAGS "-fext-numeric-literals ")
      endif()
    endforeach(_target ${_targets})
  endif(QUADMATH_FOUND)
endfunction(add_dune_quadmath_flags)
