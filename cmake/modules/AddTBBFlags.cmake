# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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

# perform DUNE-specific setup tasks
if (TBB_FOUND)
  dune_register_package_flags(
    COMPILE_DEFINITIONS ENABLE_TBB=1
    LIBRARIES TBB::tbb
    )
endif()

# function for adding TBB flags to a list of targets
function(add_dune_tbb_flags _targets)
  if(TBB_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC TBB::tbb)
      target_compile_definitions(${_target} PUBLIC ENABLE_TBB=1)
    endforeach(_target)
  endif()
endfunction(add_dune_tbb_flags)
