# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Defines the functions to use PTScotch
#
# .. cmake_function:: add_dune_ptscotch_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use PTScotch with.
#
include_guard(GLOBAL)

# set HAVE_PTSCOTCH for config.h
set(HAVE_PTSCOTCH ${PTScotch_FOUND})

# register all PTScotch related flags
if(PTScotch_SCOTCH_FOUND)
  dune_register_package_flags(LIBRARIES PTScotch::Scotch
    COMPILE_DEFINITIONS "ENABLE_SCOTCH=1")
endif()
if(PTScotch_PTSCOTCH_FOUND)
  dune_register_package_flags(LIBRARIES PTScotch::PTScotch
    COMPILE_DEFINITIONS "ENABLE_PTSCOTCH=1")
endif()

function(add_dune_ptscotch_flags _targets)
  if(PTScotch_SCOTCH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC PTScotch::Scotch)
      target_compile_definitions(${_target} PUBLIC ENABLE_SCOTCH=1)
    endforeach(_target ${_targets})
  endif()
  if(PTScotch_PTSCOTCH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC PTScotch::PTScotch)
      target_compile_definitions(${_target} PUBLIC ENABLE_PTSCOTCH=1)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_ptscotch_flags)
