# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddMETISFlags
-------------

Helpers for using `METIS <http://glaros.dtc.umn.edu/gkhome/metis/metis/overview>`_
from DUNE targets.

.. cmake:command:: add_dune_metis_flags

  Link the given targets against ``METIS::METIS`` and add the compile
  definitions corresponding to the detected METIS configuration.

  The command has an effect only when :cmake:variable:`METIS_FOUND` is true.

  .. code-block:: cmake

    add_dune_metis_flags(targets)

  :param targets: target or list of targets to configure

#]=======================================================================]

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
