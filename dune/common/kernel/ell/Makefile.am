# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libell.la

libell_la_SOURCES =				\
  mmv.cc					\
  mv.cc						\
  preconditioners/jacobi.cc			\
  umv.cc					\
  usmv.cc

ellincludedir = $(includedir)/dune/common/kernel/ell
ellinclude_HEADERS =				\
  mmv.hh					\
  mv.hh						\
  preconditioners/jacobi.hh			\
  umv.hh					\
  usmv.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
