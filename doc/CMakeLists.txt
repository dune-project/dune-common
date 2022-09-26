# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory("doxygen")
add_subdirectory("buildsystem")
add_subdirectory("comm")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dunecontrol.1
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
