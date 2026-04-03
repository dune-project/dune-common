# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddQuadMathFlags
----------------

Helpers for using `QuadMath <https://gcc.gnu.org/onlinedocs/libquadmath>`_
from DUNE targets.

.. cmake:command:: add_dune_quadmath_flags

  Link the given targets against ``QuadMath::QuadMath`` and add the compile
  definition ``HAVE_QUADMATH=1``.

  The command has an effect only when ``QuadMath_FOUND`` is true.

  .. code-block:: cmake

    add_dune_quadmath_flags(<target> [<target>...])

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

include_guard(GLOBAL)

# set HAVE_QUADMATH for config.h
set(HAVE_QUADMATH ${QuadMath_FOUND})

# register the QuadMath imported target
if(QuadMath_FOUND)
  dune_register_package_flags(
    LIBRARIES QuadMath::QuadMath
    COMPILE_DEFINITIONS "HAVE_QUADMATH=1"
  )
endif()

# add function to link against QuadMath::QuadMath
function(add_dune_quadmath_flags _targets)
  if(QuadMath_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC QuadMath::QuadMath)
      target_compile_definitions(${_target} PUBLIC HAVE_QUADMATH=1)
    endforeach(_target ${_targets})
  endif()
endfunction(add_dune_quadmath_flags)
