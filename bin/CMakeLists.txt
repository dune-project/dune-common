# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

install(PROGRAMS
  dune-ctest
  duneproject
  dunecontrol
  dunepackaging.py
  dune-git-whitespace-hook
  rmgenerated.py
  DESTINATION ${CMAKE_INSTALL_BINDIR})
