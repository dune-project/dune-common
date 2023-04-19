# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# check whether the user wants to overload compile flags upon calling make
#
# Provides the following macros:
#
#   initialize_compiler_script() : needs to be called before further flags are added to CMAKE_CXX_FLAGS
#   finalize_compiler_script()   : needs to be called at the end of the cmake macros, e.g. in finalize_dune_project
#
# Those two macro calls are hooked into dune_project/finalize_dune_project.
#
# .. cmake_variable:: ALLOW_CXXFLAGS_OVERWRITE
#
#    Setting this option will allow you to overload preprocessor definitions from
#    the command line, as it was possible naturally with the autotools build system.
#    This feature only works with a :code:`Unix Makefiles` based generator. You can
#    use it as:
#
#    :code:`make CXXFLAGS="your flags" GRIDTYPE="grid type"`
#
#    :code:`GRIDTYPE` can be anything defined in :code:`config.h` via the :ref:`dune_define_gridtype` macro from dune-grid.
#    Furthermore any CPP variable of the form :code:`-DVAR=VALUE` can be overloaded on the command line.
#
#    .. note::
#       If you don't know what this is or what it's good for, don't use it.
#
include_guard(GLOBAL)

option(ALLOW_CXXFLAGS_OVERWRITE OFF)
option(ALLOW_CFLAGS_OVERWRITE OFF)

# init compiler script and store CXX flags
macro(initialize_compiler_script)
  if(ALLOW_CXXFLAGS_OVERWRITE OR ALLOW_CFLAGS_OVERWRITE)
    message(WARNING "ALLOW_CXXFLAGS_OVERWRITE is not longer included in the cmake build process (see MR 1251 in dune-common for details). Use dune-common/bin/compiler_launcher.sh instead! A description of usage is found there.")
  endif()
endmacro()
