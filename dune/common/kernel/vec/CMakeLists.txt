
#install headers
install(FILES
  add.hh
  axpy.hh
  dot.hh
  infinity_norm.hh
  mul.hh
  one_norm.hh
  scale.hh
  shift.hh
  two_norm2.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/vec)

set(noinst_LTLIBRARIES libkernel.la)
dune_add_library(kernel OBJECT
  add.cc
  axpy.cc
  dot.cc
  infinity_norm.cc
  mul.cc
  one_norm.cc
  scale.cc
  shift.cc
  two_norm2.cc)
