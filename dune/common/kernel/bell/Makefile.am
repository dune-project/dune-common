# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libbell.la

libbell_la_SOURCES =				\
  mmv.cc					\
  mv.cc						\
  preconditioners/jacobinolupivot.cc		\
  preconditioners/sornolupivot.cc		\
  umv.cc					\
  usmv.cc

bellincludedir = $(includedir)/dune/common/kernel/bell
bellinclude_HEADERS =				\
  mmv.hh					\
  mv.hh						\
  preconditioners/jacobinolupivot.hh		\
  preconditioners/sornolupivot.hh		\
  umv.hh					\
  usmv.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
