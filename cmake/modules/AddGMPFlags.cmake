# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddGMPFlags
-----------

Helpers for using `GMP <https://gmplib.org>`_ from DUNE targets.

.. cmake:command:: add_dune_gmp_flags

  Link the given targets against ``GMP::gmpxx`` and add the compile
  definition ``HAVE_GMP=1``.

  The command has an effect only when ``GMP_FOUND`` is true.

  .. code-block:: cmake

    add_dune_gmp_flags(<target> [<target>...])

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

include_guard(GLOBAL)

# set HAVE_GMP for the config.h file
set(HAVE_GMP ${GMP_FOUND})

# register all GMP related flags
if(GMP_FOUND)
  dune_register_package_flags(
    LIBRARIES GMP::gmpxx
    COMPILE_DEFINITIONS "HAVE_GMP=1"
  )
endif()

# add function to link against the GMP library
function(add_dune_gmp_flags _targets)
  if(GMP_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC GMP::gmpxx)
      target_compile_definitions(${_target} PUBLIC HAVE_GMP=1)
    endforeach(_target ${_targets})
  endif(GMP_FOUND)
endfunction(add_dune_gmp_flags)
