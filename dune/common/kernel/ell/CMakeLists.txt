
#install headers
install(FILES
  mmv.hh
  mv.hh
  umv.hh
  usmv.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/ell)

set(noinst_LTLIBRARIES libell.la)
dune_add_library(ell OBJECT
  mmv.cc
  mv.cc
  umv.cc
  usmv.cc
  )
