# $Id$

SUBDIRS = functions groundwater operators shapefunctions

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libdisc.la
SUBLIBS = shapefunctions/libshapefunctions.la

libdisc_la_SOURCES =
libdisc_la_LIBADD = $(SUBLIBS)

discdir = $(includedir)/disc

include $(top_srcdir)/am/global-rules
