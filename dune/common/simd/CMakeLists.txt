#install headers
install(FILES
  base.hh
  defaults.hh
  interface.hh
  simd.hh
  standard.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/simd)
