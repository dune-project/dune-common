# $Id$

SUBDIRS = lagrange dgspace

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libshapefunctions.la

libshapefunctions_la_SOURCES = lagrangeshapefunctions.cc

disc_shapefunctionsdir = $(includedir)/disc/shapefunctions
disc_shapefunctions_HEADERS = lagrangeshapefunctions.hh shapefunctions.hh

include $(top_srcdir)/am/global-rules
