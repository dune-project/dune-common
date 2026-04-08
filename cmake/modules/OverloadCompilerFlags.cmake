# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
OverloadCompilerFlags
---------------------

Legacy compatibility support for overriding compiler flags from the build
command line.

.. cmake:variable:: ALLOW_CXXFLAGS_OVERWRITE

  Allow overriding preprocessor definitions from the command line, similar to
  the historical autotools workflow.

  .. deprecated:: 2.11
     This option is no longer part of the normal CMake build process. Use
     ``dune-common/bin/compiler_launcher.sh`` instead.

  This feature only worked with ``Unix Makefiles`` based generators. Typical
  usage was:

  .. code-block:: console

    make CXXFLAGS="your flags" GRIDTYPE="grid type"

  ``GRIDTYPE`` may be any symbol defined in ``config.h`` via
  ``dune_define_gridtype()`` from dune-grid. Preprocessor definitions of the
  form ``-DVAR=VALUE`` could also be overridden from the command line.

.. cmake:variable:: ALLOW_CFLAGS_OVERWRITE

  C-language analogue of :cmake:variable:`ALLOW_CXXFLAGS_OVERWRITE`.

  .. deprecated:: 2.11
     This option is no longer part of the normal CMake build process. Use
     ``dune-common/bin/compiler_launcher.sh`` instead.

.. cmake:command:: initialize_compiler_script

  .. dune:internal::

  Legacy initialization hook for compiler-flag overriding. The command now
  only emits a warning when the deprecated overwrite options are enabled.

#]=======================================================================]

include_guard(GLOBAL)

option(ALLOW_CXXFLAGS_OVERWRITE OFF)
option(ALLOW_CFLAGS_OVERWRITE OFF)

# init compiler script and store CXX flags
macro(initialize_compiler_script)
  if(ALLOW_CXXFLAGS_OVERWRITE OR ALLOW_CFLAGS_OVERWRITE)
    message(WARNING "ALLOW_CXXFLAGS_OVERWRITE is not longer included in the cmake build process (see MR 1251 in dune-common for details). Use dune-common/bin/compiler_launcher.sh instead! A description of usage is found there.")
  endif()
endmacro()
