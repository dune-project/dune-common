add_subdirectory(test)

#install headers
install(FILES
  base.hh
  defaults.hh
  interface.hh
  isstandard.hh
  simd.hh
  simdfakevector.hh
  standard.hh
  test.hh # may be used from dependent modules
  vc.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/simd)
