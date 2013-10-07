# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libvec.la

libvec_la_SOURCES =				\
  add.cc					\
  assign_scalar.cc				\
  axpy.cc					\
  dot.cc					\
  infinity_norm.cc				\
  mul.cc					\
  one_norm.cc					\
  scale.cc					\
  shift.cc					\
  subtract.cc					\
  two_norm2.cc

vecincludedir = $(includedir)/dune/common/kernel/vec
vecinclude_HEADERS =				\
  add.hh					\
  assign_scalar.hh				\
  axpy.hh					\
  dot.hh					\
  infinity_norm.hh				\
  maskeddot.hh					\
  mul.hh					\
  one_norm.hh					\
  scale.hh					\
  shift.hh					\
  subtract.hh					\
  two_norm2.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
