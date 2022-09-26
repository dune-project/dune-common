# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(test)

if(NOT VC_FOUND)
  exclude_dir_from_headercheck()
endif()

#install headers
install(FILES
  base.hh
  defaults.hh
  interface.hh
  io.hh
  loop.hh
  simd.hh
  standard.hh
  test.hh # may be used from dependent modules
  vc.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/simd)
