# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# This modules contains only documentation for CMake builtins.
# This is necessary to have an complete API documentation.
#
# .. cmake_function:: add_subdirectory
#
#    .. cmake_param:: dir
#       :single:
#       :positional:
#       :required:
#
#       The :code:`CMakeLists.txt` file from this subdirectory
#       will be executed next.
#
#    .. cmake_param:: EXCLUDE_FROM_ALL
#       :option:
#
#       Whether targets added in this subdirectory should be built
#       during :code:`make all`.
#
#    This is a cmake builtin command.
#    For detailed information, check the cmake documentation:
#
#    ::
#
#       cmake --help-command add_subdirectory
#
# .. cmake_function:: install
#
#    Define installation rules to customize the behaviour of :code:`make install`.
#
#    This is a cmake builtin command.
#    For detailed information, check the cmake documentation:
#
#    ::
#
#       cmake --help-command install
#
# .. cmake_function:: add_executable
#
#    Adds an executable to the project.
#
#    This is a cmake builtin command.
#    For detailed information, check the cmake documentation:
#
#    ::
#
#       cmake --help-command add_executable
#
# .. cmake_variable:: CMAKE_<LANG>_COMPILER
#
#    Set the compiler for the language LANG.
#    LANG is in our case out of C, CXX.
#
#    This is a cmake builtin variable.
#    For detailed information, check the cmake documentation:
#
#    ::
#
#       cmake --help-variable CMAKE_\<LANG\>_COMPILER
#
# .. cmake_variable:: CMAKE_<LANG>_FLAGS
#
#    Set the compile flags for the language LANG.
#    LANG is in our case out of C, CXX.
#
#    This is a cmake builtin variable.
#    For detailed information, check the cmake documentation:
#
#    ::
#
#       cmake --help-variable CMAKE_\<LANG\>_FLAGS
#
# .. cmake_function:: find_package
#
#    Look for an external package.
#
#    This is a cmake builtin command.
#    For detailed information, check the cmake documentation:
#
#    ::
#
#       cmake --help-command find_package
#
