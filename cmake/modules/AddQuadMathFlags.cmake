# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
include_guard(GLOBAL)

# set HAVE_QUADMATH for config.h
set(HAVE_QUADMATH ${QuadMath_FOUND})

# register the QuadMath imported target
if(QuadMath_FOUND)
  dune_register_package_flags(
    LIBRARIES QuadMath::QuadMath
    COMPILE_DEFINITIONS "ENABLE_QUADMATH=1"
  )
endif()

# add function to link against QuadMath::QuadMath
function(add_dune_quadmath_flags _targets)
  if(QuadMath_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC QuadMath::QuadMath)
      target_compile_definitions(${_target} PUBLIC ENABLE_QUADMATH=1)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_quadmath_flags)
