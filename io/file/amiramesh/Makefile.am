# $Id$

if AMIRAMESH
if UG

noinst_LTLIBRARIES = libug2amiramesh.la libug3amiramesh.la

libug2amiramesh_la_SOURCES = amuggridreader.cc
libug2amiramesh_la_CXXFLAGS = $(AMIRAMESH_CPPFLAGS) -I$(UG_INCLUDE_PATH) -D_2
libug2amiramesh_la_LIBADD   = $(AMIRAMESH_LDFLAGS)

libug3amiramesh_la_SOURCES = amuggridreader.cc
libug3amiramesh_la_CXXFLAGS = $(AMIRAMESH_CPPFLAGS) -I$(UG_INCLUDE_PATH) -D_3
libug3amiramesh_la_LIBADD   = $(AMIRAMESH_LDFLAGS)


endif
endif

amirameshdir = $(includedir)/dune/io/file/amiramesh
amiramesh_HEADERS = amirameshreader.cc amsgridwriter.cc \
  amirameshwriter.cc amsimplegridwriter.cc

include $(top_srcdir)/am/sourcescheck
