# $Id$

# the main library
lib_LTLIBRARIES = libdune.la

# this library collects from all dune top-level directories
SUBLIBS = ../common/libcommon.la ../grid/libgrid.la ../io/libio.la

# use the correct selection
libdune_la_SOURCES =
libdune_la_LIBADD = $(SUBLIBS)

include $(top_srcdir)/am/global-rules
