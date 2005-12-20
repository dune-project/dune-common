# $Id$

SUBDIRS = albertagrid alu3dgrid common onedgrid sgrid uggrid utility yaspgrid \
	test

noinst_LTLIBRARIES = libgrid.la

# always part of lib:
ONEDLIB = onedgrid/libonedgrid.la
COMMONLIB = common/libgridcommon.la

# conditional parts:
if UG
  UGLIB = uggrid/libuggrid.la
endif

if ALUGRID
  ALU3DLIB = alu3dgrid/libalu3d.la
endif

# construct only from sub-libraries
libgrid_la_SOURCES =
libgrid_la_LIBADD = $(ONEDLIB) $(UGLIB) $(ALU3DLIB) $(COMMONLIB)

griddir = $(includedir)/dune/grid
grid_HEADERS = albertagrid.hh \
               alu3dgrid.hh \
               sgrid.hh \
               uggrid.hh \
               yaspgrid.hh \
               onedgrid.hh

include $(top_srcdir)/am/global-rules
