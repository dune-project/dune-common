# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(test)

install(FILES
  childaccess.hh
  nodeconcepts.hh
  traversal.hh
  treecontainer.hh
  treepath.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/typetree)
