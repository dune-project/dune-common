# $Id$

SUBDIRS = file visual

iodir = $(includedir)/dune/io
io_HEADERS =

noinst_LTLIBRARIES = libio.la

# collect libraries from subdirs
libio_la_SOURCES =
libio_la_LIBADD = file/libfileio.la

include $(top_srcdir)/am/global-rules
