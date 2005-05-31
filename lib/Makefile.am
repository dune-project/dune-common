# $Id$

SUBDIRS = . test

# the main library
lib_LTLIBRARIES = libdune.la

# common code
libdune_la_SOURCES =

# sub-libraries
libdune_la_LIBADD = ../common/libcommon.la ../grid/libgrid.la
libdune_la_LIBADD += ../grid/onedgrid/libonedgrid.la

if UG
libdune_la_LIBADD += ../grid/uggrid/libuggrid2.la
libdune_la_LIBADD += ../grid/uggrid/libuggrid3.la
endif

if ALU3DGRID
  libdune_la_LIBADD += ../grid/alu3dgrid/libalu3d.la
endif

if AMIRAMESH
if UG
libdune_la_LIBADD += ../io/file/amiramesh/libug2amiramesh.la
libdune_la_LIBADD += ../io/file/amiramesh/libug3amiramesh.la
endif
endif

include $(top_srcdir)/am/sourcescheck
