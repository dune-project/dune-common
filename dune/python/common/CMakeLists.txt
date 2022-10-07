# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

set(HEADERS
  densematrix.hh
  densevector.hh
  dimrange.hh
  dynmatrix.hh
  dynvector.hh
  fmatrix.hh
  fvecmatregistry.hh
  fvector.hh
  getdimension.hh
  logger.hh
  mpihelper.hh
  numpycommdatahandle.hh
  numpyvector.hh
  pythonvector.hh
  string.hh
  typeregistry.hh
  vector.hh
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/python/common)
