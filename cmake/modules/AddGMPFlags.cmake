# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
include_guard(GLOBAL)

# set HAVE_GMP for the config.h file
set(HAVE_GMP ${GMP_FOUND})

# register all GMP related flags
if(GMP_FOUND)
  dune_register_package_flags(
    LIBRARIES GMP::gmpxx
    COMPILE_DEFINITIONS "ENABLE_GMP=1"
  )
endif()

# add function to link against the GMP library
function(add_dune_gmp_flags _targets)
  if(GMP_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC GMP::gmpxx)
      target_compile_definitions(${_target} PUBLIC ENABLE_GMP=1)
    endforeach(_target ${_targets})
  endif(GMP_FOUND)
endfunction(add_dune_gmp_flags)
