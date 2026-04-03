# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddTBBFlags
-----------

Helpers for using `TBB <https://github.com/oneapi-src/oneTBB>`_ from DUNE
targets.

.. cmake:command:: add_dune_tbb_flags

  Link the given targets against ``TBB::tbb``.

  The command has an effect only when ``TBB_FOUND`` is true.

  .. code-block:: cmake

    add_dune_tbb_flags(<target> [<target>...])

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

include_guard(GLOBAL)

# set variable for config.h
set(HAVE_TBB ${TBB_FOUND})

# perform DUNE-specific setup tasks
if (TBB_FOUND)
  dune_register_package_flags(
    LIBRARIES TBB::tbb
    )
endif()

# function for adding TBB flags to a list of targets
function(add_dune_tbb_flags _targets)
  if(TBB_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC TBB::tbb)
    endforeach(_target)
  endif()
endfunction(add_dune_tbb_flags)
