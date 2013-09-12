
#install headers
install(FILES
  umv.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/ell)

set(noinst_LTLIBRARIES libell.la)
dune_add_library(ell OBJECT
  umv.cc
  )
