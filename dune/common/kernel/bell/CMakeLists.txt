
#install headers
install(FILES
  mmv.hh
  mv.hh
  preconditioners/jacobinolupivot.hh
  preconditioners/sornolupivot.hh
  umv.hh
  usmv.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/bell)

dune_library_add_sources(
  dunecommon
  SOURCES
  mmv.cc
  mv.cc
  preconditioners/jacobinolupivot.cc
  preconditioners/sornolupivot.cc
  umv.cc
  usmv.cc
  )
