# $Id: $
dunemodulelibdir=$(libdir)

EXTRA_DIST = CMakeLists.txt dunemodules.lib
dunemodulelib_DATA = dunemodules.lib

#the dune-common library
lib_LTLIBRARIES = libdunecommon.la

# this is just a renaming of libcommon to prevent name clashes
libdunecommon_la_SOURCES =
# This forces automake to use the C++ linker
# (see the automake manual, section "Libtool Convenience Libraries")
nodist_EXTRA_libdunecommon_la_SOURCES = dummy.cc
sourcescheck_DUMMY = dummy.cc
libdunecommon_la_LIBADD = ../dune/common/libcommon.la

include $(top_srcdir)/am/global-rules
