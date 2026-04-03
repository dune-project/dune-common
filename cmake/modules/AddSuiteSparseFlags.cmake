# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddSuiteSparseFlags
-------------------

Helpers for using `SuiteSparse <http://faculty.cse.tamu.edu/davis/suitesparse.html>`_
from DUNE targets.

.. cmake:command:: add_dune_suitesparse_flags

  Link the given targets against ``SuiteSparse::SuiteSparse`` and add the
  compile definitions corresponding to the detected SuiteSparse components.

  The command has an effect only when ``SuiteSparse_FOUND`` is true.

  .. code-block:: cmake

    add_dune_suitesparse_flags(targets)

  :param targets: target or list of targets to configure

#]=======================================================================]

include_guard(GLOBAL)

# set HAVE_SUITESPARSE for config.h
set(HAVE_SUITESPARSE ${SuiteSparse_FOUND})
set(HAVE_UMFPACK ${SuiteSparse_UMFPACK_FOUND})

# register all SuiteSparse related flags
if(SuiteSparse_FOUND)
  set(HAVE_SUITESPARSE_FLAGS)
  foreach(_component ${SuiteSparse_FOUND_COMPONENTS})
    list(APPEND HAVE_SUITESPARSE_FLAGS "HAVE_SUITESPARSE_${_component}=1")
  endforeach(_component)
  dune_register_package_flags(
    COMPILE_DEFINITIONS ${HAVE_SUITESPARSE_FLAGS}
    LIBRARIES SuiteSparse::SuiteSparse)
  unset(HAVE_SUITESPARSE_FLAGS)
endif()

# Provide function to set target properties for linking to SuiteSparse
function(add_dune_suitesparse_flags _targets)
  if(SuiteSparse_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC SuiteSparse::SuiteSparse)
      foreach(_component ${SuiteSparse_FOUND_COMPONENTS})
        target_compile_definitions(${_target} PUBLIC HAVE_SUITESPARSE_${_component}=1)
      endforeach(_component)
    endforeach(_target)
  endif()
endfunction(add_dune_suitesparse_flags)
