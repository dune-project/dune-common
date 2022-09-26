# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# create Doxyfile.in and Doxyfile
add_doxygen_target()

install(
  FILES
    Doxystyle
    doxygen-macros
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/dune-common/doc/doxygen
  )
