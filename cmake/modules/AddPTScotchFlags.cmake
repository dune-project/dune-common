# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddPTScotchFlags
----------------

Helpers for using `PTScotch <https://gitlab.inria.fr/scotch/scotch>`_ from
DUNE targets.

.. cmake:command:: add_dune_ptscotch_flags

  Link the given targets against the imported PTScotch targets and add the
  corresponding compile definitions.

  The command links ``PTScotch::Scotch`` when the sequential Scotch library is
  available and ``PTScotch::PTScotch`` when the parallel PTScotch library is
  available.

  .. code-block:: cmake

    add_dune_ptscotch_flags(targets)

  :param targets: target or list of targets to configure

#]=======================================================================]

include_guard(GLOBAL)

# set HAVE_PTSCOTCH for config.h
set(HAVE_PTSCOTCH ${PTScotch_FOUND})

# register all PTScotch related flags
if(PTScotch_SCOTCH_FOUND)
  dune_register_package_flags(LIBRARIES PTScotch::Scotch
    COMPILE_DEFINITIONS "HAVE_SCOTCH=1")
endif()
if(PTScotch_PTSCOTCH_FOUND)
  dune_register_package_flags(LIBRARIES PTScotch::PTScotch
    COMPILE_DEFINITIONS "HAVE_PTSCOTCH=1")
endif()

function(add_dune_ptscotch_flags _targets)
  if(PTScotch_SCOTCH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC PTScotch::Scotch)
      target_compile_definitions(${_target} PUBLIC HAVE_SCOTCH=1)
    endforeach(_target ${_targets})
  endif()
  if(PTScotch_PTSCOTCH_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC PTScotch::PTScotch)
      target_compile_definitions(${_target} PUBLIC HAVE_PTSCOTCH=1)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_ptscotch_flags)
