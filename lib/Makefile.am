# $Id$

SUBDIRS = . test

# the main library
lib_LTLIBRARIES = libdune.la

# common code
libdune_la_SOURCES =

# what parts should libdune always contain?
SUBLIBS = ../common/libcommon.la ../grid/libgrid.la \
          ../grid/onedgrid/libonedgrid.la

# conditional parts:
if UG
  UGLIB = ../grid/uggrid/libuggrid2.la ../grid/uggrid/libuggrid3.la
endif

if ALU3DGRID
  ALU3DLIB = ../grid/alu3dgrid/libalu3d.la
endif

if AMIRAMESH
if UG
  AMIRALIB = ../io/file/amiramesh/libug2amiramesh.la \
             ../io/file/amiramesh/libug3amiramesh.la
endif
endif

# use the correct selection
libdune_la_LIBADD = $(SUBLIBS) $(UGLIB) $(ALU3DLIB) $(AMIRALIB)

include $(top_srcdir)/am/sourcescheck
