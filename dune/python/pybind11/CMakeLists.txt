# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(detail)

SET(HEADERS
  attr.h
  buffer_info.h
  cast.h
  chrono.h
  common.h
  complex.h
  eigen.h
  embed.h
  eval.h
  extensions.h
  functional.h
  iostream.h
  numpy.h
  operators.h
  options.h
  pybind11.h
  pytypes.h
  stl_bind.h
  stl.h
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/python/pybind11)
