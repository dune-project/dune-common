SET(HEADERS
  class.h
  common.h
  descr.h
  init.h
  internals.h
  typeid.h
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/python/pybind11/detail)
