# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Defines the functions to use ParMETIS
#
# .. cmake_function:: add_dune_parmetis_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use ParMETIS with.
#
include_guard(GLOBAL)

# set HAVE_PARMETIS for config.h
set(HAVE_PARMETIS ${ParMETIS_FOUND})

set(PARMETIS_COMPILE_DEFINITIONS)
if(HAVE_PARMETIS)
  list(APPEND PARMETIS_COMPILE_DEFINITIONS "HAVE_PARMETIS=1")
endif()
if(HAVE_SCOTCH_METIS)
  list(APPEND PARMETIS_COMPILE_DEFINITIONS "HAVE_PTSCOTCH_PARMETIS=1")
endif()

# register all ParMETIS related flags
if(ParMETIS_FOUND)
  dune_register_package_flags(LIBRARIES ParMETIS::ParMETIS
    COMPILE_DEFINITIONS ${PARMETIS_COMPILE_DEFINITIONS})
endif()

# add function to link against the ParMETIS library
function(add_dune_parmetis_flags _targets)
  if(ParMETIS_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC ParMETIS::ParMETIS)
      target_compile_definitions(${_target} PUBLIC ${PARMETIS_COMPILE_DEFINITIONS})
    endforeach(_target)
  endif()
endfunction(add_dune_parmetis_flags)
