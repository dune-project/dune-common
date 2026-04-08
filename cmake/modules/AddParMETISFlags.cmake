# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddParMETISFlags
----------------

Helpers for using `ParMETIS <http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview>`_
from DUNE targets.

.. cmake:command:: add_dune_parmetis_flags

  Link the given targets against ``ParMETIS::ParMETIS`` and add the compile
  definitions corresponding to the detected ParMETIS configuration.

  This command also applies :cmake:command:`add_dune_metis_flags` and
  :cmake:command:`add_dune_mpi_flags`.

  .. code-block:: cmake

    add_dune_parmetis_flags(<target> [<target>...])

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

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
    add_dune_metis_flags(${_targets})
    add_dune_mpi_flags(${_targets})
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC ParMETIS::ParMETIS)
      target_compile_definitions(${_target} PUBLIC ${PARMETIS_COMPILE_DEFINITIONS})
    endforeach(_target)
  endif()
endfunction(add_dune_parmetis_flags)
