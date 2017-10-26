set(HEADERS
  common.hh
  densematrix.hh
  densevector.hh
  dimrange.hh
  dynmatrix.hh
  dynvector.hh
  fmatrix.hh
  fvector.hh
  getdimension.hh
  logger.hh
  iteratorrange.hh
  mpihelper.hh
  numpyvector.hh
  pythonvector.hh
  string.hh
  typeregistry.hh
  vector.hh
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/python/common)
