
#install headers
install(FILES
  mmv.hh
  mv.hh
  preconditioners/jacobinolupivot.hh
  preconditioners/sornolupivot.hh
  umv.hh
  usmv.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/bell)

set(noinst_LTLIBRARIES libbell.la)
dune_add_library(bell OBJECT
  mmv.cc
  mv.cc
  preconditioners/jacobinolupivot.cc
  preconditioners/sornolupivot.cc
  umv.cc
  usmv.cc
  )
