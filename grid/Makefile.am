# $Id$

SUBDIRS = albertagrid alu3dgrid common onedgrid sgrid uggrid yaspgrid test

noinst_LTLIBRARIES = libgrid.la

# always part of lib:
#SUBLIBS = onedgrid/libonedgrid.la

# conditional parts:
if UG
#  UGLIB = uggrid/libuggrid.la
endif

if ALUGRID
  ALU3DLIB = alu3dgrid/libalu3d.la
endif

# construct only from sub-libraries
libgrid_la_SOURCES =
libgrid_la_LIBADD = $(SUBLIBS) $(UGLIB) $(ALU3DLIB)


griddir = $(includedir)/dune/grid
grid_HEADERS = albertagrid.hh \
               alu3dgrid.hh \
               sgrid.hh \
               uggrid.hh \
               yaspgrid.hh \
               onedgrid.hh

include $(top_srcdir)/am/global-rules
