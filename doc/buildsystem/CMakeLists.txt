# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Install the buildsystem documentation defined in dune-common
install(FILES dune-common.rst DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Also always build the CMake API documentation in dune-common
dune_cmake_sphinx_doc()
