# $Id$

SUBDIRS = alberta amiramesh rawdata test

iofiledir = $(includedir)/dune/io/file
iofile_HEADERS = amirameshreader.hh asciiparser.hh rawdatawriter.hh \
  amirameshwriter.hh grapedataio.hh albertawriter.hh

noinst_LTLIBRARIES = libfileio.la

if AMIRAMESH
if UG
  AMIRALIB = amiramesh/libugamiramesh.la
endif
endif

libfileio_la_SOURCES =
libfileio_la_LIBADD = $(AMIRALIB)

include $(top_srcdir)/am/global-rules
