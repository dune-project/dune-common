# $Id$

SUBDIRS = albertagrid alu3dgrid common onedgrid sgrid uggrid yaspgrid . test

noinst_LTLIBRARIES = libgrid.la
libgrid_la_SOURCES = reftopology.cc

griddir = $(includedir)/dune/grid
grid_HEADERS = albertagrid.hh \
               alu3dgrid.hh \
               sgrid.hh \
               simplegrid.hh \
               uggrid.hh \
               yaspgrid.hh \
               onedgrid.hh \
               reftopology.hh

include $(top_srcdir)/am/sourcescheck
