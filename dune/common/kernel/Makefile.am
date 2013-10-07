SUBDIRS =					\
  bell						\
  blockdiagonal					\
  ell						\
  vec

noinst_LTLIBRARIES = libkernel.la

libkernel_la_SOURCES =
# This forces automake to use the C++ linker
# (see the automake manual, section "Libtool Convenience Libraries")
nodist_EXTRA_libkernel_la_SOURCES = dummy.cc
sourcescheck_DUMMY = dummy.cc
libkernel_la_LIBADD =				\
  bell/libbell.la				\
  blockdiagonal/libblockdiagonal.la		\
  ell/libell.la					\
  vec/libvec.la

kernelincludedir = $(includedir)/dune/common/kernel
kernelinclude_HEADERS =				\
  bell.hh					\
  blockdiagonal.hh				\
  ell.hh					\
  instantiation.hh				\
  utility.hh					\
  vec.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
