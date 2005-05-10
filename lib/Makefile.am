# $Id$

SUBDIRS = . test

# the main library
lib_LTLIBRARIES = libdune.la

# common code
libdune_la_SOURCES =

# sub-libraries
libdune_la_LIBADD = ../common/libcommon.la
libdune_la_LIBADD += ../grid/onedgrid/libonedgrid.la
libdune_la_LIBADD += ../grid/alu3dgrid/libalu3d.la

include $(top_srcdir)/am/sourcescheck
