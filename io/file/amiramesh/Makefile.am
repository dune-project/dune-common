# $Id$

if AMIRAMESH
if UG

noinst_LTLIBRARIES = libug2amiramesh.la libug3amiramesh.la

libug2amiramesh_la_SOURCES = amuggridreader.cc
libug2amiramesh_la_CXXFLAGS = $(AMIRAMESH_CPPFLAGS) $(UG_NODIM_CPPFLAGS) -D_2
libug2amiramesh_la_LIBADD   = $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS)

libug3amiramesh_la_SOURCES = amuggridreader.cc
libug3amiramesh_la_CXXFLAGS = $(AMIRAMESH_CPPFLAGS) $(UG_NODIM_CPPFLAGS) -D_3
libug3amiramesh_la_LIBADD   = $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS)

endif
endif

amirameshdir = $(includedir)/dune/io/file/amiramesh
amiramesh_HEADERS = amirameshreader.cc amsgridwriter.cc \
  amirameshwriter.cc amsimplegridwriter.cc

include $(top_srcdir)/am/sourcescheck
