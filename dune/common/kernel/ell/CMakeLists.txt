
#install headers
install(FILES
  mmv.hh
  mv.hh
  umv.hh
  usmv.hh
  preconditioners/jacobi.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/ell)

dune_library_add_sources(
  dunecommon
  SOURCES
  mmv.cc
  mv.cc
  umv.cc
  usmv.cc
  preconditioners/jacobi.cc
  )
