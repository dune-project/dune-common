set(HEADERS
  base.hh
  defaults.hh
  interface.hh
  io.hh
  loop.hh
  simd.hh
  standard.hh
  test.hh # may be used from dependent modules
  vc.hh
)

# install headers
install(
  FILES ${HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/simd
)

# NOTE(SP): tests need to checked
# add_subdirectory(test)
