# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Defines the functions to use METIS
#
# .. cmake_function:: add_dune_metis_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use METIS with.
#
include_guard(GLOBAL)

# register HAVE_METIS for config.h
set(HAVE_METIS ${METIS_FOUND})

set(METIS_COMPILE_DEFINITIONS)
if(HAVE_METIS)
  list(APPEND METIS_COMPILE_DEFINITIONS "HAVE_METIS=1")
endif()
if(HAVE_SCOTCH_METIS)
  list(APPEND METIS_COMPILE_DEFINITIONS "HAVE_SCOTCH_METIS=1")
endif()

# register METIS library as dune package
if(METIS_FOUND)
  dune_register_package_flags(LIBRARIES METIS::METIS
    COMPILE_DEFINITIONS ${METIS_COMPILE_DEFINITIONS})
endif()

# Add function to link targets against METIS library
function(add_dune_metis_flags _targets)
  if(METIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC METIS::METIS)
      target_compile_definitions(${_target} PUBLIC ${METIS_COMPILE_DEFINITIONS})
    endforeach(_target)
  endif()
endfunction(add_dune_metis_flags _targets)
