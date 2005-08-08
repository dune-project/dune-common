# $Id$

if AMIRAMESH
if UG

noinst_LTLIBRARIES = libugamiramesh.la

libugamiramesh_la_SOURCES = amuggridreader.cc
libugamiramesh_la_CXXFLAGS = $(AMIRAMESH_CPPFLAGS) $(UG_NODIM_CPPFLAGS)
libugamiramesh_la_LIBADD   = $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS)

if PSURFACE
libugamiramesh_la_CXXFLAGS += $(PSURFACE_CPPFLAGS)
libugamiramesh_la_LIBADD   += $(PSURFACE_LDFLAGS) $(PSURFACE_LIBS)
endif

endif
endif

amirameshdir = $(includedir)/dune/io/file/amiramesh
amiramesh_HEADERS = amirameshreader.cc amsgridwriter.cc \
  amirameshwriter.cc amsimplegridwriter.cc amuggridreader.hh

include $(top_srcdir)/am/global-rules
