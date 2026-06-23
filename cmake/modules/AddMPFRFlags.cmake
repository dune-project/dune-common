# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddMPFRFlags
------------

Helpers for using `MPFR <https://github.com/advanpix/mpreal>`_ from DUNE targets.

.. cmake:command:: add_dune_mpfr_flags

  Link the given targets against ``MPFR::mpreal`` and add the compile
  definition ``HAVE_MPFR=1``.

  The command has an effect only when ``MPFR_FOUND`` is true.

  .. code-block:: cmake

    add_dune_mpfr_flags(<target> [<target>...])

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

include_guard(GLOBAL)

# register all MPFR related flags
if(MPFR_FOUND)
  dune_register_package_flags(
    LIBRARIES MPFR::mpreal
    COMPILE_DEFINITIONS "HAVE_MPFR=1"
  )
endif()

# add function to link against the MPFR library
function(add_dune_mpfr_flags _targets)
  if(MPFR_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC MPFR::mpreal)
      target_compile_definitions(${_target} PUBLIC HAVE_MPFR=1)
    endforeach(_target ${_targets})
  endif()
endfunction()
