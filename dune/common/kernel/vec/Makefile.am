# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libvec.la

libvec_la_SOURCES =				\
  add.cc					\
  axpy.cc					\
  dot.cc					\
  infinity_norm.cc				\
  mul.cc					\
  one_norm.cc					\
  scale.cc					\
  shift.cc					\
  two_norm2.cc

vecincludedir = $(includedir)/dune/common/kernel/vec
vecinclude_HEADERS =				\
  add.hh					\
  axpy.hh					\
  dot.hh					\
  infinity_norm.hh				\
  mul.hh					\
  one_norm.hh					\
  scale.hh					\
  shift.hh					\
  two_norm2.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
