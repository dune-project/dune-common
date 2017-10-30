add_subdirectory(test)

#install headers
install(FILES
  base.hh
  defaults.hh
  interface.hh
  isstandard.hh
  loop.hh
  simd.hh
  standard.hh
  test.hh # may be used from dependent modules
  vc.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/simd)
