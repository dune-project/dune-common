# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(detail)
add_subdirectory(eigen)
add_subdirectory(stl)

SET(HEADERS
attr.h         common.h   eval.h        iostream.h   pybind11.h
buffer_info.h  complex.h  extensions.h  numpy.h      pytypes.h
cast.h         eigen.h    functional.h  operators.h  stl_bind.h
chrono.h       embed.h    gil.h         options.h    stl.h)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/python/pybind11)
