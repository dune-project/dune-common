# $Id$

SUBDIRS = lagrange dgspace test

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libshapefunctions.la

libshapefunctions_la_SOURCES = lagrangeshapefunctions.cc

disc_shapefunctionsdir = $(includedir)/disc/shapefunctions
disc_shapefunctions_HEADERS = lagrangeshapefunctions.hh shapefunctions.hh \
	quadpointshapefunctions.hh

include $(top_srcdir)/am/global-rules
